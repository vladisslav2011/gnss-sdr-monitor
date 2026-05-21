/*!
 * \file ppm_widget.cpp
 * \brief Implementation of a widget that shows the clock drift in a chart as
 * reported by the receiver.
 *
 * \author vladisslav2011, 2026. vladisslav2011(at)gmail.com
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


#include "ppm_widget.h"
#include <QChart>
#include <QGraphicsLayout>
#include <QLayout>

/*!
 Constructs a ppm widget.
 */
PpmWidget::PpmWidget(QWidget *parent) : QWidget(parent)
{
    // Default buffer size.
    m_bufferSize = 100;
    alfa=1.;
    decim=1;
    count=0;filtered=0.;

    m_dataBuffer.resize(m_bufferSize);
    m_dataBuffer.clear();

    m_series = new QtCharts::QLineSeries();
    m_chartView = new QtCharts::QChartView(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    this->setLayout(layout);
    layout->addWidget(m_chartView);

    QtCharts::QChart *chart = m_chartView->chart();

    chart->addSeries(m_series);
    chart->setTitle("Clock drift vs Time");
    chart->legend()->hide();
    chart->createDefaultAxes();
    chart->axes(Qt::Horizontal).back()->setTitleText("TOW [s]");
    chart->axes(Qt::Vertical).back()->setTitleText("PPM");
    chart->layout()->setContentsMargins(0, 0, 0, 0);
    chart->setContentsMargins(-18, -18, -14, -16);

    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setContentsMargins(0, 0, 0, 0);

    min_x = std::numeric_limits<double>::max();
    max_x = -std::numeric_limits<double>::max();

    min_y = std::numeric_limits<double>::max();
    max_y = -std::numeric_limits<double>::max();
}

/*!
 Adds the \a PPM and associated \a tow to the widget's internal data structures.
 */
void PpmWidget::addData(qreal tow, qreal ppm)
{
    if(m_dataBuffer.back().x() == tow)
        return;
    count++;
    filtered+=(ppm-filtered)*alfa;
    if(count>=decim)
    {
        m_dataBuffer.push_back(QPointF(tow, filtered));
        count=0;
    }
}

/*!
 Redraws the chart by replacing the old data in the series object with the new data.
 */
void PpmWidget::redraw()
{
    if (!m_dataBuffer.empty())
    {
        double min_x = std::numeric_limits<double>::max();
        double max_x = -std::numeric_limits<double>::max();

        double min_y = std::numeric_limits<double>::max();
        double max_y = -std::numeric_limits<double>::max();

        QtCharts::QChart *chart = m_chartView->chart();
        QPointF p;
        QVector<QPointF> vec;

        for (size_t i = 0; i < m_dataBuffer.size(); i++)
        {
            p = m_dataBuffer.at(i);
            vec << p;

            min_x = std::min(min_x, p.x());
            min_y = std::min(min_y, p.y());

            max_x = std::max(max_x, p.x());
            max_y = std::max(max_y, p.y());
        }

        m_series->replace(vec);

        chart->axes(Qt::Horizontal).back()->setRange(min_x, max_x);
        chart->axes(Qt::Vertical).back()->setRange(min_y, max_y);
    }
}

/*!
 Clears all the data from the widget's internal data structures.
 */
void PpmWidget::clear()
{
    m_dataBuffer.clear();
    m_series->clear();
}

/*!
 Sets the size of the internal circular buffer that stores the widget's data.
 */
void PpmWidget::setBufferSize(size_t size)
{
    if(m_bufferSize == size)
        return;
    m_bufferSize = size;
    m_dataBuffer.resize(m_bufferSize);
    clear();
}

void PpmWidget::setFilter(double value)
{
    if(alfa==value)
        return;
    alfa=qreal(value);
}

void PpmWidget::setDecim(int value)
{
    if(decim==value)
        return;
    decim=value;
    if(count>=value)
        count=0;
}

