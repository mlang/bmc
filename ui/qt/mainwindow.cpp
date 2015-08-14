#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QColorDialog>
#include <QComboBox>
#include <QDesktopServices>
#include <QFontComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QInputDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QProcess>
#include <QSplitter>
#include <QTemporaryDir>
#include <QTextCodec>
#include <QTextCursor>
#include <QTextDocumentWriter>
#include <QTextEdit>
#include <QTextList>
#include <QToolBar>
#include <QtDebug>

#include "mainwindow.h"
#include "optionsdialog.h"

#include <boost/spirit/include/qi_parse.hpp>
#include <boost/spirit/include/qi_core.hpp>
#include <bmc/braille/parsing/grammar/score.hpp>
#include <bmc/braille/semantic_analysis.hpp>
#include <bmc/braille/reformat.hpp>
#include <bmc/musicxml.hpp>
#include <bmc/lilypond.hpp>
#include <bmc/braille/ast/visitor.hpp>

#ifdef Q_OS_MAC
const QString rsrcPath = ":/images/mac";
#else
const QString rsrcPath = ":/images/win";
#endif

BrailleMusicEditor::BrailleMusicEditor(QWidget *parent)
: QMainWindow(parent), ok(this), fail(this), lilypond(this) {
#ifdef Q_OS_OSX
  setUnifiedTitleAndToolBarOnMac(true);
#endif

  setToolButtonStyle(Qt::ToolButtonFollowStyle);
  setupFileActions();
  setupEditActions();
  setupOptionsActions();
  setupSoundEffects();

  {
    auto helpMenu = new QMenu(tr("Help"), this);
    menuBar()->addMenu(helpMenu);
    helpMenu->addAction(tr("About"), this, SLOT(about()));
    helpMenu->addAction(tr("About &Qt"), qApp, SLOT(aboutQt()));
    helpMenu->addAction(tr("&Report a problem..."), this, SLOT(reportBug()));
  }

  textEdit = new QTextEdit(this);
  textEdit->setWordWrapMode(QTextOption::NoWrap);
  connect(textEdit, &QTextEdit::cursorPositionChanged,
          this, &BrailleMusicEditor::cursorPositionChanged);
  connect(textEdit, &QTextEdit::textChanged,
          this, &BrailleMusicEditor::textChanged);

  svgScrollArea = new QScrollArea(this);
  svgScrollArea->setFocusPolicy(Qt::NoFocus);

  auto splitter = new QSplitter(Qt::Vertical);
  splitter->addWidget(textEdit);
  splitter->addWidget(svgScrollArea);

  QSettings settings;
  if (settings.value("ui/persist_layout",1).toInt() &&
      settings.contains("ui/layout_data")) {
	  splitter->restoreState(settings.value("ui/layout_data").toByteArray());
    } else {
      splitter->setStretchFactor(0, 1);
      splitter->setStretchFactor(1, 1);
    }
  setCentralWidget(splitter);
 
  textEdit->setFocus();
  setCurrentFileName(QString());

  QFont textFont("Helvetica");
  textFont.setStyleHint(QFont::SansSerif);
  textEdit->setFont(textFont);
  fontChanged(textEdit->font());
  colorChanged(textEdit->textColor());

  connect(textEdit->document(), &QTextDocument::modificationChanged,
          actionSave, &QAction::setEnabled);
  connect(textEdit->document(), &QTextDocument::modificationChanged,
          this, &BrailleMusicEditor::setWindowModified);
  connect(textEdit->document(), &QTextDocument::undoAvailable,
          actionUndo, &QAction::setEnabled);
  connect(textEdit->document(), &QTextDocument::redoAvailable,
          actionRedo, &QAction::setEnabled);

  setWindowModified(textEdit->document()->isModified());
  actionSave->setEnabled(textEdit->document()->isModified());
  actionUndo->setEnabled(textEdit->document()->isUndoAvailable());
  actionRedo->setEnabled(textEdit->document()->isRedoAvailable());

  connect(actionUndo, &QAction::triggered, textEdit, &QTextEdit::undo);
  connect(actionRedo, &QAction::triggered, textEdit, &QTextEdit::redo);

  actionCut->setEnabled(false);
  actionCopy->setEnabled(false);

  connect(actionCut, &QAction::triggered, textEdit, &QTextEdit::cut);
  connect(actionCopy, &QAction::triggered, textEdit, &QTextEdit::copy);
  connect(actionPaste, &QAction::triggered, textEdit, &QTextEdit::paste);

  connect(textEdit, &QTextEdit::copyAvailable, actionCut, &QAction::setEnabled);
  connect(textEdit, &QTextEdit::copyAvailable, actionCopy, &QAction::setEnabled);

#ifndef QT_NO_CLIPBOARD
  connect(QApplication::clipboard(), SIGNAL(dataChanged()),
          this, SLOT(clipboardDataChanged()));
#endif

  connect(&lilypond, &QProcess::started,
          this, &BrailleMusicEditor::lilyPondStarted);
  connect(&lilypond, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>
                     (&QProcess::finished),
          this, &BrailleMusicEditor::lilyPondFinished);
  connect(&lilypond, static_cast<void (QProcess::*)(QProcess::ProcessError)>
                     (&QProcess::error),
          this, &BrailleMusicEditor::lilyPondError);

  QString initialFile = ":/examples/bwv988-v01.bmc";
  const QStringList args = QCoreApplication::arguments();
  if (args.count() == 2) initialFile = args.at(1);

  if (!load(initialFile)) fileNew();
}

