#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QSettings>
#include <QFileInfo>
#include <QLineEdit>
#include <QLabel>
#include <QList>
#include <QCheckBox>

class GenericTab : public QWidget {
  Q_OBJECT

public:
  explicit GenericTab(QWidget *parent) : QWidget(parent) {}

  virtual void persistSettings() = 0;
  virtual void setup() = 0;

protected:
  QSettings settings;
};

class LilypondTab : public GenericTab {
  Q_OBJECT

public:
  explicit LilypondTab(QWidget *parent = 0);
  void persistSettings();
  void setup();

private:
  QLabel *lilypondExeLabel;
  QLineEdit *lilypondExeEdit;

  QLabel *lilypondVerboseLabel;
  QCheckBox *lilypondVerboseCheckbox;
};

class TimidityTab : public GenericTab {
  Q_OBJECT

public:
  explicit TimidityTab(QWidget *parent = 0);

  void persistSettings();
  void setup();

private:
  QLabel *timidityExeLabel;
  QLineEdit *timidityExeEdit;
};


class UISettingsTab : public GenericTab {
  Q_OBJECT

public:
  explicit UISettingsTab(QWidget *parent = 0);

  void persistSettings();
  void setup();

private:
  QCheckBox *persistLayoutCheckBox;
  QCheckBox *persistWindowPosCheckBox;
  QPushButton *highlightingColorPushButton;
  QColor *highlightingColor;
  QPixmap *highlightingPixmap;
private slots:
  void showColorChooser();
};

class OptionsDialog : public QDialog {
  Q_OBJECT

public:
  explicit OptionsDialog(QWidget *parent = 0);

  void saveSettings();

private:
  QTabWidget *tabWidget;
  QDialogButtonBox *buttonBox;

  QSettings settings;
};






#endif // OPTIONSDIALOG_H
