#ifndef LILYPOND_SVG_CONTAINER_H
#define LILYPOND_SVG_CONTAINER_H

#include <QStringList>
#include <QVBoxLayout>
#include <QWidget>

#include "lilypondsvgwidget.h"

class LilyPondSvgContainer: public QWidget
{
  Q_OBJECT

private:
  QString const &lilypondCode;

signals:
  void clicked(int);
  void noteHovered(int);

public slots:
  void highlight(int id) {
    for (int i = 0; i < layout()->count(); ++i) {
      if (LilyPondSvgWidget *widget = qobject_cast<LilyPondSvgWidget *>
                                      (layout()->itemAt(i)->widget())) {
        widget->hilightNote(id);
      }
    }
  }

private slots:
  void click(int id) { emit clicked(id); }
  void noteHover(int id) { emit noteHovered(id); }

public:
  LilyPondSvgContainer(QStringList svgFiles, QString const &lilypondCode)
  : lilypondCode{lilypondCode} {
    auto vbox = new QVBoxLayout;
    for (auto &&fileName : svgFiles) {
      auto svgWidget = new LilyPondSvgWidget{lilypondCode};
      connect(svgWidget, &LilyPondSvgWidget::clicked,
              this, &LilyPondSvgContainer::click);
      connect(svgWidget, &LilyPondSvgWidget::noteHovered,
              this, &LilyPondSvgContainer::noteHover);

      svgWidget->load(fileName);
      vbox->addWidget(svgWidget);
      svgWidget->setMouseTracking(true);
    }
    setLayout(vbox);
  }
};

#endif
