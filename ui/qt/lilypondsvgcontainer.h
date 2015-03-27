#ifndef LILYPOND_SVG_CONTAINER_H
#define LILYPOND_SVG_CONTAINER_H

#include <QStringList>
#include <QVBoxLayout>
#include <QWidget>

#include "lilypondsvgwidget.h"

class LilyPondSvgContainer: public QWidget
{
  Q_OBJECT

signals:
  void clicked(int);

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

public:
  LilyPondSvgContainer(QStringList svgFiles) {
    auto vbox = new QVBoxLayout;
    for (auto &&fileName: svgFiles) {
      auto svgWidget = new LilyPondSvgWidget;
      connect(svgWidget, SIGNAL(clicked(int)), this, SLOT(click(id)));
      svgWidget->load(fileName);
      vbox->addWidget(svgWidget);
    }
    setLayout(vbox);
  }
};

#endif