void BrailleMusicEditor::closeEvent(QCloseEvent *e) {
  
  QSettings settings;
  if (settings.value("ui/persist_layout",1).toInt())
    {
      settings.beginGroup("ui");
      settings.setValue("layout_data",(qobject_cast<QSplitter*>)(centralWidget())->saveState());
      settings.endGroup();
      settings.sync();
    }

  if (maybeSave())
    {
      e->accept();
    }
  else
    e->ignore();
}

void BrailleMusicEditor::setupSoundEffects() {
  fail.setSource(QUrl("qrc:///sounds/fail.wav"));
  ok.setSource(QUrl("qrc:///sounds/ok.wav"));
}

void BrailleMusicEditor::setupFileActions() {
  auto tb = new QToolBar(this);
  tb->setWindowTitle(tr("File Actions"));
  addToolBar(tb);

  auto menu = new QMenu(tr("&File"), this);
  menuBar()->addMenu(menu);

  QAction *a;

  auto newIcon = QIcon::fromTheme("document-new");
  a = new QAction(newIcon, tr("&New"), this);
  a->setPriority(QAction::LowPriority);
  a->setShortcut(QKeySequence::New);
  connect(a, &QAction::triggered, this, &BrailleMusicEditor::fileNew);
  tb->addAction(a);
  menu->addAction(a);

  a = new QAction(QIcon::fromTheme("document-open"), tr("&Open..."), this);
  a->setShortcut(QKeySequence::Open);
  connect(a, &QAction::triggered, this, &BrailleMusicEditor::fileOpen);
  tb->addAction(a);
  menu->addAction(a);

  menu->addSeparator();

  actionSave = a =
    new QAction(QIcon::fromTheme("document-save"), tr("&Save"), this);
  a->setShortcut(QKeySequence::Save);
  connect(a, &QAction::triggered, this, &BrailleMusicEditor::fileSave);
  a->setEnabled(false);
  tb->addAction(a);
  menu->addAction(a);

  a = new QAction(tr("Save &As..."), this);
  a->setPriority(QAction::LowPriority);
  connect(a, &QAction::triggered, this, &BrailleMusicEditor::fileSaveAs);
  menu->addAction(a);

  menu->addSeparator();

  a = new QAction(tr("&Compile"), this);
  a->setPriority(QAction::LowPriority);
  a->setShortcut(Qt::Key_F8);
  connect(a, &QAction::triggered, this, &BrailleMusicEditor::fileCompile);
  menu->addAction(a);

  a = new QAction(tr("Export &MusicXML"), this);
  a->setPriority(QAction::LowPriority);
  connect(a, &QAction::triggered, this, &BrailleMusicEditor::fileExportMusicXML);
  a->setEnabled(false);
  connect(this, SIGNAL(scoreAvailable(bool)), a, SLOT(setEnabled(bool)));
  menu->addAction(a);

  a = new QAction(tr("Export &LilyPond"), this);
  a->setPriority(QAction::LowPriority);
  connect(a, &QAction::triggered, this, &BrailleMusicEditor::fileExportLilyPond);
  a->setEnabled(false);
  connect(this, SIGNAL(scoreAvailable(bool)), a, SLOT(setEnabled(bool)));
  menu->addAction(a);

  menu->addSeparator();

  a = new QAction(tr("&Quit"), this);
  a->setShortcut(Qt::CTRL + Qt::Key_Q);
  connect(a, &QAction::triggered, this, &BrailleMusicEditor::close);
  menu->addAction(a);

  connect(this, &BrailleMusicEditor::scoreAvailable,
          this, &BrailleMusicEditor::runLilyPond);
}

