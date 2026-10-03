// SPDX-FileCopyrightText: 2026 Artur Bać
// SPDX-License-Identifier: MIT

#include "diff_dialog.h"

#include <KLocalizedString>
#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/View>

#include <QDialogButtonBox>
#include <QFileInfo>
#include <QProcess>
#include <QPushButton>
#include <QTemporaryFile>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

namespace
  {
/// unified diff of the file on disk against the proposed contents, using diff from diffutils
[[nodiscard]]
auto unified_diff(QString const & file_path, QString const & new_contents) -> QString
  {
  QTemporaryFile proposed;
  if(not proposed.open())
    return i18n("Cannot create a temporary file for the diff.");
  proposed.write(new_contents.toUtf8());
  proposed.flush();

  auto const source{QFileInfo::exists(file_path) ? file_path : u"/dev/null"_s};
  QProcess diff;
  diff.start(u"diff"_s, {u"-u"_s, u"--label"_s, file_path, u"--label"_s, file_path, source, proposed.fileName()});
  // exit code 1 means the files differ
  if(not diff.waitForFinished() or diff.exitCode() > 1)
    return i18n("Running diff failed: %1", diff.errorString());
  return QString::fromUtf8(diff.readAllStandardOutput());
  }
  }  // namespace

diff_dialog_t::diff_dialog_t(
  QString const & tab_name,
  QString const & file_path,
  QString const & new_contents,
  ide_protocol::diff_reply_t reply,
  QWidget * parent
) :
    QDialog{parent},
    reply_{std::move(reply)}
  {
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle(tab_name);
  resize(1200, 800);
  auto * layout{new QVBoxLayout{this}};

  auto * document{KTextEditor::Editor::instance()->createDocument(this)};
  document->setText(unified_diff(file_path, new_contents));
  document->setHighlightingMode(u"Diff"_s);
  document->setModified(false);
  document->setReadWrite(false);
  layout->addWidget(document->createView(this), 1);

  auto * buttons{new QDialogButtonBox{this}};
  buttons->addButton(i18n("Accept"), QDialogButtonBox::AcceptRole)->setIcon(QIcon::fromTheme(u"dialog-ok-apply"_s));
  buttons->addButton(i18n("Reject"), QDialogButtonBox::RejectRole)->setIcon(QIcon::fromTheme(u"dialog-cancel"_s));
  layout->addWidget(buttons);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(
    this,
    &QDialog::finished,
    this,
    [this](int result)
    {
      using enum ide_protocol::diff_outcome_e;
      send(result == QDialog::Accepted ? accepted : rejected);
    }
  );
  }

diff_dialog_t::~diff_dialog_t() { send(ide_protocol::diff_outcome_e::rejected); }

auto diff_dialog_t::close_tab() -> void
  {
  send(ide_protocol::diff_outcome_e::tab_closed);
  close();
  }

auto diff_dialog_t::send(ide_protocol::diff_outcome_e outcome) -> void
  {
  if(reply_)
    std::exchange(reply_, nullptr)(outcome);
  }
