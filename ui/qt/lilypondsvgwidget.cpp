#include "lilypondsvgwidget.h"

#include <QSvgRenderer>
#include <QDomDocument>
#include <QFile>
#include <QDebug>
#include <QApplication>

LilyPondSvgWidget::LilyPondSvgWidget() : _oldid(-1) {
  connect(this, SIGNAL(noteClicked(int)), this, SLOT(hilightNote(int)));
}

void LilyPondSvgWidget::load(const QString filename) {
  QFile file(filename);

  if (!file.open(QIODevice::ReadOnly) || !doc.setContent(&file)) {
    qDebug() << "error opening file!";
  }

  QDomNodeList links = doc.elementsByTagName("a");

  int link_id = 0;

  for (int i = 0; i < links.size(); i++) {
    QDomElement a = links.item(i).toElement();

    if (a.attribute("href").startsWith("textedit://")) {
      QDomElement p = a.firstChildElement("path");

      if (p.hasAttribute("transform")) {
        //	    qDebug() << "id:"<<i<<"Child p:"<<p.attribute("transform");
        p.setAttribute("id", link_id);
        link_id++;
      }
    }
  }

  // qDebug()<<doc.toString();

  QByteArray bytearray = doc.toString().toLocal8Bit();

  //   qDebug()<< "loading stuff!!";
  QSvgWidget::load(bytearray);

  for (int i = 0; i < link_id; i++) {
    this->rects.append(this->renderer()->boundsOnElement(QString::number(i)));

    //  qDebug()<< "adding bounds for id "<< i << ":"<<
    //  r.boundsOnElement(QString::number(i));
  }
}

void LilyPondSvgWidget::mousePressEvent(QMouseEvent *event) {
  qDebug() << "Pos:" << event->pos();
  qDebug() << "size: " << this->width() << "x" << this->height();
  float factor_x = this->renderer()->viewBoxF().width() / (this->width());
  float factor_y = this->renderer()->viewBoxF().height() / (this->height());

  float scaled_x = factor_x * event->x();
  float scaled_y = factor_y * event->y();

  qDebug() << "scaled:" << scaled_x << "," << scaled_y;

  for (int i = 0; i < rects.size(); i++) {
    if (rects[i].contains(scaled_x, scaled_y)) {
      emit clicked(i);
      break;
    }
  }
}
