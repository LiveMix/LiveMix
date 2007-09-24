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

#ifndef FADERNAME_H_
#define FADERNAME_H_

#include "PixmapWidget.h"

namespace LiveMix
{

class FaderName : public QWidget
{
    Q_OBJECT
public:
    FaderName(QWidget* parent =NULL);
    ~FaderName();

    void setText(QString text);
    QString text();

    void setFont(QFont p_font)
    {
        m_mixerFont = p_font;
    };
    QFont font()
    {
        return m_mixerFont;
    };

    void mousePressEvent( QMouseEvent * e );
    void mouseDoubleClickEvent( QMouseEvent * e );

signals:
    void clicked();
    void doubleClicked();

protected:
    virtual void paintEvent(QPaintEvent *ev);

private:
//  int m_nWidgetWidth;
//  int m_nWidgetHeight;
    QString m_sName;
    QFont m_mixerFont;
};

}
; //LiveMix

#endif /*FADERNAME_H_*/
