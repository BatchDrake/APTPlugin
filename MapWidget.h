//
//    MapWidget.h: Map widget
//    Copyright (C) 2022 Gonzalo José Carracedo Carballal
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as
//    published by the Free Software Foundation, either version 3 of the
//    License, or (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful, but
//    WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this program.  If not, see
//    <http://www.gnu.org/licenses/>
//
#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>
#include <vector>

class MapWidget : public QWidget
{
    Q_OBJECT

  public:
    enum Channel {
      MAP_WIDGET_CHANNEL_A,
      MAP_WIDGET_CHANNEL_B
    };

  private:

    Channel channel = MAP_WIDGET_CHANNEL_A;

    QRgb colormap[256];

    bool haveGeometry = false;
    float pxToLine = 0;
    unsigned int lineOff = 0;
    bool autoScroll = true;
    bool falseColor = false;

    QPixmap mapPixmap;

    std::vector<uint8_t> channelAdata;
    std::vector<uint8_t> channelBdata;
    std::vector<QRgb>    falseColorData;

    void redrawChannel(Channel);
    void redrawChannels();
    void redrawFalseColor();
    void recalcFalseColor();

    std::vector<uint8_t> *getData(void);
    const std::vector<uint8_t> *getData(void) const;

    void paintEvent(QPaintEvent *ev) override;
    void resizeEvent(QResizeEvent *ev) override;

  public:
    explicit MapWidget(QWidget *parent = 0);

    void pushLine(Channel, const uint8_t *, size_t, bool highsnr = true);

    void setGradient(QColor const *gradient);

    void setFalseColorEnabled(bool);
    bool isFalseColorEnabled(void) const;

    void setAutoScrollEnabled(bool);
    bool isAutoScrollEnabled(void) const;

    void setCurrentChannel(Channel);
    Channel currentChannel(void) const;

    void setOffset(unsigned int);
    unsigned int offset(void) const;

    unsigned int lines(void) const;
    unsigned int viewPortLines(void) const;

    void saveTo(QString);

  signals:
    void geometryChanged();
    void offsetChanged(int);

  public slots:
};

#endif // MAPWIDGET_H
