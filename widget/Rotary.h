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

#ifndef ROTARY_H
#define ROTARY_H

#include "LCD.h"
#include "db.h"
#include "Action.h"

#include <QWidget>
#include <QPixmap>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPaintEvent>
#include <QDebug>

namespace LiveMix
{

class RotaryTooltip : public QWidget
{
    Q_OBJECT
public:
    RotaryTooltip();
    ~RotaryTooltip();
    void showTip(QPoint pos, QString sText);

private:
    LCDDisplay *m_pDisplay;
};



class Rotary : public Volume
{
    Q_OBJECT
public:
    enum RotaryType {
        TYPE_NORMAL,
        TYPE_CENTER
    };

    Rotary(QWidget* parent, RotaryType type, QString sToolTip, bool bUseIntSteps, bool bUseValueTip);
    ~Rotary();
    virtual QWidget* getWidget();

    void setMinValue(float fMin);
    float getMinValue();

    void setMaxValue(float fMax);
    float getMaxValue();

    void setValue(float fValue, bool emit = false, int p_iSource =0);
    float getValue() {
        if (m_bUseIntSteps) {
            int val = (int)m_fValue;
            return val;
        } else
            return m_fValue;
    }
    // in fact the external value is standanrd and internal in dB
    void setDbValue(float fValue);
    float getDbValue() {
        if (m_bUseIntSteps) {
            int val = (int)db2lin(m_fValue, m_fMin);
            return val;
        } else
            return db2lin(m_fValue, m_fMin);
    }
    void incValue(bool p_bDirection, int p_iStep =1);

private:
    bool m_bUseIntSteps;
    RotaryType m_type;
    static QPixmap* m_background_normal;
    static QPixmap* m_background_center;

    int m_nWidgetWidth;
    int m_nWidgetHeight;

    float m_fMin;
    float m_fMax;
    float m_fValue;

    float m_fMousePressValue;
    float m_fMousePressY;

    RotaryTooltip *m_pValueToolTip;
    bool m_bShowValueToolTip;

    virtual void paintEvent(QPaintEvent *ev);
    virtual void mousePressEvent(QMouseEvent *ev);
    virtual void mouseReleaseEvent(QMouseEvent *ev);
    virtual void mouseMoveEvent(QMouseEvent *ev);
//    virtual void wheelEvent( QWheelEvent *ev );
};

}
; //LiveMix

#endif
