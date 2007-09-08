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
 *
 */

#ifndef PIXMAP_WIDGET_H
#define PIXMAP_WIDGET_H

#include <QtGui/QWidget>
#include <QtGui/QPixmap>
#include <QtGui/QPaintEvent>
#include <QtGui/QColor>

namespace LiveMix
{

class PixmapWidget : public QWidget
{
    Q_OBJECT

public:
    PixmapWidget( QWidget *pParent);
    ~PixmapWidget();

    void setPixmap( QString sPixmapPath, bool expand_horiz = false );
    void setColor( const QColor& color );

protected:
    QString m_sPixmapPath;
    QColor __color;
    QPixmap m_pixmap;
    bool __expand_horiz;

    virtual void paintEvent( QPaintEvent* ev);
};

}
; //LiveMix

#endif
