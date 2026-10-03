// SPDX-FileCopyrightText: 2026 Artur Bać
// SPDX-License-Identifier: MIT

#include "kdevcxx_with_ai.h"
#include "claude_view.h"
#include "config_page.h"
#include "diff_dialog.h"

#include <ide_server.h>

#include <interfaces/contextmenuextension.h>
#include <interfaces/icore.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iuicontroller.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/problem.h>
#include <language/duchain/topducontext.h>
#include <language/interfaces/editorcontext.h>
#include <util/path.h>

#include <KActionCollection>
#include <KLocalizedString>
#include <KParts/MainWindow>
#include <KPluginFactory>
#include <KTextEditor/Document>
#include <KTextEditor/View>

#include <QAction>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>

K_PLUGIN_FACTORY_WITH_JSON(kdevcxx_with_ai_factory, "kdevcxx_with_ai.json", registerPlugin<kdevcxx_with_ai>();)

using namespace Qt::StringLiterals;

class claude_view_factory_t : public KDevelop::IToolViewFactory
  {
public:
  explicit claude_view_factory_t(kdevcxx_with_ai * plugin) : plugin_{plugin} {}

  auto create(QWidget * parent) -> QWidget * override
    { return new claude_view_t{plugin_->launch_command(), plugin_->working_dir(), parent}; }

  [[nodiscard]]
  auto id() const -> QString override
    { return u"org.kdevelop.ClaudeCodeView"_s; }

  [[nodiscard]]
  auto defaultPosition() const -> Qt::DockWidgetArea override
    { return Qt::BottomDockWidgetArea; }

private:
  kdevcxx_with_ai * plugin_;
  };

namespace
  {
[[nodiscard]]
auto tool_view_title() -> QString
  { return i18n("Claude Code"); }

[[nodiscard]]
auto to_severity(KDevelop::IProblem::Severity severity) -> ide_protocol::severity_e
  {
  switch(severity)
    {
    case KDevelop::IProblem::Error:      return ide_protocol::severity_e::error;
    case KDevelop::IProblem::Warning:    return ide_protocol::severity_e::warning;
    case KDevelop::IProblem::Hint:       return ide_protocol::severity_e::info;
    case KDevelop::IProblem::NoSeverity: return ide_protocol::severity_e::hint;
    }
  return ide_protocol::severity_e::hint;
  }

[[nodiscard]]
auto string_property(QString const & description) -> QJsonObject
  { return {{u"type"_s, u"string"_s}, {u"description"_s, description}}; }
  }  // namespace

kdevcxx_with_ai::kdevcxx_with_ai(QObject * parent, KPluginMetaData const & meta_data, QVariantList const &) :
    KDevelop::IPlugin{u"kdevcxx_with_ai"_s, parent, meta_data},
    auth_token_{ide_protocol::make_auth_token()},
    view_factory_{std::make_unique<claude_view_factory_t>(this)}
  {
  server_ = new ide_protocol::ide_server_t{auth_token_, tools(), this};
  port_ = server_->listen();
  if(port_ == 0)
    qWarning("kdevcxx_with_ai: cannot listen on 127.0.0.1, Claude Code IDE integration is disabled");
  else
    write_lock_file();

  auto * projects{KDevelop::ICore::self()->projectController()};
  connect(projects, &KDevelop::IProjectController::projectOpened, this, &kdevcxx_with_ai::write_lock_file);
  connect(projects, &KDevelop::IProjectController::projectClosed, this, &kdevcxx_with_ai::write_lock_file);

  selection_timer_.setSingleShot(true);
  selection_timer_.setInterval(100);
  connect(&selection_timer_, &QTimer::timeout, this, &kdevcxx_with_ai::send_selection);
  connect(
    KDevelop::ICore::self()->documentController(),
    &KDevelop::IDocumentController::documentActivated,
    this,
    &kdevcxx_with_ai::track_view
  );
  connect(
    server_,
    &ide_protocol::ide_server_t::client_connected,
    this,
    [this]
    {
      if(not last_selection_.isEmpty())
        server_->broadcast(last_selection_);
    }
  );

  KDevelop::ICore::self()->uiController()->addToolView(tool_view_title(), view_factory_.get());
  }

kdevcxx_with_ai::~kdevcxx_with_ai() { remove_lock_file(); }

auto kdevcxx_with_ai::unload() -> void
  {
  remove_lock_file();
  for(auto const & dialog: std::as_const(diffs_))
    if(dialog)
      dialog->close();
  KDevelop::ICore::self()->uiController()->removeToolView(view_factory_.get());
  }

