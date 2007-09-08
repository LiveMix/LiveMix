/*
 * Hydrogen
 * Copyright(c) 2002-2007 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 * http://www.hydrogen-music.org
 * Copyright 2007 Stéphane Brunner <stephane.brunner@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "CpuLoadWidget.h"

#include "globals.h"

#include <QtCore/QTimer>
#include <QtGui/QMouseEvent>
#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

namespace LiveMix
{

CpuLoadWidget::CpuLoadWidget( QWidget *pParent )
        : QWidget(pParent)
        , m_fValue(0)
{
    setAttribute(Qt::WA_NoBackground);

    static const uint WIDTH = 88;
    static const uint HEIGHT = 11;

    resize( WIDTH, HEIGHT );
    setMinimumSize( width(), height() );
    setMaximumSize( width(), height() );

// m_nXRunValue = 0;

    // Background image
    QString background_path = ":/data/cpuLoad_back.png";
    bool ok = m_back.load( background_path );
    if( !ok ) {
        qDebug() << "Error loading pixmap " + background_path;
    }

    // Leds image
    QString leds_path = ":/data/cpuLoad_leds.png";
    ok = m_leds.load( leds_path );
    if( !ok ) {
        qDebug() << "Error loading pixmap " + leds_path;
    }

// QTimer *timer = new QTimer(this);
// connect( timer, SIGNAL( timeout() ), this, SLOT( updateCpuLoadWidget() ) );
// timer->start(200); // update player control at 5 fps

// connect( backend, SIGNAL(xrun()), this, SLOT(XRunEvent()));
}


CpuLoadWidget::~CpuLoadWidget()
{}


void CpuLoadWidget::mousePressEvent(QMouseEvent *ev)
{
    UNUSED(ev);
}


void CpuLoadWidget::setValue(float newValue)
{
    if ( newValue > 1.0 ) {
        newValue = 1.0;
    } else if (newValue < 0.0) {
        newValue = 0.0;
    }

    if (m_fValue != newValue) {
        m_fValue = newValue;
        update();
    }
}


float CpuLoadWidget::getValue()
{
    return m_fValue;
}


void CpuLoadWidget::setValue2(float newValue)
{
    if ( newValue > 1.0 ) {
        newValue = 1.0;
    } else if (newValue < 0.0) {
        newValue = 0.0;
    }

    if (m_fValue2 != newValue) {
        m_fValue2 = newValue;
        update();
    }
}


float CpuLoadWidget::getValue2()
{
    return m_fValue2;
}


void CpuLoadWidget::paintEvent( QPaintEvent*)
{
    if (!isVisible()) {
        return;
    }

    QPainter painter(this);

    // background
    painter.drawPixmap( rect(), m_back, QRect( 0, 0, width(), height() ) );

    // leds
    int pos = (int)( 3 + m_fValue * ( width() - 3 * 2 ) );
    painter.drawPixmap( QRect( 0, 0, pos, (height()-1)/2 -1 ), m_leds, QRect( 0, 0, pos, 1) );

    pos = (int)( 3 + m_fValue2 * ( width() - 3 * 2 ) );
    painter.drawPixmap( QRect( 0, (height()-1)/2 +2, pos, (height()-1)/2 -1), m_leds, QRect( 0, 0, pos, 1) );
}

}
; //LiveMix
