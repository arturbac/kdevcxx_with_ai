// SPDX-FileCopyrightText: 2026 Artur Bać
// SPDX-License-Identifier: MIT

#include "kdevcxx_with_ai.h"
#include "claude_view.h"
#include "config_page.h"
#include "diff_dialog.h"

#include <ide_server.h>
#include <qt_bridge.h>

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

K_PLUGIN_FACTORY_WITH_JSON(kdevcxx_with_ai_factory, "kdevcxx_with_ai.json", registerPlugin<kdevcxx_with_ai>();)

using namespace Qt::StringLiterals;
using ide_qt::to_qt;
using ide_qt::to_std;

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
  }  // namespace

kdevcxx_with_ai::kdevcxx_with_ai(QObject * parent, KPluginMetaData const & meta_data, QVariantList const &) :
    KDevelop::IPlugin{u"kdevcxx_with_ai"_s, parent, meta_data},
    view_factory_{std::make_unique<claude_view_factory_t>(this)}
  {
  if(auto token{ide_protocol::make_auth_token()}; not token)
    qWarning("kdevcxx_with_ai: no auth token: %s", token.error().message().c_str());
  else
    {
    auth_token_ = std::move(*token);
    server_ = new ide_qt::ide_server_t{auth_token_, *this, this};
    port_ = server_->listen();
    }
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
  if(server_)
    connect(
      server_,
      &ide_qt::ide_server_t::client_connected,
      this,
      [this]
      {
        if(not last_selection_.empty())
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

auto kdevcxx_with_ai::open_diff(ide_protocol::open_diff_args_t const & args, ide_protocol::diff_reply_t reply) -> void
  {
  auto const tab_name{to_qt(args.tab_name)};
  if(auto previous{diffs_.take(tab_name)}; previous)
    previous->close_tab();
  diffs_.removeIf([](auto const & entry) { return entry.value().isNull(); });

  auto * dialog{new diff_dialog_t{
    tab_name,
    to_qt(args.old_file_path),
    to_qt(args.new_file_contents),
    std::move(reply),
    KDevelop::ICore::self()->uiController()->activeMainWindow()
  }};
  diffs_.insert(tab_name, dialog);
  dialog->show();
  dialog->raise();
  dialog->activateWindow();
  }

auto kdevcxx_with_ai::close_tab(std::string_view tab_name) -> void
  {
  if(auto dialog{diffs_.take(to_qt(tab_name))}; dialog)
    dialog->close_tab();
  }

auto kdevcxx_with_ai::close_all_diff_tabs() -> std::size_t
  {
  std::size_t count{};
  for(auto const & dialog: std::exchange(diffs_, {}))
    if(dialog)
      {
      dialog->close_tab();
      ++count;
      }
  return count;
  }

auto kdevcxx_with_ai::diagnostics(std::string_view file) -> ide_protocol::diagnostics_t
  {
  QStringList files;
  if(not file.empty())
    files.append(to_qt(file));
  else
    for(auto const * document: KDevelop::ICore::self()->documentController()->openDocuments())
      if(document->url().isLocalFile())
        files.append(document->url().toLocalFile());

  ide_protocol::diagnostics_t result{.files = to_std(files), .diagnostics = {}};
  // claude waits for the answer with a timeout, never block on a busy DUChain
  KDevelop::DUChainReadLocker lock{KDevelop::DUChain::lock(), 500};
  if(not lock.locked())
    return result;
  for(qsizetype i{}; i != files.size(); ++i)
    {
    auto const & path{files[i]};
    auto const * top{KDevelop::DUChainUtils::standardContextForUrl(QUrl::fromLocalFile(path))};
    if(not top)
      continue;
    for(auto const & problem: top->problems())
      {
      auto const location{problem->finalLocation()};
      if(location.document.str() != path)
        continue;
      result.diagnostics.push_back(
        {.file_path = result.files[static_cast<std::size_t>(i)],
         .message = to_std(problem->description()),
         .severity = to_severity(problem->severity()),
         .start = {location.start().line(), location.start().column()},
         .end = {location.end().line(), location.end().column()},
         .source = to_std(problem->sourceString())}
      );
      }
    }
  return result;
  }

auto kdevcxx_with_ai::launch_command() const -> QString
  { return to_qt(ide_protocol::claude_launch_command(port_, to_std(read_claude_command()))); }

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
  std::vector<std::string> folders;
  for(auto const * project: KDevelop::ICore::self()->projectController()->projects())
    folders.push_back(to_std(project->path().toLocalFile()));

  QDir const dir{
    to_qt(ide_protocol::lock_dir(to_std(qEnvironmentVariable("CLAUDE_CONFIG_DIR")), to_std(QDir::homePath())).string())
  };
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
  auto const json{ide_protocol::lock_file_json(QCoreApplication::applicationPid(), folders, auth_token_)};
  file.write(json.data(), static_cast<qint64>(json.size()));
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
    to_std(view->document()->url().toLocalFile()),
    to_std(view->selectionText()),
    {range.start().line(), range.start().column()},
    {range.end().line(), range.end().column()}
  )};
  if(message == last_selection_)
    return;
  last_selection_ = std::move(message);
  if(server_)
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
  if(server_)
    server_->broadcast(ide_protocol::at_mentioned(to_std(view->document()->url().toLocalFile()), line_start, line_end));
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
