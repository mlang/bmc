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
  }

public:
  LilyPondSvgContainer(QStringList svgFiles) {
    auto vbox = new QVBoxLayout;
    for (auto &&fileName: svgFiles) {
      auto svgWidget = new LilyPondSvgWidget;
      svgWidget->load(fileName);
      vbox->addWidget(svgWidget);
    }
    setLayout(vbox);
  }
};

#endif
