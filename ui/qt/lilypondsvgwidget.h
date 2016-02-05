#ifndef LILYPOND_SVG_WIDGET_H
#define LILYPOND_SVG_WIDGET_H

#include <QSvgWidget>
#include <QSvgRenderer>
#include <QDomDocument>
#include <QFile>
#include <QDebug>
#include <QApplication>
#include <QMouseEvent>

class LilyPondSvgWidget : public QSvgWidget {
  Q_OBJECT

private:
  QString const &lilypondCode;
  int find_id(int line, int character, int column);

signals:
  void clicked(int id);
  void noteHovered(int id);

public slots:
  void hilightNote(int id) {
    qDebug() << "old:" << _oldid << "id:" << id;
    qDebug() << "oldstyle:" << _oldFillStyle;

    if (id == _oldid) return;
    if (_oldid > 0) { _hilightNote(_oldid, false); }
    _hilightNote(id, true);
  }

private:
  QString _oldFillStyle;
  int _oldid = -1;

  void _hilightNote(int id, bool highlight) {
    QDomNodeList paths = doc.elementsByTagName("path");

    for (int i = 0; i < paths.size(); i++) {
      if (QString(paths.item(i).toElement().attribute("id")) ==
          QString::number(id)) {
        if (highlight) {
          qDebug() << "highlighting id " << id;
          _oldFillStyle = paths.item(i).toElement().attribute("fill");

          if (_oldFillStyle == "currentColor" || _oldFillStyle == "") {
            _oldFillStyle = "#000000";
          }
          _oldid = id;
          qDebug() << "Setting oldid to " << i;
          paths.item(i).toElement().setAttribute("fill", "#FF0000");

        } else {
          if (this->_oldFillStyle == QString("")) {
            this->_oldFillStyle = "#000000";
          }
          qDebug() << "restoring id " << _oldid
                   << ", style:" << this->_oldFillStyle;

          paths.item(i).toElement().setAttribute("fill", this->_oldFillStyle);
        }

        QByteArray bytearray = doc.toByteArray();

        //      qDebug()<< "reloading stuff!!";
        QSvgWidget::load(bytearray);

        this->update();
        break;
      }
    }
  }

public:
  QList<QPair<QRectF, int>> rects;
  QDomDocument doc;

  LilyPondSvgWidget(QString const &lilypondCode);

  void load(const QString filename);
  void mouseMoveEvent(QMouseEvent *event); // { mousePressEvent(event); }

  void mousePressEvent(QMouseEvent *event);
};

#endif
