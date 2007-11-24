/*
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

#include "Action.h"

namespace LiveMix
{

Action::Action(QWidget* p_pParent) : QWidget(p_pParent)
{}

Action::~Action()
{}

void Action::mouseDoubleClickEvent(QMouseEvent *p_pEvent) {
    emit(emitMouseDoubleClickEvent(p_pEvent));
}

Volume::Volume(QWidget* p_pParent) : Action(p_pParent)
{}

Volume::~Volume()
{}


Toggle::Toggle(QWidget* p_pParent) : Action(p_pParent)
{}

Toggle::~Toggle()
{}

}; // LiveMix
