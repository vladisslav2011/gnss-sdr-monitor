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
void SkyViewWidget::addData(uint32_t prn, qreal az, qreal el, std::string System, std::string Signal, bool combined, qreal snr, int channel, bool used, bool healthy)
{
    auto key = std::pair<std::string,uint32_t>(Signal, prn);
    auto it = m_items.find(key);
    if(it!=m_items.end())
    {
        it->second.az=az;
        it->second.el=el;
        it->second.snr=snr;
        it->second.used=used;
        it->second.channel=channel;
        it->second.healthy=healthy;
        it->second.isRefreshed=true;
    }else
        m_items.emplace(key,int_data{prn,az,el,System, Signal, combined,snr, channel, used, healthy, true});
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
    update();
}

void SkyViewWidget::startUpdate()
{
    for(auto it = m_items.begin(); it!=m_items.end();it++)
        it->second.isRefreshed = false;
}

void SkyViewWidget::finishUpdate()
{
    for(auto it = m_items.cbegin(); it!=m_items.cend();)
        if(!it->second.isRefreshed)
            m_items.erase(it++);
        else
            it++;
}

void SkyViewWidget::selectSat(uint32_t prn, char system)
{
    m_sel_item = std::pair<char,uint32_t>(system,prn);
    redraw();
}

void SkyViewWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(rect(),Qt::black);
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

    for( auto& it: m_items)
        if(isPointCloseTo(pt,it.second.x,it.second.y,10))
        {
            int dist=std::min(std::abs(pt.x()-it.second.x),std::abs(pt.y()-it.second.y));
            if(dist<min_dist)
            {
                min_dist = dist;
                p = & it.second;
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
        m_top_item = std::pair<std::string,uint32_t>(p->Signal,p->prn);
    }
}

void SkyViewWidget::mousePressEvent(QMouseEvent* event)
{

    QPoint pt = event->pos();
    int_data * p = nullptr;
    int min_dist = 1000000;

    for( auto& it: m_items)
        if(isPointCloseTo(pt,it.second.x,it.second.y,10))
        {
            int dist=std::min(std::abs(pt.x()-it.second.x),std::abs(pt.y()-it.second.y));
            if(dist<min_dist)
            {
                min_dist = dist;
                p = & it.second;
            }
        }

    if(p)
        emit satSelected(p->prn, p->System[0]);
    else
        emit satSelected(0, ' ');
}

void SkyViewWidget::draw(QPainter &painter)
{
    const qreal sz = m_clientSize - m_marg * 2.;
    const qreal circle_d = std::max(7., sz * 0.02);
    const qreal fontsz = std::max(m_fontSize, sz * 0.02);
    static const QColor redColor = QColor(255,100,100,255);
    m_font.setPixelSize(fontsz);
    painter.setFont(m_font);
    auto paintItem=[&](int_data& p)
    {
        qreal x = m_width * 0.5 + cos((p.az - 90)/ 180. * M_PI) * (90 - p.el) / 180. * sz;
        qreal y = m_height * 0.5 + sin((p.az - 90) / 180. * M_PI) * (90 - p.el) / 180. * sz;
        p.x = x;
        p.y = y;
        QColor color;
        if(12. > p.snr)
            color = QColor(190, 190, 190, 255);      // gray
        else if(30. > p.snr)
            color = QColor(255, 0, 0, 255);          // red
        else if(36. > p.snr)
            color = QColor(255, 255, 0, 255);        // yellow
        else if(42. > p.snr)
            color = QColor(0, 205, 0, 255);          // green3
        else
            color = QColor(0, 255, 180, 255);        // green and some blue
        painter.setPen(QPen(color, 1, Qt::SolidLine));
        painter.setBrush(p.used ? QBrush(color) : QBrush(QColor(0,0,0,0)));
        painter.drawEllipse(QRectF(x-circle_d*0.5,y-circle_d*0.5,circle_d,circle_d));
        painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
        painter.drawText(x - fontsz * 0.5 - 1, y + fontsz * 1.5 - 1, QString::fromStdString(p.System) + QString::number(p.prn) );
        painter.drawText(x - fontsz * 0.5 + 1, y + fontsz * 1.5 + 1, QString::fromStdString(p.System) + QString::number(p.prn) );
        painter.setPen(QPen(p.healthy ? Qt::white : redColor, 1, Qt::SolidLine));
        painter.drawText(x - fontsz * 0.5, y + fontsz * 1.5, QString::fromStdString(p.System) + QString::number(p.prn) );
    };
    int_data * top_item{};
    int_data * sel_item{};
    for(auto& it: m_items)
    {
        if(it.second.System[0] == m_sel_item.first && it.second.prn == m_sel_item.second)
            sel_item=&it.second;
        if(it.first == m_top_item)
        {
            top_item=&it.second;
            continue;
        }
        paintItem(it.second);
    }
    if(top_item)
        paintItem(*top_item);
    if(sel_item)
    {
        //draw a pink cross
        auto& p = *sel_item;
        painter.setPen(QPen(QColor(255,0,255,255), 2, Qt::SolidLine));
        painter.drawLine(QLineF(p.x - fontsz, p.y, p.x + fontsz, p.y));
        painter.drawLine(QLineF(p.x, p.y - fontsz, p.x, p.y + fontsz));
    }
}

void SkyViewWidget::drawOverlay(QPainter &painter)
{
    const qreal sz = m_clientSize - m_marg * 2.;
    painter.setPen(QPen(Qt::white, 1, Qt::SolidLine));
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