void BrailleMusicEditor::setupEditActions() {
  auto tb = new QToolBar(this);
  tb->setWindowTitle(tr("Edit Actions"));
  addToolBar(tb);
  auto menu = new QMenu(tr("&Edit"), this);
  menuBar()->addMenu(menu);

  QAction *a;
  a = actionUndo =
    new QAction(QIcon::fromTheme("edit-undo"), tr("&Undo"), this);
  a->setShortcut(QKeySequence::Undo);
  tb->addAction(a);
  menu->addAction(a);
  a = actionRedo =
    new QAction(QIcon::fromTheme("edit-redo"), tr("&Redo"), this);
  a->setPriority(QAction::LowPriority);
  a->setShortcut(QKeySequence::Redo);
  tb->addAction(a);
  menu->addAction(a);
  menu->addSeparator();
  a = actionCut = new QAction(QIcon::fromTheme("edit-cut"), tr("Cu&t"), this);
  a->setPriority(QAction::LowPriority);
  a->setShortcut(QKeySequence::Cut);
  tb->addAction(a);
  menu->addAction(a);
  a = actionCopy =
    new QAction(QIcon::fromTheme("edit-copy"), tr("&Copy"), this);
  a->setPriority(QAction::LowPriority);
  a->setShortcut(QKeySequence::Copy);
  tb->addAction(a);
  menu->addAction(a);
  a = actionPaste =
    new QAction(QIcon::fromTheme("edit-paste"), tr("&Paste"), this);
  a->setPriority(QAction::LowPriority);
  a->setShortcut(QKeySequence::Paste);
  tb->addAction(a);
  menu->addAction(a);
#ifndef QT_NO_CLIPBOARD
  if (const QMimeData *md = QApplication::clipboard()->mimeData())
    actionPaste->setEnabled(md->hasText());
#endif
  menu->addSeparator();

  a = new QAction(tr("&Reformat"), this);
  a->setPriority(QAction::LowPriority);
  a->setShortcut(Qt::Key_F9);
  a->setEnabled(false);
  connect(this, &BrailleMusicEditor::scoreAvailable, a, &QAction::setEnabled);
  connect(a, &QAction::triggered, this, &BrailleMusicEditor::editReformat);
  tb->addAction(a);
  menu->addAction(a);
}

void BrailleMusicEditor::setupOptionsActions() {
  auto menu = new QMenu(tr("O&ptions"), this);
  menuBar()->addMenu(menu);
  QAction *settings = new QAction(tr("&Settings..."), this);
  connect(settings, SIGNAL(triggered()), this, SLOT(showOptions()));
  menu->addAction(settings);
}

bool BrailleMusicEditor::load(const QString &f) {
  if (!QFile::exists(f)) return false;
  QFile file(f);
  if (!file.open(QFile::ReadOnly)) return false;

  QByteArray data = file.readAll();
  QTextCodec *codec = QTextCodec::codecForName("UTF-8");
  QString str = codec->toUnicode(data);
  textEdit->setPlainText(str);

  setCurrentFileName(f);
  return true;
}

