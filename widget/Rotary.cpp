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

#include "Rotary.h"

#include <QtGui/QPainter>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#define MV_HEIGHT 201.0

namespace LiveMix
{

RotaryTooltip::RotaryTooltip()
//  : QWidget( 0, "RotaryTooltip", Qt::WStyle_Customize| Qt::WStyle_NoBorder | Qt::WStyle_StaysOnTop| Qt::WX11BypassWM )
        :
        QWidget(0, Qt::ToolTip)
{
    m_pDisplay = new LCDDisplay(this, LCDDigit::SMALL_BLUE, 5);
    m_pDisplay->move(0, 0);
    resize(m_pDisplay->size());

    QPalette defaultPalette;
    defaultPalette.setColor(QPalette::Background, QColor(49, 53, 61));
    this->setPalette(defaultPalette);

}


void RotaryTooltip::showTip(QPoint pos, QString sText)
{
    move(pos);
    m_pDisplay->setText(sText);
    show();
}

RotaryTooltip::~RotaryTooltip()
{
// delete m_pDisplay;
}

///////////////////

QPixmap* Rotary::m_background_normal = NULL;
QPixmap* Rotary::m_background_center = NULL;


Rotary::Rotary(QWidget* parent, RotaryType type, QString sToolTip, bool bUseIntSteps, bool bUseValueTip)
        : Volume(parent)
        , m_bUseIntSteps(bUseIntSteps)
        , m_type(type)
        , m_fMin(type == TYPE_CENTER ? -1 : -60.0)
        , m_fMax(type == TYPE_CENTER ? 1 : 20.0)
        , m_bShowValueToolTip(bUseValueTip)
{
    setAttribute(Qt::WA_NoBackground);
    setToolTip(sToolTip);

    m_pValueToolTip = new RotaryTooltip();

    m_nWidgetWidth = 28;
    m_nWidgetHeight = 26;
    m_fValue = 0.0;
    m_fMousePressValue = m_fMin - 1;

    if (m_background_normal == NULL) {
        m_background_normal = new QPixmap();
        if (m_background_normal->load(":/data/rotary_images.png") == false) {
            qDebug() << "Error loading pixmap: " << ":/data/rotary_images.png";
        }
    }
    if (m_background_center == NULL) {
        m_background_center = new QPixmap();
        if (m_background_center->load(":/data/rotary_center_images.png") == false) {
            qDebug() << "Error loading pixmap: " << ":/data/rotary_center_images.png";
        }
    }

    resize(m_nWidgetWidth, m_nWidgetHeight);
    setFixedSize(m_nWidgetWidth, m_nWidgetHeight);
// m_temp.resize( m_nWidgetWidth, m_nWidgetHeight );
}

Rotary::~Rotary()
{
    delete m_pValueToolTip;
}

QWidget* Rotary::getWidget()
{
    return this;
}

void Rotary::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    float fRange = m_fMax - m_fMin;
    float fValue = m_fValue - m_fMin;

    int nFrame;
    if (m_bUseIntSteps) {
        nFrame = (int)(63.0 * ((int)fValue / fRange));
    } else {
        nFrame = (int)(63.0 * (fValue / fRange));
    }

    //qDebug() << "\nrange: " << fRange;
    //qDebug() << "norm value: " << fValue;
    //qDebug() << "frame: " << nFrame;

    if (m_type == TYPE_NORMAL) {

        int xPos = m_nWidgetWidth * nFrame;
        painter.drawPixmap(rect(), *m_background_normal, QRect(xPos, 0, m_nWidgetWidth, m_nWidgetHeight));
    } else {
        // the image is broken...
        if (nFrame > 62) {
            nFrame = 62;
        }
        int xPos = m_nWidgetWidth * nFrame;
        painter.drawPixmap(rect(), *m_background_center, QRect(xPos, 0, m_nWidgetWidth, m_nWidgetHeight));
    }
}


