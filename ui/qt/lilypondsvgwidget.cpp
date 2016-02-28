#include "lilypondsvgwidget.h"

#include <QSvgRenderer>
#include <QDomDocument>
#include <QFile>
#include <QDebug>
#include <QApplication>
#include <QString>
#include <QByteArray>

LilyPondSvgWidget::LilyPondSvgWidget(QString const &lilypondCode)
: lilypondCode{lilypondCode}, _oldid(-1) {
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

  QByteArray bytearray = doc.toByteArray();

  QSvgWidget::load(bytearray);

  for (auto i = ids.begin(); i != ids.end(); ++i) {
    rects.push_back(qMakePair(renderer()->boundsOnElement(QString::number(i.key())),
                              i.value()));
  }
}

void LilyPondSvgWidget::mouseMoveEvent(QMouseEvent *event) {
  float factor_x = this->renderer()->viewBoxF().width() / (this->width());
  float factor_y = this->renderer()->viewBoxF().height() / (this->height());

  float scaled_x = factor_x * event->x();
  float scaled_y = factor_y * event->y();

  for (auto &&pair : rects) {
    if (pair.first.contains(scaled_x, scaled_y)) {
      // ok, over a note

      if (!(pair.second == this->_oldid)) { // new note
        this->_oldid = pair.second;
        emit noteHovered(pair.second);

        return;
      }

    } else {
      if (!(this->_oldid == -1)) {
        this->_oldid = -1;
        emit noteHovered(-1);
      }
    }
  }
}

void LilyPondSvgWidget::mousePressEvent(QMouseEvent *event) {
  float factor_x = this->renderer()->viewBoxF().width() / (this->width());
  float factor_y = this->renderer()->viewBoxF().height() / (this->height());

  float scaled_x = factor_x * event->x();
  float scaled_y = factor_y * event->y();

  for (auto &&pair : rects) {
    if (pair.first.contains(scaled_x, scaled_y)) {
      emit clicked(pair.second);
      return;
    }
  }
}

int LilyPondSvgWidget::find_id(int line, int character, int column) {
  static QString const commentBegin = "%{", commentEnd = "%}";

  if (line > 0) {
    QString const lineText = lilypondCode.section('\n', line - 1, line - 1);
    int begin = lineText.indexOf(commentBegin, character);
    if (begin != -1) {
      begin += commentBegin.length();
      int end = lineText.indexOf(commentEnd, begin);
      if (end > begin) {
        return lineText.mid(begin, end - begin).toInt();
      }
    }
  }
  return -1;
}
