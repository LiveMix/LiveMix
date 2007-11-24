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

#include "Fader.h"

#include "LCD.h"
#include "db.h"

#include <QtGui/QPainter>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <sys/time.h>

namespace LiveMix
{
    
#define VU_SUBSTRACT 2.0;

Fader::Fader(QWidget *pParent, bool bUseIntSteps, bool bWithoutKnob, bool p_bLinDb)
        : Volume(pParent)
        , m_bWithoutKnob(bWithoutKnob)
        , m_bUseIntSteps(bUseIntSteps)
        , m_bLinDb(p_bLinDb)
        , m_rLastVuCalculate_L(QDateTime::currentDateTime())
        , m_rLastVuCalculate_R(QDateTime::currentDateTime())
        , m_fVuValue_L(-60)
        , m_fVuValue_R(-60)
        , m_fPeakValue_L(-60)
        , m_fPeakValue_R(-60)
        , m_fMinPeak(-60)
        , m_fMaxPeak(20)
        , m_fValue(0.0)
        , m_fMinValue(-60)
        , m_fMaxValue(20)
{
    setAttribute(Qt::WA_NoBackground);
    setMinimumSize(23, 116);
    setMaximumSize(23, 116);
    resize(23, 116);

    m_fMousePressValue = m_fMinValue - 1;

    // Background image
    QString path = ":/data/fader_background.svg";
    bool ok = m_back_original.load(path);
    if (ok == false) {
        qDebug() << "Fader: Error loading pixmap: " << path;
    }

    path = ":/data/fader_top.svg";
    ok = m_top.load(path);
    if (ok == false) {
        qDebug() << "Fader: Error loading pixmap: " << path;
    }

    path = ":/data/fader_bottom.svg";
    ok = m_bottom.load(path);
    if (ok == false) {
        qDebug() << "Fader: Error loading pixmap: " << path;
    }

    // Leds image
    QString leds_path = ":/data/fader_leds.svg";
    ok = m_leds_original.load(leds_path);
    if (ok == false) {
        qDebug() << "Error loading pixmap: " << ":/data/fader_background.svg";
    }

    // Knob image
    QString knob_path = ":/data/fader_knob.png";
    ok = m_knob.load(knob_path);
    if (ok == false) {
        qDebug() << "Error loading pixmap: " << ":/data/fader_knob.png";
    }
}
Fader::~Fader()
{
// qDebug() << "[Destroy] Fader";
}
QWidget* Fader::getWidget()
{
    return this;
}

void Fader::setFixedHeight(int h)
{
    QWidget::setFixedHeight(h);
    m_back_scaled = m_back_original.scaled(width(), height()-30, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_leds_scaled = m_leds_original.scaled(width(), height()-30, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}
void Fader::setFixedSize(const QSize & s)
{
    QWidget::setFixedSize(s);
    m_back_scaled = m_back_original.scaled(width(), height()-30, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_leds_scaled = m_leds_original.scaled(width(), height()-30, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}
void Fader::setFixedSize(int w, int h)
{
    QWidget::setFixedSize(w, h);
    m_back_scaled = m_back_original.scaled(width(), height()-30, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_leds_scaled = m_leds_original.scaled(width(), height()-30, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

void Fader::mouseMoveEvent(QMouseEvent *ev)
{
    if (m_fMousePressValue != m_fMinValue - 1) {
        float fRange = m_fMaxValue - m_fMinValue;

        float deltaY = ev->y() - m_fMousePressY;
        float fNewValue = (m_fMousePressValue - (deltaY / height() * fRange));

        setValue(fNewValue, true);
    }
    /* if (ev->button() == Qt::LeftButton) {
         float fVal = (float)( height() - ev->y() ) / (float)height();
         fVal = fVal * ( m_fMaxValue - m_fMinValue );

         fVal = fVal + m_fMinValue;

         setValue( fVal, true );
     }*/
}


void Fader::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton) {
        setCursor(QCursor(Qt::SizeVerCursor));

        m_fMousePressValue = m_fValue;
        m_fMousePressY = ev->y();
    }
}

void Fader::mouseReleaseEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton) {
        setCursor(QCursor(Qt::ArrowCursor));
        m_fMousePressValue = m_fMinValue - 1;
        emit leftClick(ev);
    } else if (ev->button() == Qt::RightButton) {
        emit rightClick(ev);
    } else if (ev->button() == Qt::MidButton) {
        emit middleClick(ev);
    }
}

void Fader::mouseDoubleClickEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton) {
        float fVal = (float)(height() - ev->y() - 15.0) / ((float)height() - 30.0);
        fVal = fVal * (m_fMaxValue - m_fMinValue);

        fVal = fVal + m_fMinValue;

        setValue(fVal, true);
    }
}

void Fader::incValue(bool p_bDirection, int p_iStep)
{
    if (m_bUseIntSteps) {
        if (p_bDirection) {
            setValue(m_fValue + 1 * p_iStep, true);
        } else {
            setValue(m_fValue - 1 * p_iStep, true);
        }
    } else {
        float step = 0.5;
        if (m_fMinValue > -20) {
            step = (m_fMaxValue - m_fMinValue) / 50.0;
        }

        if (p_bDirection) {
            setValue(m_fValue + step * p_iStep, true);
        } else {
            setValue(m_fValue - step * p_iStep, true);
        }
    }
}

void Fader::setValue(float fVal, bool do_emit)
{
    if (fVal > m_fMaxValue) {
        fVal = m_fMaxValue;
    } else if (fVal < m_fMinValue) {
        fVal = m_fMinValue;
    }

    if (m_bUseIntSteps) {
        fVal = (int)fVal;
    }

    if (m_fValue != fVal) {
        m_fValue = fVal;
        update();
    }

    if (do_emit) {
        emit valueChanged(this);
    }
}


float Fader::getValue()
{
    return m_fValue;
}
float Fader::getDbValue()
{
    return m_bLinDb ? db2lin(m_fValue, m_fMinValue) :  db2lin(m_fValue);
}


// in fact the external value is standanrd and internal in dB
void Fader::setDbValue(float val)
{
    setValue(m_bLinDb ? lin2db(val, m_fMinValue) : lin2db(val));
}
void Fader::setDbPeak_L(float peak)
{
    setPeak_L(m_bLinDb ? lin2db(peak, m_fMinPeak) : lin2db(peak));
}
float Fader::getDbPeak_L()
{
    return m_bLinDb ? db2lin(m_fPeakValue_L, m_fMinPeak) : db2lin(m_fPeakValue_L);
}
void Fader::setDbPeak_R(float peak)
{
    setPeak_R(m_bLinDb ? lin2db(peak, m_fMinPeak) : lin2db(peak));
}
float Fader::getDbPeak_R()
{
    return m_bLinDb ? db2lin(m_fPeakValue_R, m_fMinPeak) : db2lin(m_fPeakValue_R);
}

void Fader::setDbMaxValue(float val)
{
    setMaxValue(m_bLinDb ? lin2db(val, m_fMinValue) : lin2db(val));
}
void Fader::setDbMinValue(float val)
{
    setMinValue(m_bLinDb ? lin2db(val, m_fMinValue) : lin2db(val));
}
void Fader::setDbMaxPeak(float val)
{
    setMaxPeak(m_bLinDb ? lin2db(val, m_fMinValue) : lin2db(val));
}
void Fader::setDbMinPeak(float val)
{
    setMinPeak(m_bLinDb ? lin2db(val, m_fMinValue) : lin2db(val));
}

/// Set peak value in db
void Fader::setPeak_L(float fPeak)
{
    if (fPeak <  m_fMinPeak) {
        fPeak = m_fMinPeak;
    } else if (fPeak > m_fMaxPeak) {
        fPeak = m_fMaxPeak;
    }

    QDateTime now = QDateTime::currentDateTime();
    float newVu = m_fVuValue_L - (m_rLastVuCalculate_L.toTime_t() - now.toTime_t()) * VU_SUBSTRACT;
    m_rLastVuCalculate_L = now;
    
    if (newVu <  fPeak) {
        newVu = fPeak;
    }
    if (m_fPeakValue_L != fPeak || newVu != m_fVuValue_L) {
        m_fPeakValue_L = fPeak;
        m_fVuValue_L = newVu;
        update();
    }
}

/// Set peak value in db
void Fader::setPeak_R(float fPeak)
{
    if (fPeak <  m_fMinPeak) {
        fPeak = m_fMinPeak;
    } else if (fPeak > m_fMaxPeak) {
        fPeak = m_fMaxPeak;
    }

    QDateTime now = QDateTime::currentDateTime();
    float newVu = m_fVuValue_R - (m_rLastVuCalculate_R.toTime_t() - now.toTime_t()) * VU_SUBSTRACT;
    m_rLastVuCalculate_R = now;
    
    if (newVu <  fPeak) {
        newVu = fPeak;
    }
    if (m_fPeakValue_R != fPeak || newVu != m_fVuValue_R) {
        m_fPeakValue_R = fPeak;
        m_fVuValue_R = newVu;
        update();
    }
}

void Fader::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    // background
    painter.drawPixmap(QRect(0, 15, width(), height()-30), m_back_scaled, QRect(0, 0, width(), height()-30));
    painter.drawPixmap(QRect(0, 0, width(), 15), m_top, QRect(0, 0, width(), 15));
    painter.drawPixmap(QRect(0, height() - 15, width(), 15), m_bottom, QRect(0, 0, width(), 15));

    // peak leds
    if (m_fMaxPeak > m_fMinPeak) {
        float realPeak_L = m_fPeakValue_L - m_fMinPeak;
        int peak_L = (int)(height() - 30 - (realPeak_L / (m_fMaxPeak - m_fMinPeak)) * (height() - 30));
        if (peak_L > height() - 30) {
            peak_L = height() - 30;
        }
        painter.drawPixmap(QRect(0, peak_L + 15, width() / 2, height() - 30 - peak_L), m_leds_scaled,
                           QRect(0, peak_L     , width() / 2, height() - 30 - peak_L));


        float realPeak_R = m_fPeakValue_R - m_fMinPeak;
        int peak_R = (int)(height() - 30 - (realPeak_R / (m_fMaxPeak - m_fMinPeak)) * (height() - 30));
        if (peak_R > height() - 30) {
            peak_R = height() - 30;
        }
        painter.drawPixmap(QRect(width() / 2, peak_R + 15, width() / 2, height() - 30 - peak_R), m_leds_scaled,
                           QRect(width() / 2, peak_R     , width() / 2, height() - 30 - peak_R));
    }

    // VU leds
    if (m_fMaxPeak > m_fMinPeak) {
//        painter.setBrush(QBrush(QColor(255, 240, 0)));
        painter.setPen(QColor(255, 255, 255));
        
        float realVu_L = m_fVuValue_L - m_fMinPeak;
        int Vu_L = (int)(height() - 30 - (realVu_L / (m_fMaxPeak - m_fMinPeak)) * (height() - 30));
        if (Vu_L > height() - 30) {
            Vu_L = height() - 30;
        }
        if (Vu_L < height() - 31) {
            painter.drawLine(4, Vu_L + 15, 7, Vu_L + 15);
//            painter.drawPixmap(QRect(0, Vu_L + 15, width() / 2, 2), m_leds_scaled,
//                               QRect(0, Vu_L     , width() / 2, 2));
        }


        float realVu_R = m_fVuValue_R - m_fMinPeak;
        int Vu_R = (int)(height() - 30 - (realVu_R / (m_fMaxPeak - m_fMinPeak)) * (height() - 30));
        if (Vu_R > height() - 30) {
            Vu_R = height() - 30;
        }
            qDebug()<<Vu_R<<Vu_R<<height();
        if (Vu_R < height() - 31) {
            painter.drawLine(15, Vu_R + 15, 18, Vu_R + 15);
//            painter.drawPixmap(QRect(width() / 2, Vu_R + 15, width() / 2, 2), m_leds_scaled,
//                               QRect(width() / 2, Vu_R     , width() / 2, 2));
        }
    }
    
    if (!m_bWithoutKnob) {
        // knob
        static const uint knob_height = 29;
        static const uint knob_width = 15;

        float fRange = m_fMaxValue - m_fMinValue;

        float realVal = m_fValue - m_fMinValue;

        uint knob_y = (uint)(height() - ((height() - 30) * (realVal / fRange)));

        painter.drawPixmap(QRect(4, knob_y - knob_height, knob_width, knob_height), m_knob, QRect(0, 0, knob_width, knob_height));
    }
}


void Fader::setMinValue(float fMin)
{
    m_fMinValue = fMin;
    m_fMousePressValue = m_fMinValue - 1;
}


void Fader::setMaxValue(float fMax)
{
    m_fMaxValue = fMax;
}


void Fader::setMaxPeak(float fMax)
{
    m_fMaxPeak = fMax;
}


void Fader::setMinPeak(float fMin)
{
    m_fMinPeak = fMin;
}

}
; //LiveMix
