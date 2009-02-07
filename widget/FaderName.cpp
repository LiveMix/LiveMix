/*
 * Hydrogen
 * Copyright(c) 2007 by Alex >Comix< Cominu [comix@users.sourceforge.net]
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "FaderName.h"

#include <QtGui/QPainter>
#include <QtCore/QDebug>

namespace LiveMix
{

FaderName::FaderName(QWidget* parent)
        : QWidget(parent)//, "FaderName")
{
// qDebug() << "[INIT] FaderName";
// m_nWidgetWidth = 17;
// m_nWidgetHeight = 116;

// Preferences *pref = Preferences::getInstance();
// QString family = pref->getMixerFontFamily().c_str();
// int size = pref->getMixerFontPointSize();
// m_mixerFont.setFamily( family );
// m_mixerFont.setPointSize( size );
//    m_mixerFont.setBold(true);
    m_mixerFont.setItalic(true);

// setPixmap( "mixerline_label_background.png" );

// setFixedSize( m_nWidgetWidth, m_nWidgetHeight );
// resize( m_nWidgetWidth, m_nWidgetHeight );
    setFixedSize(17, 116);
    resize(17, 116);
}


FaderName::~FaderName()
{
// qDebug() << "[DESTROY] FaderName";
}


void FaderName::paintEvent(QPaintEvent* ev)
{
    QWidget::paintEvent(ev);

    QPainter p(this);

    p.setPen(QColor(230, 230, 230));
    p.setFont(m_mixerFont);
    p.rotate(-90);
// p.drawText( -m_nWidgetHeight + 5, 0, m_nWidgetHeight - 10, m_nWidgetWidth, Qt::AlignVCenter, m_sName );
    p.drawText(-height() + 5, 0, height() - 10, width(), Qt::AlignVCenter, m_sName);
}


void FaderName::setText(QString text)
{
    if (m_sName != text) {
        m_sName = text;
        update();
    }
}


QString FaderName::text()
{
    return m_sName;
}


void FaderName::mousePressEvent(QMouseEvent*)
{
    emit clicked();
}


void FaderName::mouseDoubleClickEvent(QMouseEvent*)
{
    emit doubleClicked();
}

}
; //LiveMix