void Rotary::setDbValue(float val)
{
    setValue(lin2db(val, m_fMin));
}
void Rotary::setValue(float fValue, bool do_emit, int p_iSource)
{
    if (fValue == m_fValue) {
        return;
    }

    if (fValue < m_fMin) {
        fValue = m_fMin;
    } else if (fValue > m_fMax) {
        fValue = m_fMax;
    }

    if (fValue != m_fValue) {
        m_fValue = fValue;
        update();
    }

    if (do_emit) {
        emit valueChanged(this, p_iSource);
    }
}

void Rotary::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton) {
        setCursor(QCursor(Qt::SizeVerCursor));

        m_fMousePressValue = m_fValue;
        m_fMousePressY = ev->y();

        if (m_bShowValueToolTip) {
            char tmp[20];
            if (m_type == TYPE_NORMAL) {
                sprintf(tmp, "%#.1f", m_fValue);
            } else {
                sprintf(tmp, "%#.2f", m_fValue);
            }
            m_pValueToolTip->showTip(mapToGlobal(QPoint(-38, 1)), QString(tmp));
        }
    }
}

void Rotary::mouseReleaseEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton) {
        setCursor(QCursor(Qt::ArrowCursor));
        m_pValueToolTip->hide();
        m_fMousePressValue = m_fMin - 1;
        ev->setAccepted(false);
        emit leftClick(ev);
    } else if (ev->button() == Qt::RightButton) {
        emit rightClick(ev);
    } else if (ev->button() == Qt::MidButton) {
        emit middleClick(ev);
    }
}

void Rotary::incValue(bool p_bDirection, int p_iStep)
{
    if (m_bUseIntSteps) {
        if (p_bDirection) {
            setValue(getValue() + 1 * p_iStep, true);
        } else {
            setValue(getValue() - 1 * p_iStep, true);
        }
    } else {
        float delta = 0.5;
        if (m_fMin > -20) {
            float fRange = m_fMax - m_fMin;
            delta = fRange / MV_HEIGHT;
        }

        if (p_bDirection) {
            setValue(m_fValue + delta * p_iStep, true);
        } else {
            setValue(m_fValue - delta * p_iStep, true);
        }
    }
}

/*void Rotary::wheelEvent ( QWheelEvent *ev )
{
// qDebug() << "wheelEvent delta: " << ev->delta();
    ev->accept();

    if ( m_bUseIntSteps ) {
        if ( ev->delta() > 0 ) {
            setValue( getValue() + 1, true );
        } else {
            setValue( getValue() - 1, true );
        }
    } else {
        float delta = 0.5;
        if (m_fMin != -60) {
            float fRange = m_fMax - m_fMin;
            delta = fRange / MV_HEIGHT;
        }

        if ( ev->delta() > 0 ) {
            setValue( m_fValue + delta, true );
        } else {
            setValue( m_fValue - delta, true );
        }
    }
}*/

void Rotary::mouseMoveEvent(QMouseEvent *ev)
{
    if (m_fMousePressValue != m_fMin - 1) {
        float fRange = m_fMax - m_fMin;

        float deltaY = ev->y() - m_fMousePressY;
        float fNewValue = (m_fMousePressValue - (deltaY / MV_HEIGHT * fRange));

        setValue(fNewValue, true);

        if (m_bShowValueToolTip) {
            QString tip;
            if (m_type == TYPE_NORMAL) {
                tip = displayDbShort(m_fValue, m_fMin);
            } else {
                char tmp[20];
                sprintf(tmp, "%#.2f", m_fValue);
                tip = tmp;
            }
            m_pValueToolTip->showTip(mapToGlobal(QPoint(-38, 1)), tip);
        }
    }
}

void Rotary::setMinValue(float fMin)
{
    m_fMin = fMin;
    m_fMousePressValue = m_fMin - 1;
    update();
}

float Rotary::getMinValue()
{
    return m_fMin;
}

void Rotary::setMaxValue(float fMax)
{
    m_fMax = fMax;
    update();
}

float Rotary::getMaxValue()
{
    return m_fMax;
}

}
; //LiveMix
