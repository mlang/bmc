#include "lilypondsvgwidget.h"

#include <QSvgRenderer>
#include <QDomDocument>
#include <QFile>
#include <QDebug>
#include <QApplication>

LilyPondSvgWidget::LilyPondSvgWidget(QString const &lilypondCode)
: lilypondCode{lilypondCode}, _oldid(-1) {
  connect(this, SIGNAL(noteClicked(int)), this, SLOT(hilightNote(int)));
}

void LilyPondSvgWidget::load(const QString filename) {
  QFile file(filename);

  if (!file.open(QIODevice::ReadOnly) || !doc.setContent(&file)) {
    qDebug() << "error opening file!";
  }

  QDomNodeList links = doc.elementsByTagName("a");

  QHash<int, int> ids;
  int link_id = 0;
  for (int i = 0; i < links.size(); i++) {
    QDomElement a = links.item(i).toElement();

    if (a.attribute("xlink:href").startsWith("textedit://")) {
      QDomElement p = a.firstChildElement("path");

      if (p.hasAttribute("transform")) {
        QString href = a.attribute("xlink:href");
        int pos = href.lastIndexOf(':');
        if (pos > 0) {
          int column = href.mid(pos + 1).toInt();
          pos = href.lastIndexOf(':', pos - 1);
          if (pos > 0) {
            int character = href.mid(pos + 1, href.indexOf(':', pos + 1) - pos - 1).toInt();
            pos = href.lastIndexOf(':', pos - 1);;
            if (pos > 0) {
              int line = href.mid(pos + 1, href.indexOf(':', pos + 1) - pos - 1).toInt();
              int id = find_id(line, character, column);
              if (id != -1) {
                p.setAttribute("id", link_id);
                ids.insert(link_id++, id);
              }
            }
          }
        }
      }
    }
  }

  QByteArray bytearray = doc.toString().toLocal8Bit();

  QSvgWidget::load(bytearray);

  for (auto i = ids.begin(); i != ids.end(); ++i) {
    rects.push_back(qMakePair(renderer()->boundsOnElement(QString::number(i.key())),
                              i.value()));
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

  for (auto pair: rects) {
    if (pair.first.contains(scaled_x, scaled_y)) {
      emit clicked(pair.second);
      break;
    }
  }
}

int LilyPondSvgWidget::find_id(int line, int character, int column) {
  if (line > 0) {
    QString lineText = lilypondCode.section('\n', line - 1, line - 1);
    int begin = lineText.indexOf("%{");
    if (begin != -1) {
      int end = lineText.indexOf("%}", begin);
      if (end > begin) {
        begin += 2;
        return lineText.mid(begin, end - begin).toInt();
      }
    }
  }
  return -1;
}
