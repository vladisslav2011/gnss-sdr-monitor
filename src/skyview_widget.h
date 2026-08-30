/*!
 * \file skyview_widget.h
 * \brief Interface of a widget that shows satellite positions as
 * reported by the receiver.
 *
 * \author vladisslav2011(at)gmail.com
 *
 * -----------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019  (see AUTHORS file for a list of contributors)
 *
 * GNSS-SDR is a software defined Global Navigation
 *      Satellite Systems receiver
 *
 * This file is part of GNSS-SDR.
 *
 * GNSS-SDR is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GNSS-SDR is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNSS-SDR. If not, see <https://www.gnu.org/licenses/>.
 *
 * -----------------------------------------------------------------------
 */


#ifndef GNSS_SDR_MONITOR_SKYVIEW_WIDGET_H_
#define GNSS_SDR_MONITOR_SKYVIEW_WIDGET_H_

#include <QtGui>
#include <QFrame>

class SkyViewWidget : public QFrame
{
    Q_OBJECT

public:
    explicit SkyViewWidget(QWidget *parent = nullptr);
    QSize minimumSizeHint() const;
    QSize sizeHint() const;

public slots:
    void addData(int prn, qreal az, qreal el, std::string System, std::string Signal, bool combined, qreal snr, int channel);
    void redraw();
    void clear();

protected:
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent * event) override;

private:
    struct int_data
    {
        int prn;
        qreal az;
        qreal el;
        std::string System;
        std::string Signal;
        bool combined;
        qreal snr;
        int channel;
        qreal x;
        qreal y;
    };
    void draw(QPainter &painter);
    void drawOverlay(QPainter &painter);
    static bool isPointCloseTo(QPoint pt, int x, int y, int delta)
    {
        return ((pt.x() > (x - delta)) && (pt.x() < (x + delta)) && (pt.y() > (y - delta)) && (pt.y() < (y + delta)));
    }
    void showToolTip(QMouseEvent* event, QString toolTipText);

    QList<int_data> m_items{};
    QFont   m_font;
    qreal   m_fontSize{0};
    qreal   m_topMargin{0};
    qreal   m_leftMargin{0};
    qreal   m_hline{0};
    qreal   m_width{0};
    qreal   m_clientSize{0};
    qreal   m_height{0};
    qreal   m_marg{20};
};


#endif  // GNSS_SDR_MONITOR_SKYVIEW_WIDGET_H_
