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

#include "Fader.h"

#include "LCD.h"
#include "db.h"

#include <QtGui/QPainter>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

namespace LiveMix
{

Fader::Fader( QWidget *pParent, bool bUseIntSteps, bool bWithoutKnob, QString channel, Backend::ChannelType p_type, bool p_bLinDb )
        : Volume( pParent )
        , _channel( channel )
        , m_type(p_type)
        , m_bWithoutKnob( bWithoutKnob )
        , m_bUseIntSteps( bUseIntSteps )
        , m_bLinDb(p_bLinDb)
        , m_fPeakValue_L( -60 )
        , m_fPeakValue_R( -60 )
        , m_fMinPeak( -60 )
        , m_fMaxPeak( 20 )
        , m_fValue( 0.0 )
        , m_fMinValue( -60 )
        , m_fMaxValue( 20 )
{
    setAttribute( Qt::WA_NoBackground );
    setMinimumSize( 23, 116 );
    setMaximumSize( 23, 116);
    resize( 23, 116 );

    // Background image
    QString background_path = ":/data/fader_background.png";
    bool ok = m_back.load( background_path );
    if( ok == false ) {
        qDebug() << "Fader: Error loading pixmap: " << ":/data/fader_background.png";
    }

    // Leds image
    QString leds_path = ":/data/fader_leds.png";
    ok = m_leds.load( leds_path );
    if( ok == false ) {
        qDebug() << "Error loading pixmap: " << ":/data/fader_background.png";
    }

    // Knob image
    QString knob_path = ":/data/fader_knob.png";
    ok = m_knob.load( knob_path );
    if( ok == false ) {
        qDebug() << "Error loading pixmap: " << ":/data/fader_knob.png";
    }
}
Fader::~Fader()
{
// qDebug() << "[Destroy] Fader";
}


void Fader::mouseMoveEvent( QMouseEvent *ev )
{
    float fVal = (float)( height() - ev->y() ) / (float)height();
    fVal = fVal * ( m_fMaxValue - m_fMinValue );

    fVal = fVal + m_fMinValue;

    setValue( fVal, true );
}


void Fader::mousePressEvent(QMouseEvent *ev)
{
    mouseMoveEvent( ev );
}

void Fader::incValue(bool p_bDirection)
{
    if (m_bUseIntSteps) {
        if (p_bDirection) {
            setValue( m_fValue + 1, true );
        } else {
            setValue( m_fValue - 1, true );
        }
    } else {
        float step = 0.5;
        if (m_fMinValue > -20) {
            step = ( m_fMaxValue - m_fMinValue ) / 50.0;
        }

        if (p_bDirection) {
            setValue( m_fValue + step, true );
        } else {
            setValue( m_fValue - step, true );
        }
    }
}

/*void Fader::wheelEvent ( QWheelEvent *ev )
{
    ev->accept();
 
    if ( m_bUseIntSteps ) {
        if ( ev->delta() > 0 ) {
            setValue( m_fValue + 1, true );
        } else {
            setValue( m_fValue - 1, true );
        }
    } else {
        float step = 0.5;
        if (m_fMinValue != -60) {
            step = ( m_fMaxValue - m_fMinValue ) / 50.0;
        }
 
        if ( ev->delta() > 0 ) {
            setValue( m_fValue + step, true );
        } else {
            setValue( m_fValue - step, true );
        }
    }
}*/


void Fader::setValue( float fVal, bool do_emit )
{
    if ( fVal > m_fMaxValue ) {
        //qDebug() <<  fVal << " > " << m_fMaxValue;
        fVal = m_fMaxValue;
    } else if ( fVal < m_fMinValue ) {
        //qDebug() << fVal << " < " << m_fMinValue;
        fVal = m_fMinValue;
    }

    if ( m_bUseIntSteps ) {
        fVal = (int)fVal;
    }

    if ( m_fValue != fVal ) {
//  qDebug() << "new value: " << fVal;
        m_fValue = fVal;
        update();
    }

    if (do_emit) {
        emit valueChanged(this);
        emit valueChanged(_channel, m_fValue);
        emit dbValueChanged(_channel, m_bLinDb ? db2lin(m_fValue, m_fMinValue) : db2lin(m_fValue));
        emit displayValueChanged(m_type, _channel, displayDb(m_fValue, m_fMinValue));
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
void Fader::setDbValue( float val )
{
    setValue(m_bLinDb ? lin2db(val, m_fMinValue) : lin2db(val));
}
void Fader::setDbPeak_L( float peak )
{
    setPeak_L(m_bLinDb ? lin2db(peak, m_fMinPeak) : lin2db(peak));
}
float Fader::getDbPeak_L()
{
    return m_bLinDb ? db2lin(m_fPeakValue_L, m_fMinPeak) : db2lin(m_fPeakValue_L);
}
void Fader::setDbPeak_R( float peak )
{
    setPeak_R(m_bLinDb ? lin2db(peak, m_fMinPeak) : lin2db(peak));
}
float Fader::getDbPeak_R()
{
    return m_bLinDb ? db2lin(m_fPeakValue_R, m_fMinPeak) : db2lin(m_fPeakValue_R);
}

void Fader::setDbMaxValue( float val )
{
    setMaxValue(m_bLinDb ? lin2db(val, m_fMinValue) : lin2db(val));
}
void Fader::setDbMinValue( float val )
{
    setMinValue(m_bLinDb ? lin2db(val, m_fMinValue) : lin2db(val));
}
void Fader::setDbMaxPeak( float val )
{
    setMaxPeak(m_bLinDb ? lin2db(val, m_fMinValue) : lin2db(val));
}
void Fader::setDbMinPeak( float val )
{
    setMinPeak(m_bLinDb ? lin2db(val, m_fMinValue) : lin2db(val));
}

///
/// Set peak value (0.0 .. 1.0)
///
void Fader::setPeak_L( float fPeak )
{
    if ( fPeak <  m_fMinPeak ) {
        fPeak = m_fMinPeak;
    } else if ( fPeak > m_fMaxPeak ) {
        fPeak = m_fMaxPeak;
    }

    if ( m_fPeakValue_L != fPeak) {
        m_fPeakValue_L = fPeak;
        update();
    }
}


///
/// Set peak value (0.0 .. 1.0)
///
void Fader::setPeak_R( float fPeak )
{
    if ( fPeak <  m_fMinPeak ) {
        fPeak = m_fMinPeak;
    } else if ( fPeak > m_fMaxPeak ) {
        fPeak = m_fMaxPeak;
    }

    if ( m_fPeakValue_R != fPeak ) {
        m_fPeakValue_R = fPeak;
        update();
    }
}


void Fader::paintEvent( QPaintEvent *ev)
{
    QPainter painter(this);

    // background
// painter.drawPixmap( rect(), m_back, QRect( 0, 0, 23, height() ) );
    painter.drawPixmap( ev->rect(), m_back, ev->rect() );

    // peak leds
    QPixmap leds = m_leds.scaled(m_leds.width(), height());

    float realPeak_L = m_fPeakValue_L - m_fMinPeak;
    //int peak_L = height() - ( realPeak_L / fRange ) * height();
    int peak_L = (int)(height() - ( realPeak_L / ( m_fMaxPeak - m_fMinPeak ) ) * height());

    if ( peak_L > height() ) {
        peak_L = height();
    }
    painter.drawPixmap( QRect( 0, peak_L, 11, height() - peak_L ), leds, QRect( 0, peak_L, 11, height() - peak_L ) );


    float realPeak_R = m_fPeakValue_R - m_fMinPeak;
    int peak_R = (int)(height() - ( realPeak_R / ( m_fMaxPeak - m_fMinPeak ) ) * height());
    if ( peak_R > height() ) {
        peak_R = height();
    }
    painter.drawPixmap( QRect( 11, peak_R, 11, height() - peak_R ), leds, QRect( 11, peak_R, 11, height() - peak_R ) );

    if ( m_bWithoutKnob == false ) {
        // knob
        static const uint knob_height = 29;
        static const uint knob_width = 15;

        float fRange = m_fMaxValue - m_fMinValue;

        float realVal = m_fValue - m_fMinValue;

//  uint knob_y = (uint)( height() - ( ( height() - 30 ) * ( m_fValue / fRange ) ) );
        uint knob_y = (uint)( height() - ( ( height() - 30 ) * ( realVal / fRange ) ) );

        painter.drawPixmap( QRect( 4, knob_y - knob_height, knob_width, knob_height), m_knob, QRect( 0, 0, knob_width, knob_height ) );
    }
}


void Fader::setMinValue( float fMin )
{
    m_fMinValue = fMin;
}


void Fader::setMaxValue( float fMax )
{
    m_fMaxValue = fMax;
}


void Fader::setMaxPeak( float fMax )
{
    m_fMaxPeak = fMax;
}


void Fader::setMinPeak( float fMin )
{
    m_fMinPeak = fMin;
}

}
; //LiveMix
