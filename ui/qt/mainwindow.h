#ifndef BRAILLE_MUSIC_EDITOR_H
#define BRAILLE_MUSIC_EDITOR_H

#include <QMainWindow>
#include <QMap>
#include <QPointer>
#include <QSoundEffect>
#include <QtSvg>

#include <config.hpp>
#include <bmc/braille/ast/ast.hpp>
#include <boost/optional.hpp>

QT_BEGIN_NAMESPACE
class QAction;
class QTextEdit;
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

private slots:
    void fileNew();
    void fileOpen();
    bool fileSave();
    bool fileSaveAs();
    void fileCompile();
    void fileExportMusicXML();
    void filePrint();
    void filePrintPreview();
    void filePrintPdf();

    void textBold();
    void textUnderline();
    void textItalic();
    void textFamily(const QString &f);
    void textSize(const QString &p);
    void textColor();
    void textAlign(QAction *a);

    void currentCharFormatChanged(const QTextCharFormat &format);
    void cursorPositionChanged();
    void textChanged();

    void clipboardDataChanged();
    void about();
    void printPreview(QPrinter *);
    void runLilyPond(bool);

signals:
    void scoreAvailable(bool);

private:
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void fontChanged(const QFont &f);
    void colorChanged(const QColor &c);
    void alignmentChanged(Qt::Alignment a);

    QAction *actionSave;
    QAction *actionTextBold;
    QAction *actionTextUnderline;
    QAction *actionTextItalic;
    QAction *actionTextColor;
    QAction *actionAlignLeft;
    QAction *actionAlignCenter;
    QAction *actionAlignRight;
    QAction *actionAlignJustify;
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

    QSoundEffect ok, fail;
};

#endif // BRAILLE_MUSIC_EDITOR_H