bool BrailleMusicEditor::maybeSave() {
  if (!textEdit->document()->isModified()) return true;

  QMessageBox::StandardButton ret;
  ret = QMessageBox::warning(
    this, tr("Application"), tr("The document has been modified.\n"
                                "Do you want to save your changes?"),
    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
  if (ret == QMessageBox::Save)
    return fileSave();
  else if (ret == QMessageBox::Cancel)
    return false;
  return true;
}

void BrailleMusicEditor::setCurrentFileName(const QString &fileName) {
  this->fileName = fileName;
  textEdit->document()->setModified(false);

  QString shownName;
  if (fileName.isEmpty())
    shownName = "untitled.txt";
  else
    shownName = QFileInfo(fileName).fileName();

  setWindowTitle(tr("%1[*] - %2").arg(shownName).arg(tr("Braille Music")));
  setWindowModified(false);
}

void BrailleMusicEditor::goTo(int line, int column) {
  QTextBlock block = textEdit->document()->findBlockByLineNumber(line - 1);
  if (block.isValid()) {
    QTextCursor cursor = textEdit->textCursor();
    cursor.setPosition(block.position() + column - 1);
    textEdit->setTextCursor(cursor);
    textEdit->ensureCursorVisible();
    textEdit->setFocus();
  }
}

void BrailleMusicEditor::showEmptySVG()
{
  auto widget = new QSvgWidget{":/images/empty.svg"};
  svgScrollArea->setWidget(widget);
  widget->show();

  auto scalefactor = (double)svgScrollArea->viewport()->width()/widget->width();
  widget->resize(svgScrollArea->viewport()->width(),(int)(widget->height()*scalefactor));
}

void BrailleMusicEditor::fileNew() {
  if (maybeSave()) {
    textEdit->clear();
    setCurrentFileName(QString());
    showEmptySVG();
  }
}

void BrailleMusicEditor::fileOpen() {
  QString fn = QFileDialog::getOpenFileName(
    this, tr("Open File..."), QString(),
    tr("Braille Music Files (*.bmc);;All Files (*)"));
  if (!fn.isEmpty()) load(fn);
}

bool BrailleMusicEditor::fileSave() {
  if (fileName.isEmpty()) return fileSaveAs();
  if (fileName.startsWith(QStringLiteral(":/"))) return fileSaveAs();

  QTextDocumentWriter writer(fileName);
  bool success = writer.write(textEdit->document());
  if (success) textEdit->document()->setModified(false);
  return success;
}

bool BrailleMusicEditor::fileSaveAs() {
  QString fn = QFileDialog::getSaveFileName(
    this, tr("Save as..."), QString(),
    tr("Braille music files (*.bmc);;LilyPond-Files "
       "(*.ly);;MusicXML Files (*.xml)"));
  if (fn.isEmpty()) return false;
  if (!(fn.endsWith(".bmc", Qt::CaseInsensitive) ||
        fn.endsWith(".ly", Qt::CaseInsensitive) ||
        fn.endsWith(".xml", Qt::CaseInsensitive))) {
    fn += ".bmc"; // default
  }
  setCurrentFileName(fn);
  return fileSave();
}

void BrailleMusicEditor::fileCompile() {
  std::wstring input;
#if !defined(MSVC)
  input = textEdit->toPlainText().toStdWString();
#else
  // MSVC crashes if we attempt to use toStdWString.
  input = (wchar_t const *)textEdit->toPlainText().utf16();
#endif
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type score;
  if (parse(begin, end, parser, score) && begin == end) {
    ::bmc::braille::compiler<error_handler_type> compile(errors);

    if (compile(score)) {
      ok.play();
      this->score = score;
      emit scoreAvailable(true);
      return;
    } else {
      fail.play();
      std::wstringstream ss;
      ss << errors;
      QMessageBox::critical(this, tr("Compilation error"),
                            QString::fromStdWString(ss.str()));
    }
  } else {
    fail.play();
    QMessageBox::critical(this, tr("Parse error"),
                          QString("Failed to parse braille music code."));
  }
  this->score = boost::none;
  emit scoreAvailable(false);
}

void BrailleMusicEditor::runLilyPond(bool scoreAvailable) {
  if (scoreAvailable) {
    BOOST_ASSERT(this->score);

    tmpdir.reset(new QTemporaryDir);
    lilypond.setWorkingDirectory(tmpdir->path());
    lilypond.setProcessChannelMode(QProcess::ForwardedChannels);
    QSettings settings;
    lilypond.start(settings.value("lilypond/executable", "lilypond").toString(),
                   QStringList() << "-o" << "out" << "-dbackend=svg" << "-");
  }
}

void BrailleMusicEditor::lilyPondStarted() {
  std::stringstream ss;
  ::bmc::lilypond::generator make_lilypond(ss, true, true, true);
  make_lilypond(*this->score);
  lilypond.write(ss.str().c_str());
  lilypondCode = QString::fromStdString(ss.str());
  lilypond.closeWriteChannel();
}

void BrailleMusicEditor::lilyPondFinished(int exitCode,
                                          QProcess::ExitStatus exitStatus) {
  if (exitStatus == QProcess::NormalExit) {
    QDir dir(tmpdir->path());
    dir.setNameFilters(QStringList() << "*.svg");

    QStringList svgFiles;
    for (auto &&path: dir.entryList()) {
      QFile svgFile{dir.absoluteFilePath(path)};
      if (svgFile.exists()) svgFiles << svgFile.fileName();
    }

    auto widget = new LilyPondSvgContainer{svgFiles, lilypondCode};
    connect(widget, SIGNAL(clicked(int)), this, SLOT(goToObject(int)));
    connect(widget, SIGNAL(noteHovered(int)), this, SLOT(highlightObject(int)));
    svgScrollArea->setWidget(widget);
    widget->setMouseTracking(true);
    widget->show();

    // scale svg display to match svgScrollarea width
    auto scalefactor =
      (double)svgScrollArea->viewport()->width() / widget->width();
    widget->resize(svgScrollArea->viewport()->width(),
                   (int)(widget->height() * scalefactor));
    ok.play();
  } else {
    lilyPondError(QProcess::Crashed);
  }
}

void BrailleMusicEditor::lilyPondError(QProcess::ProcessError error) {
  QMessageBox::critical(this, tr("LilyPond failed"),
                        tr("There was an error during execution of Lilypond."));

}

void BrailleMusicEditor::goToObject(int id) {
  qDebug() << "Clicked on object" << id;

  struct finder: public ::bmc::braille::ast::const_visitor<finder> {
    int target_id, line, column;

    finder(int target_id): target_id{target_id}, line{-1}, column{-1} {}

    bool visit_locatable(::bmc::braille::ast::locatable const &lexeme) {
      if (lexeme.id == target_id) {
        line = lexeme.begin_line;
        column = lexeme.begin_column;
        return false;
      }
      return true;
    }
  } find{id};

  if (score && !find.traverse_score(*score)) { goTo(find.line, find.column); }
  // this->highlightObject(id);
}

void BrailleMusicEditor::highlightObject(int id) {
  QList<QTextEdit::ExtraSelection> selections;
  QSettings settings;

  if (id == -1) // remove highlighting
  {
    textEdit->setExtraSelections(selections);
    textEdit->show();
    return;
  }

  struct finder : public ::bmc::braille::ast::const_visitor<finder> {
    int target_id, beg_line, beg_column, end_line, end_column;

    finder(int target_id)
    : target_id{target_id}
    , beg_line{-1}
    , beg_column{-1}
    , end_line{-1}
    , end_column{-1} {}

    bool visit_locatable(::bmc::braille::ast::locatable const &lexeme) {
      if (lexeme.id == target_id) {
        beg_line = lexeme.begin_line;
        beg_column = lexeme.begin_column;
        end_line = lexeme.end_line;
        end_column = lexeme.end_column;

        return false;
      }
      return true;
    }
  } find{id};

  QColor highlightingColor(230, 230, 250);

  if (auto rgba = settings.value("ui/highlighting_color").toUInt()) {
    highlightingColor.setRgba(rgba);
  }

  if (score && find.traverse_score(*score) == false) {
    QTextCharFormat format;
    format.setBackground(QBrush(highlightingColor));
    QTextBlock block =
      textEdit->document()->findBlockByLineNumber(find.beg_line - 1);
    if (block.isValid()) {
      QTextCursor cursor = textEdit->textCursor();
      cursor.setPosition(block.position() + find.beg_column - 1);
      cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor,
                          find.beg_line - find.end_line);
      cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                          find.end_column - find.beg_column);
      QTextEdit::ExtraSelection sel = {cursor, format};
      selections.append(sel);
    }
    textEdit->setExtraSelections(selections);
    textEdit->show();
  }
}

