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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "GetKeyField.h"
#include "globals.h"

#include "QDebug"

namespace LiveMix
{

GetKeyField::GetKeyField()
: QLineEdit()
, m_pKeySequence()
{
//	setFixedSize(150, 25);
//	setTextInteractionFlags(Qt::NoTextInteraction);
	setReadOnly(true); 
}

GetKeyField::~GetKeyField()
{
}

QKeySequence GetKeyField::getKeySequence() {
	return m_pKeySequence;
}
void GetKeyField::setKeySequence(QKeySequence p_pKeySequence) {
	m_pKeySequence = p_pKeySequence;
	setText(m_pKeySequence.toString());
}

void GetKeyField::keyPressEvent(QKeyEvent * p_pEvent) {
	switch (p_pEvent->key()) {
		case Qt::Key_Alt:
		case Qt::Key_AltGr:
		case Qt::Key_Shift:
		case Qt::Key_Meta:
		case Qt::Key_Control:
		case Qt::Key_Super_L:
		case Qt::Key_Super_R:
		case Qt::Key_Menu:
		case Qt::Key_Hyper_L:
		case Qt::Key_Hyper_R:
		case Qt::Key_Home:
		case Qt::Key_Up:
		case Qt::Key_PageUp:
		case Qt::Key_Left:
		case Qt::Key_Right:
		case Qt::Key_End:
		case Qt::Key_Down:
		case Qt::Key_PageDown:
			setKeySequence(QKeySequence());
			break;
		case Qt::Key_Escape:
		case Qt::Key_Enter:
		case Qt::Key_Return:
			p_pEvent->setAccepted(false);
			break;
		default:
			setKeySequence(QKeySequence(p_pEvent->key()+p_pEvent->modifiers()));
	}
}
void GetKeyField::keyReleaseEvent(QKeyEvent * p_pEvent) {
	UNUSED(p_pEvent);
	setKeySequence(m_pKeySequence);
}

}; // LiveMix
