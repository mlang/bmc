#include <QtWidgets>

#include <OptionsDialog.h>
#include <QFileInfo>
#include <QDebug>

OptionsDialog::OptionsDialog(QWidget *parent)
    : QDialog(parent)
{

  tabWidget = new QTabWidget;
  LilypondTab *lilypondTab=new LilypondTab();
  TimidityTab *timidityTab=new TimidityTab();      
  qDebug() << "od const";
  tabWidget->addTab(lilypondTab,tr("Lilypond settings"));
  tabWidget->addTab(timidityTab,tr("Timidity settings"));
  tabs.append(lilypondTab);
  tabs.append(timidityTab);
  
  buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

      connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Options"));
    tabWidget->show();
}


void OptionsDialog::accept() {

  this->done(QDialog::Accepted);
 }

void OptionsDialog::reject() {
  this->done(QDialog::Rejected);
}


void OptionsDialog::saveSettings() {

for (int i=0; i< tabs.size();i++)
  {
tabs.at(i)->persistSettings();
}
settings.sync();
}





TimidityTab::TimidityTab(QWidget *parent)
  : GenericTab(parent)
{
  this->setup();
}


void TimidityTab::persistSettings()
{
  qDebug() << "persisting timidity settings";
  settings.setValue("timidity/executable",QString(timidityExeEdit->text()));
}





void TimidityTab::setup() {
    timidityExeLabel = new QLabel(tr("timidity executable:"));
    timidityExeEdit = new QLineEdit(settings.value("timidity/executable",QString("foo/bat/bat12")).toString() );
    timidityExeLabel->setBuddy(timidityExeEdit);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(timidityExeLabel);
    mainLayout->addWidget(timidityExeEdit);

    mainLayout->addStretch(1);
    setLayout(mainLayout);
}



void LilypondTab::setup() {

    lilypondExeLabel = new QLabel(tr("lilypond executable:"));
    lilypondExeEdit = new QLineEdit(settings.value("lilypond/executable",QString("foo/bat/bat12")).toString() );
    lilypondExeLabel->setBuddy(lilypondExeEdit);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(lilypondExeLabel);
    mainLayout->addWidget(lilypondExeEdit);

    lilypondVerboseLabel = new QLabel(tr("verbose"));
    lilypondVerboseCheckbox = new QCheckBox();
    lilypondVerboseCheckbox->setCheckState(Qt::CheckState(settings.value("lilypond/verbose",Qt::Unchecked).toInt()) );
    
    mainLayout->addWidget(lilypondVerboseLabel);
    mainLayout->addWidget(lilypondVerboseCheckbox);
    lilypondVerboseLabel->setBuddy(lilypondVerboseCheckbox);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
    
   
}


LilypondTab::LilypondTab(QWidget *parent)
  : GenericTab(parent)
{
  this->setup();
 
}


void LilypondTab::persistSettings()
{
  qDebug() << "persisting lilypond settings";
  settings.setValue("lilypond/executable",QString(lilypondExeEdit->text()));

  settings.setValue("lilypond/verbose",(int)(lilypondVerboseCheckbox->checkState()));
}