void BrailleMusicEditor::fileExportMusicXML() {
  BOOST_ASSERT(this->score);

  std::stringstream ss;
  ::bmc::musicxml(ss, *this->score);
  QString fn = QFileDialog::getSaveFileName(
    this, tr("Export MusicXML..."), QString(), tr("MusicXML files (*.xml)"));
  if (fn.isEmpty()) return;
  if (!fn.endsWith(".xml", Qt::CaseInsensitive)) {
    fn += ".xml"; // default
  }
  QFile file(fn);
  if (!file.open(QIODevice::WriteOnly)) { return; }
  QTextStream out(&file);
  out << QString::fromStdString(ss.str());
}

void BrailleMusicEditor::showOptions() {
  OptionsDialog optionsDialog;
  int retval = optionsDialog.exec();
  if (retval == QDialog::Accepted) { optionsDialog.saveSettings(); }
}

void BrailleMusicEditor::resizeEvent(QResizeEvent *event) {
  QMainWindow::resizeEvent(event);

  auto widget = svgScrollArea->widget();
  if (!widget) return;
  auto scalefactor =
    (double)svgScrollArea->viewport()->width() / widget->width();
  widget->resize(svgScrollArea->viewport()->width(),
                 (int)(widget->height() * scalefactor));
}

void BrailleMusicEditor::fileExportLilyPond() {
  BOOST_ASSERT(this->score);

  std::stringstream ss;
  ::bmc::lilypond::generator make_lilypond(ss);
  make_lilypond(*this->score);
  QString fn = QFileDialog::getSaveFileName(
    this, tr("Export LilyPond..."), QString(), tr("LilyPond files (*.ly)"));
  if (fn.isEmpty()) return;
  if (!fn.endsWith(".ly", Qt::CaseInsensitive)) {
    fn += ".ly"; // default
  }
  QFile file(fn);
  if (!file.open(QIODevice::WriteOnly)) { return; }
  QTextStream out(&file);
  out << QString::fromStdString(ss.str());
}

