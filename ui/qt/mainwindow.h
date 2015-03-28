#ifndef BRAILLE_MUSIC_EDITOR_H
#define BRAILLE_MUSIC_EDITOR_H

#include <QMainWindow>
#include <QMap>
#include <QPointer>
#include <QProcess>
#include <QScrollArea>
#include <QSoundEffect>
#include <QTemporaryDir>
#include <QTextBlock>
#include <QTextEdit>

#include <config.hpp>
#include <bmc/braille/ast/ast.hpp>
#include <boost/optional.hpp>

#include "lilypondsvgcontainer.h"

QT_BEGIN_NAMESPACE
class QAction;
class QTextCharFormat;
class QMenu;
class QPrinter;
QT_END_NAMESPACE

class BrailleMusicEditor : public QMainWindow
{
    Q_OBJECT

public:
    BrailleMusicEditor(QWidget *parent = 0);

protected:
    virtual void closeEvent(QCloseEvent *e) Q_DECL_OVERRIDE;

private:
    void setupSoundEffects();
    void setupFileActions();
    void setupEditActions();
    void setupTextActions();
    bool load(const QString &f);
    bool maybeSave();
    void setCurrentFileName(const QString &fileName);
    void goTo(int line, int column) {
      QTextBlock block = textEdit->document()->findBlockByLineNumber(line - 1);
      if (block.isValid()) {
        QTextCursor cursor = textEdit->textCursor();
        cursor.setPosition(block.position() + column - 1);
        textEdit->setTextCursor(cursor);
      }
    }

private slots:
    void fileNew();
    void fileOpen();
    bool fileSave();
    bool fileSaveAs();
    void fileCompile();
    void fileExportMusicXML();
    void fileExportLilyPond();
    void filePrint();
    void filePrintPreview();
    void filePrintPdf();

    void textBold();
    void textUnderline();
    void textItalic();
    void textFamily(const QString &f);
    void textSize(const QString &p);
    void textColor();

    void currentCharFormatChanged(const QTextCharFormat &format);
    void cursorPositionChanged();
    void textChanged();

    void clipboardDataChanged();
    void about();
    void reportBug();
    void printPreview(QPrinter *);
    void runLilyPond(bool);
    void lilypondFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void lilypondError(QProcess::ProcessError error);

    void goToObject(int id);

signals:
    void scoreAvailable(bool);

private:
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void fontChanged(const QFont &f);
    void colorChanged(const QColor &c);

    QAction *actionSave;
    QAction *actionTextBold;
    QAction *actionTextUnderline;
    QAction *actionTextItalic;
    QAction *actionTextColor;
    QAction *actionUndo;
    QAction *actionRedo;
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;

    QToolBar *tb;
    QString fileName;
    QTextEdit *textEdit;
    QScrollArea *svgScrollArea;

    boost::optional<::bmc::braille::ast::score> score;
    QProcess lilypond;
    QTemporaryDir *tmpdir;
    QString lilypondCode;

    QSoundEffect ok, fail;
};

#endif // BRAILLE_MUSIC_EDITOR_H
