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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef CPU_LOAD_WIDGET_H
#define CPU_LOAD_WIDGET_H

#include <QtGui/QWidget>
#include <QtGui/QPixmap>
#include <QtGui/QMouseEvent>
#include <QtGui/QPaintEvent>

//#include "backend_interface.h"

namespace LiveMix
{

///
/// Shows CPU load
///
class CpuLoadWidget : public QWidget
{
    Q_OBJECT

public:
//  CpuLoadWidget(QWidget *pParent, Backend* pBackend);
    CpuLoadWidget(QWidget *pParent);
    ~CpuLoadWidget();

    void setValue(float newValue);
    float getValue();

    void setValue2(float newValue);
    float getValue2();

    void mousePressEvent(QMouseEvent *ev);
    void paintEvent(QPaintEvent *ev);

// public slots:
//  void updateCpuLoadWidget();
//  void XRunEvent();

private:
//  Backend* m_pBackend;

    float m_fValue;
    float m_fValue2;
//  uint m_nXRunValue;

    QPixmap m_back;
    QPixmap m_leds;
};

}
; //LiveMix

#endif
