#include "config.h"
#include <QtWidgets>

#include <optionsdialog.h>

#include <QFileInfo>
#include <QDebug>

OptionsDialog::OptionsDialog(QWidget *parent): QDialog(parent) {
  tabWidget = new QTabWidget;

  auto lilypondTab = new LilypondTab;
  auto timidityTab = new TimidityTab;
  auto uisettingsTab = new UISettingsTab();

  tabWidget->addTab(lilypondTab, tr("Lilypond settings"));
  tabWidget->addTab(uisettingsTab, tr("UI settings"));
  tabWidget->addTab(timidityTab, tr("Timidity settings"));

  buttonBox =
    new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

  connect(buttonBox, &QDialogButtonBox::accepted,
          this, &OptionsDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected,
          this, &OptionsDialog::reject);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(tabWidget);
  mainLayout->addWidget(buttonBox);
  setLayout(mainLayout);

  setWindowTitle(tr("Options"));
  tabWidget->show();
}

void OptionsDialog::saveSettings() {
  for (int i = 0; i < tabWidget->count(); i++) {
    qobject_cast<GenericTab *>(tabWidget->widget(i))->persistSettings();
  }
  settings.sync();
}

TimidityTab::TimidityTab(QWidget *parent) : GenericTab(parent) {
  setup();
}

void TimidityTab::persistSettings() {
  settings.setValue("timidity/executable", QString(timidityExeEdit->text()));
}

void TimidityTab::setup() {
  timidityExeLabel = new QLabel(tr("timidity executable:"));
  timidityExeEdit = new QLineEdit(
    settings.value("timidity/executable", QString("foo/bat/bat12")).toString());
  timidityExeLabel->setBuddy(timidityExeEdit);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(timidityExeLabel);
  mainLayout->addWidget(timidityExeEdit);

  mainLayout->addStretch(1);
  setLayout(mainLayout);
}

void LilypondTab::setup() {
  lilypondExeLabel = new QLabel(tr("lilypond executable:"));
  lilypondExeEdit = new QLineEdit(
    settings.value("lilypond/executable", QString(LILYPOND_EXECUTABLE)).toString());
  lilypondExeLabel->setBuddy(lilypondExeEdit);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(lilypondExeLabel);
  mainLayout->addWidget(lilypondExeEdit);

  lilypondVerboseLabel = new QLabel(tr("verbose"));
  lilypondVerboseCheckbox = new QCheckBox();
  lilypondVerboseCheckbox->setCheckState(
    Qt::CheckState(settings.value("lilypond/verbose", Qt::Unchecked).toInt()));

  mainLayout->addWidget(lilypondVerboseLabel);
  mainLayout->addWidget(lilypondVerboseCheckbox);
  lilypondVerboseLabel->setBuddy(lilypondVerboseCheckbox);
  mainLayout->addStretch(1);
  setLayout(mainLayout);
}

LilypondTab::LilypondTab(QWidget *parent) : GenericTab(parent) {
  setup();
}

void LilypondTab::persistSettings() {
  settings.setValue("lilypond/executable", QString(lilypondExeEdit->text()));

  settings.setValue("lilypond/verbose",
                    (int)(lilypondVerboseCheckbox->checkState()));
}

void UISettingsTab::showColorChooser() {
  QColor col = QColorDialog::getColor(*highlightingColor, this);
  if (!col.isValid()) return;

  highlightingColor->setRgba(col.rgba());
  highlightingPixmap->fill(col);
  highlightingColorPushButton->setIcon(*highlightingPixmap);
  highlightingColorPushButton->setIconSize(highlightingPixmap->rect().size());
}

void UISettingsTab::setup() {
  persistLayoutCheckBox = new QCheckBox(tr("Persist Layout"));
  persistLayoutCheckBox->setCheckState(
    Qt::CheckState(settings.value("ui/persist_layout", Qt::Checked).toInt()));

  persistWindowPosCheckBox = new QCheckBox(tr("Persist Window Position"));
  persistWindowPosCheckBox->setCheckState(Qt::CheckState(
    settings.value("ui/persist_window_position", Qt::Checked).toInt()));

  highlightingColorPushButton = new QPushButton("Highlighting color...");
  highlightingPixmap = new QPixmap(32, 32);
  highlightingColor = new QColor(230, 230, 250);
  ;
  if (settings.value("ui/highlighting_color").toUInt() > 0) {
    highlightingColor->setRgba(
      settings.value("ui/highlighting_color").toUInt());
  }
  highlightingPixmap->fill(*highlightingColor);
  highlightingColorPushButton->setIcon(*highlightingPixmap);
  highlightingColorPushButton->setIconSize(highlightingPixmap->rect().size());

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(persistLayoutCheckBox);
  mainLayout->addWidget(persistWindowPosCheckBox);
  mainLayout->addWidget(highlightingColorPushButton);

  connect(highlightingColorPushButton, SIGNAL(released()), this,
          SLOT(showColorChooser()));
  setLayout(mainLayout);
}

UISettingsTab::UISettingsTab(QWidget *parent) : GenericTab(parent) { setup(); }

void UISettingsTab::persistSettings() {
  QSettings settings;
  settings.beginGroup("ui");

  settings.setValue("persist_layout",
                    (int)(persistLayoutCheckBox->checkState()));
  settings.setValue("persist_window_position",
                    (int)(persistWindowPosCheckBox->checkState()));
  settings.setValue("highlighting_color",
                    (unsigned int)highlightingColor->rgba());
}