auto kdevcxx_with_ai::tools() -> std::vector<ide_protocol::tool_t>
  {
  auto bind{[this](auto member)
            {
              return [this, member](QJsonObject const & args, ide_protocol::reply_t reply)
              { (this->*member)(args, std::move(reply)); };
            }};
  return {
    {u"openDiff"_s,
     u"Open a diff of a proposed file change for the user to accept or reject (blocking)"_s,
     {{u"type"_s, u"object"_s},
      {u"properties"_s,
       QJsonObject{
         {u"old_file_path"_s, string_property(u"Path of the file to change"_s)},
         {u"new_file_path"_s, string_property(u"Path of the changed file"_s)},
         {u"new_file_contents"_s, string_property(u"Proposed contents"_s)},
         {u"tab_name"_s, string_property(u"Name of the diff tab"_s)}
       }},
      {u"required"_s, QJsonArray{u"old_file_path"_s, u"new_file_path"_s, u"new_file_contents"_s, u"tab_name"_s}}},
     bind(&kdevcxx_with_ai::open_diff)},
    {u"close_tab"_s,
     u"Close a diff tab by name"_s,
     {{u"type"_s, u"object"_s},
      {u"properties"_s, QJsonObject{{u"tab_name"_s, string_property(u"Name of the diff tab"_s)}}},
      {u"required"_s, QJsonArray{u"tab_name"_s}}},
     bind(&kdevcxx_with_ai::close_tab)},
    {u"closeAllDiffTabs"_s,
     u"Close all diff tabs"_s,
     {{u"type"_s, u"object"_s}, {u"properties"_s, QJsonObject{}}},
     bind(&kdevcxx_with_ai::close_all_diff_tabs)},
    {u"getDiagnostics"_s,
     u"Get language diagnostics (errors, warnings) from KDevelop for a file, or for all open files when uri is "
     u"omitted"_s,
     {{u"type"_s, u"object"_s},
      {u"properties"_s, QJsonObject{{u"uri"_s, string_property(u"file:// URI of the file"_s)}}}},
     bind(&kdevcxx_with_ai::get_diagnostics)}
  };
  }

auto kdevcxx_with_ai::open_diff(QJsonObject const & arguments, ide_protocol::reply_t reply) -> void
  {
  auto const tab_name{arguments.value(u"tab_name"_s).toString()};
  if(auto previous{diffs_.take(tab_name)}; previous)
    previous->close_tab();
  diffs_.removeIf([](auto const & entry) { return entry.value().isNull(); });

  auto * dialog{new diff_dialog_t{
    tab_name,
    arguments.value(u"old_file_path"_s).toString(),
    arguments.value(u"new_file_contents"_s).toString(),
    std::move(reply),
    KDevelop::ICore::self()->uiController()->activeMainWindow()
  }};
  diffs_.insert(tab_name, dialog);
  dialog->show();
  dialog->raise();
  dialog->activateWindow();
  }

auto kdevcxx_with_ai::close_tab(QJsonObject const & arguments, ide_protocol::reply_t reply) -> void
  {
  if(auto dialog{diffs_.take(arguments.value(u"tab_name"_s).toString())}; dialog)
    dialog->close_tab();
  reply(ide_protocol::text_result(QString::fromLatin1(ide_protocol::tab_closed)));
  }

auto kdevcxx_with_ai::close_all_diff_tabs(QJsonObject const &, ide_protocol::reply_t reply) -> void
  {
  int count{};
  for(auto const & dialog: std::exchange(diffs_, {}))
    if(dialog)
      {
      dialog->close_tab();
      ++count;
      }
  reply(ide_protocol::text_result(u"CLOSED_%1_DIFF_TABS"_s.arg(count)));
  }

auto kdevcxx_with_ai::get_diagnostics(QJsonObject const & arguments, ide_protocol::reply_t reply) -> void
  {
  QStringList files;
  if(auto const uri{arguments.value(u"uri"_s).toString()}; not uri.isEmpty())
    files.append(ide_protocol::uri_to_path(uri));
  else
    for(auto const * document: KDevelop::ICore::self()->documentController()->openDocuments())
      if(document->url().isLocalFile())
        files.append(document->url().toLocalFile());

  std::vector<ide_protocol::diagnostic_t> diagnostics;
  // claude waits for the answer with a timeout, never block on a busy DUChain
  KDevelop::DUChainReadLocker lock{KDevelop::DUChain::lock(), 500};
  if(lock.locked())
    for(auto const & file: std::as_const(files))
      {
      auto const * top{KDevelop::DUChainUtils::standardContextForUrl(QUrl::fromLocalFile(file))};
      if(not top)
        continue;
      for(auto const & problem: top->problems())
        {
        auto const location{problem->finalLocation()};
        if(location.document.str() != file)
          continue;
        diagnostics.push_back(
          {.file_path = file,
           .message = problem->description(),
           .severity = to_severity(problem->severity()),
           .start = {location.start().line(), location.start().column()},
           .end = {location.end().line(), location.end().column()},
           .source = problem->sourceString()}
        );
        }
      }
  reply(ide_protocol::diagnostics_result(files, diagnostics));
  }

auto kdevcxx_with_ai::launch_command() const -> QString
  { return ide_protocol::claude_launch_command(port_, read_claude_command()); }

