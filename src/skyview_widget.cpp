/*!
 * \file skyview_widget.cpp
 * \brief Implementation of a widget that shows satellite positions as
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


#include "skyview_widget.h"
#include <QChart>
#include <QGraphicsLayout>
#include <QLayout>
#include <QToolTip>
#include <cmath>
#include <iostream>

/*!
 Constructs an skyview widget.
 */
SkyViewWidget::SkyViewWidget(QWidget *parent) : QFrame(parent)
{
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_font = QFont("Arial");
    m_fontSize = 10;
    // Debug 
    #if 0
    addData(1,55,38,"E","1B",0,40,0);
    addData(2,0,0,"E","1B",0,40,1);
    addData(3,180,1,"E","1B",0,20,2);
    addData(4,270,5,"E","1B",0,32,3);
    addData(5,180,90,"E","1B",0,40,4);
    #endif
}

QSize SkyViewWidget::minimumSizeHint() const
{
    return QSize(100, 30);
}

QSize SkyViewWidget::sizeHint() const
{
    return QSize(100, 30);
}

/*!
 Adds the new point to the widget's internal data structures.
 */
void SkyViewWidget::addData(int prn, qreal az, qreal el, std::string System, std::string Signal, bool combined, qreal snr, int channel)
{
    m_items.push_back({prn,az,el,System, Signal, combined,snr, channel});
}

/*!
 Redraws the chart by replacing the old data in the series object with the new data.
 */
void SkyViewWidget::redraw()
{
    update();
}

/*!
 Clears all the data from the widget's internal data structures.
 */
void SkyViewWidget::clear()
{
    m_items.clear();
}

void SkyViewWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    m_width = width();
    m_height = height();
    m_clientSize = std::min(m_width, m_height);
    m_topMargin = m_height > m_width ? (m_height - m_width) * 0.5 : 0.;
    m_leftMargin = m_height < m_width ? (m_width - m_height) * 0.5 : 0.;

    drawOverlay(painter);
    draw(painter);
    painter.end();
}

void SkyViewWidget::mouseMoveEvent(QMouseEvent* event)
{

    QPoint pt = event->pos();
    int_data * p = nullptr;
    int min_dist = 1000000;

    for( auto & it: m_items)
        if(isPointCloseTo(pt,it.x,it.y,5))
        {
            int dist=std::min(std::abs(pt.x()-it.x),std::abs(pt.y()-it.y));
            if(dist<min_dist)
            {
                min_dist = dist;
                p = & it;
            }
        }

    if(p)
    {
        QString txt = QString::fromStdString(p->System) + QString::number(p->prn) + "/" + QString::fromStdString(p->Signal)
                    + "\n" + "AZ: " + QString::number(p->az)
                    + "\n" + "EL: " + QString::number(p->el)
                    + "\n" + "SNR: " + QString::number(p->snr)
                    + "\n" + "CH: " + QString::number(p->channel)
                    ;
        showToolTip(event, txt);
    }
}

void SkyViewWidget::draw(QPainter &painter)
{
    const qreal sz = m_clientSize - m_marg * 2.;
    m_font.setPixelSize(m_fontSize);
    painter.setFont(m_font);
    for( auto & p: m_items)
    {
        qreal x = m_width * 0.5 + cos((p.az - 90)/ 180. * M_PI) * (90 - p.el) / 180. * sz;
        qreal y = m_height * 0.5 + sin((p.az - 90) / 180. * M_PI) * (90 - p.el) / 180. * sz;
        int snr = std::max(20, std::min(255, static_cast<int>(std::floor((p.snr - 15.) * 235 / 35.))));
        p.x = x;
        p.y = y;
        int red = p.snr > 0 ? 255 - snr : 99;
        int green = p.snr > 0 ? snr : 99;
        int blue = p.snr > 0 ? snr >> 4 : 99;
        auto color = QColor(red, green, blue, 0xFF);
        painter.setPen(QPen(color, 1, Qt::SolidLine));
        painter.setBrush(QBrush(color));
        painter.drawEllipse(QRectF(x-2,y-2,5,5));
        painter.setPen(QPen(Qt::white, 1, Qt::SolidLine));
        painter.drawText(x - m_fontSize * 0.5 - 1, y + m_fontSize * 1.5 - 1, QString::fromStdString(p.System) + QString::number(p.prn) );
        painter.drawText(x - m_fontSize * 0.5 + 1, y + m_fontSize * 1.5 + 1, QString::fromStdString(p.System) + QString::number(p.prn) );
        painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
        painter.drawText(x - m_fontSize * 0.5, y + m_fontSize * 1.5, QString::fromStdString(p.System) + QString::number(p.prn) );
        
        
    }
}

void SkyViewWidget::drawOverlay(QPainter &painter)
{
    const qreal sz = m_clientSize - m_marg * 2.;
    painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
    painter.drawLine(QLineF(m_leftMargin + m_marg, m_height * 0.5, m_width - m_leftMargin - m_marg, m_height * 0.5));        // H line
    painter.drawLine(QLineF(m_width * 0.5, m_topMargin + m_marg, m_width * 0.5, m_height - m_topMargin - m_marg));    // V line
    painter.drawEllipse(QRectF(m_leftMargin + m_marg, m_topMargin + m_marg, sz, sz));    // outer circle
    painter.drawEllipse(QRectF(m_leftMargin + m_marg + sz * 0.25,
                               m_topMargin + m_marg + sz * 0.25,
                               sz * 0.5,
                               sz * 0.5));    // inner circle
    m_font.setPixelSize(m_fontSize);
    painter.setFont(m_font);
    painter.drawText(m_leftMargin + m_marg - m_fontSize * 1.5, m_height * 0.5 + m_fontSize * 0.5, "W" );
    painter.drawText(m_width - m_leftMargin - m_marg + m_fontSize * 0.5, m_height * 0.5 + m_fontSize * 0.5, "E" );
    painter.drawText(m_width * 0.5 - m_fontSize * 0.25, m_topMargin + m_marg - m_fontSize * 0.5, "N" );
    painter.drawText(m_width * 0.5 - m_fontSize * 0.25, m_height - m_topMargin - m_marg + m_fontSize * 1.5, "S" );
}

void SkyViewWidget::showToolTip(QMouseEvent* event, QString toolTipText)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QToolTip::showText(event->globalPos(), toolTipText, this);
#else
    QToolTip::showText(event->globalPosition().toPoint(), toolTipText, this);
#endif
}
