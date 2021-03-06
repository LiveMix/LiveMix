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

#include "ClickableLabel.h"

#include <QtGui/QLabel>
#include <QtGui/QPalette>

namespace LiveMix
{

ClickableLabel::ClickableLabel(QWidget *pParent)
        : QLabel(pParent)
{

    QPalette defaultPalette;
    defaultPalette.setColor(QPalette::Background, QColor(58, 62, 72));
    defaultPalette.setColor(QPalette::Foreground, QColor(230, 230, 230));
    this->setPalette(defaultPalette);

    this->setAlignment(Qt::AlignCenter);
}


void ClickableLabel::mousePressEvent(QMouseEvent*)
{
    emit labelClicked(this);
}

}
; //LiveMix
