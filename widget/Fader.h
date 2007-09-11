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


#ifndef FADER_H
#define FADER_H

#include "backend.h"
#include "Action.h"

#include <QtGui/QWidget>
#include <QtGui/QPixmap>
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>
#include <QtGui/QPaintEvent>

namespace LiveMix
{

///
/// Fader and VuMeter widget
///
class Fader : public Volume
{
    Q_OBJECT

public:
    Fader(QWidget *pParent, bool bUseIntSteps, bool bWithoutKnob, QString channel =NULL, Backend::ChannelType p_type =Backend::IN, bool p_bLinDb =true );
    ~Fader();

    void setMinValue( float fMin );
    void setMaxValue( float fMax );
    float getMinValue()
    {
        return m_fMinValue;
    }
    float getMaxValue()
    {
        return m_fMaxValue;
    }

    void setValue( float fVal, bool emit = false );
    float getValue();
    // in fact the external value is standanrd and internal in dB
    void setDbValue( float fVal );
    float getDbValue();

    bool getLinDb()
    {
        return m_bLinDb;
    };

    void incValue(bool p_bDirection);

    void setMaxPeak( float fMax );
    void setMinPeak( float fMin );

    void setPeak_L( float peak );
    float getPeak_L()
    {
        return m_fPeakValue_L;
    }

    void setPeak_R( float peak );
    float getPeak_R()
    {
        return m_fPeakValue_R;
    }

    // in fact the external value is standanrd and internal in dB
    void setDbPeak_L( float peak );
    float getDbPeak_L();

    // in fact the external value is standanrd and internal in dB
    void setDbPeak_R( float peak );
    float getDbPeak_R();

    void setDbMaxValue( float val );
    void setDbMinValue( float val );
    void setDbMaxPeak( float val );
    void setDbMinPeak( float val );

    virtual void mousePressEvent(QMouseEvent *ev);
    virtual void mouseMoveEvent(QMouseEvent *ev);
	virtual void mouseReleaseEvent(QMouseEvent* ev);
	virtual void mouseDoubleClickEvent(QMouseEvent* ev);
//    virtual void wheelEvent( QWheelEvent *ev );
    virtual void paintEvent(QPaintEvent *ev);

signals:
    void valueChanged(Fader*);
    void valueChanged(QString Channel, float value);
    // in fact the external value is standanrd and internal in dB
    void dbValueChanged(QString Channel, float value);
    void displayValueChanged(Backend::ChannelType, QString Channel, QString value);

private:
    QString _channel;
    Backend::ChannelType m_type;

    float m_fMousePressValue;
    float m_fMousePressY;

    bool m_bWithoutKnob;
    bool m_bUseIntSteps;
    bool m_bLinDb;

    float m_fPeakValue_L;
    float m_fPeakValue_R;
    float m_fMinPeak;
    float m_fMaxPeak;

    float m_fValue;
    float m_fMinValue;
    float m_fMaxValue;

    QPixmap m_back;
    QPixmap m_leds;
    QPixmap m_knob;
};


/*
class Knob : public QWidget
{
 Q_OBJECT
 public:
  Knob( QWidget* parent, QString channel );
  ~Knob();
 
  void setValue( float fValue, bool emit = false );
  float getValue() { return m_fValue; }
  // in fact the external value is standanrd and internal in dB
  void setDbValue( float fVal );
  float getDbValue();
 
 signals:
  void valueChanged(QString Channel, float value);
  // in fact the external value is standanrd and internal in dB
  void dbValueChanged(QString Channel, float value);
 
 private:
  QString _channel;
 
  static QPixmap *m_background;
 
  int m_nWidgetWidth;
  int m_nWidgetHeight;
 
  float m_fValue;
  float m_fMousePressValue;
  float m_fMousePressY;
 
  virtual void paintEvent( QPaintEvent *ev );
  virtual void mousePressEvent( QMouseEvent *ev );
  virtual void mouseReleaseEvent( QMouseEvent *ev );
  virtual void mouseMoveEvent( QMouseEvent *ev );
  virtual void wheelEvent( QWheelEvent *ev );
};
*/
}
; //JackMix

#endif