void BrailleMusicEditor::filePrint() {}

void BrailleMusicEditor::filePrintPreview() {}

void BrailleMusicEditor::printPreview(QPrinter *printer) { Q_UNUSED(printer); }

void BrailleMusicEditor::filePrintPdf() {}

void BrailleMusicEditor::editReformat() {
  bool ok = false;
  int columns {
    QInputDialog::getInt(this, tr("Reformat Braille"),
                         tr("Line width"), 32, 25, 88, 1, &ok)
  };
  if (ok) {
    bmc::braille::format_style style;
    style.columns = columns;
    std::stringstream ss;
    ss << bmc::braille::reformat(*score, style);
    textEdit->setPlainText(QString::fromStdString(ss.str()));
    scoreAvailable(false);
    fileCompile();
  }
}


void BrailleMusicEditor::textFamily(const QString &f) {
  QTextCharFormat fmt;
  fmt.setFontFamily(f);
  mergeFormatOnWordOrSelection(fmt);
}

void BrailleMusicEditor::textSize(const QString &p) {
  qreal pointSize = p.toFloat();
  if (p.toFloat() > 0) {
    QTextCharFormat fmt;
    fmt.setFontPointSize(pointSize);
    mergeFormatOnWordOrSelection(fmt);
  }
}

void BrailleMusicEditor::cursorPositionChanged() {}

void BrailleMusicEditor::textChanged() {
  this->score = boost::none;
  emit scoreAvailable(false);
}

void BrailleMusicEditor::clipboardDataChanged() {
#ifndef QT_NO_CLIPBOARD
  if (const QMimeData *md = QApplication::clipboard()->mimeData())
    actionPaste->setEnabled(md->hasText());
#endif
}

void BrailleMusicEditor::about() {
  QMessageBox::about(
    this, tr("About"),
    tr("This example demonstrates Qt's "
       "rich text editing facilities in action, providing an example "
       "document for you to experiment with."));
}

void BrailleMusicEditor::reportBug() {
  QDesktopServices::openUrl(QUrl{"mailto:mlang@blind.guru?subject=BMC"});
}

void BrailleMusicEditor::mergeFormatOnWordOrSelection(
  const QTextCharFormat &format) {
  QTextCursor cursor = textEdit->textCursor();
  if (!cursor.hasSelection()) cursor.select(QTextCursor::WordUnderCursor);
  cursor.mergeCharFormat(format);
  textEdit->mergeCurrentCharFormat(format);
}

void BrailleMusicEditor::fontChanged(const QFont &f) {
}

void BrailleMusicEditor::colorChanged(const QColor &c) {
  QPixmap pix(16, 16);
  pix.fill(c);
}