auto kdevcxx_with_ai::working_dir() const -> QString
  {
  auto const projects{KDevelop::ICore::self()->projectController()->projects()};
  if(not projects.isEmpty())
    return projects.front()->path().toLocalFile();
  if(
    auto const * document{KDevelop::ICore::self()->documentController()->activeDocument()};
    document and document->url().isLocalFile()
  )
    return QFileInfo{document->url().toLocalFile()}.absolutePath();
  return QDir::homePath();
  }

auto kdevcxx_with_ai::write_lock_file() -> void
  {
  if(port_ == 0)
    return;
  QStringList folders;
  for(auto const * project: KDevelop::ICore::self()->projectController()->projects())
    folders.append(project->path().toLocalFile());

  QDir const dir{ide_protocol::lock_dir(qEnvironmentVariable("CLAUDE_CONFIG_DIR"), QDir::homePath())};
  dir.mkpath(u"."_s);
  lock_path_ = dir.filePath(u"%1.lock"_s.arg(port_));
  QFile file{lock_path_};
  // the file holds the auth token, keep it private
  if(
    not file.open(QIODevice::WriteOnly | QIODevice::Truncate)
    or not file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner)
  )
    {
    qWarning("kdevcxx_with_ai: cannot write %s", qPrintable(lock_path_));
    return;
    }
  file.write(ide_protocol::lock_file_json(QCoreApplication::applicationPid(), folders, auth_token_));
  }

auto kdevcxx_with_ai::remove_lock_file() -> void
  {
  if(not lock_path_.isEmpty())
    QFile::remove(std::exchange(lock_path_, {}));
  }

auto kdevcxx_with_ai::track_view(KDevelop::IDocument * document) -> void
  {
  if(auto * view{document ? document->activeTextView() : nullptr}; view)
    {
    connect(
      view, &KTextEditor::View::selectionChanged, this, &kdevcxx_with_ai::schedule_selection, Qt::UniqueConnection
    );
    connect(
      view, &KTextEditor::View::cursorPositionChanged, this, &kdevcxx_with_ai::schedule_selection, Qt::UniqueConnection
    );
    }
  schedule_selection();
  }

auto kdevcxx_with_ai::schedule_selection() -> void { selection_timer_.start(); }

auto kdevcxx_with_ai::send_selection() -> void
  {
  auto const * view{KDevelop::ICore::self()->documentController()->activeTextDocumentView()};
  if(not view or not view->document()->url().isLocalFile())
    return;
  auto const range{
    view->selection() ? view->selectionRange() : KTextEditor::Range{view->cursorPosition(), view->cursorPosition()}
  };
  auto message{ide_protocol::selection_changed(
    view->document()->url().toLocalFile(),
    view->selectionText(),
    {range.start().line(), range.start().column()},
    {range.end().line(), range.end().column()}
  )};
  if(message == last_selection_)
    return;
  last_selection_ = std::move(message);
  server_->broadcast(last_selection_);
  }

auto kdevcxx_with_ai::send_at_mention() -> void
  {
  auto const * view{KDevelop::ICore::self()->documentController()->activeTextDocumentView()};
  if(not view or not view->document()->url().isLocalFile())
    return;
  std::optional<int> line_start;
  std::optional<int> line_end;
  if(view->selection())
    {
    auto const range{view->selectionRange()};
    line_start = range.start().line();
    // a selection ending at column 0 does not include that line
    line_end = range.end().column() == 0 and range.end().line() > range.start().line() ? range.end().line() - 1
                                                                                       : range.end().line();
    }
  server_->broadcast(ide_protocol::at_mentioned(view->document()->url().toLocalFile(), line_start, line_end));
  KDevelop::ICore::self()->uiController()->findToolView(
    tool_view_title(), view_factory_.get(), KDevelop::IUiController::CreateAndRaise
  );
  }

auto kdevcxx_with_ai::contextMenuExtension(KDevelop::Context * context, QWidget * parent)
  -> KDevelop::ContextMenuExtension
  {
  KDevelop::ContextMenuExtension extension;
  if(context->type() == KDevelop::Context::EditorContext)
    {
    auto * action{new QAction{QIcon::fromTheme(u"utilities-terminal"_s), i18n("Send to Claude Code"), parent}};
    connect(action, &QAction::triggered, this, &kdevcxx_with_ai::send_at_mention);
    extension.addAction(KDevelop::ContextMenuExtension::EditGroup, action);
    }
  return extension;
  }

auto kdevcxx_with_ai::createActionsForMainWindow(Sublime::MainWindow *, QString &, KActionCollection & actions) -> void
  {
  auto * action{new QAction{QIcon::fromTheme(u"utilities-terminal"_s), i18n("Send Selection to Claude Code"), this}};
  actions.addAction(u"claude_code_send_selection"_s, action);
  connect(action, &QAction::triggered, this, &kdevcxx_with_ai::send_at_mention);
  }

auto kdevcxx_with_ai::configPages() const -> int { return 1; }

auto kdevcxx_with_ai::configPage(int number, QWidget * parent) -> KDevelop::ConfigPage *
  { return number == 0 ? new config_page_t{this, parent} : nullptr; }

#include "kdevcxx_with_ai.moc"
