/*
 * LiveMix
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

#include "AssigneToPannel.h"

#import <QLabel>
#import <QDebug>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>

namespace LiveMix
{

AssigneToPannel::AssigneToPannel(QString p_sChannel, QString p_sFunction, bool p_bVolume, bool p_bOnlyDirrect, QKeySequence p_rActionOnChannelKeySequence
			, QKeySequence p_rSelectChannelKeySequence, QKeySequence p_rActionOnSelectedChannelKeySequence)
: QDialog()
, m_pActionOnChannel(new GetKeyField)
, m_pSelectChannel(new GetKeyField)
, m_pActionOnSelectedChannel(new GetKeyField)
{
	QGridLayout* layout = new QGridLayout();
	setLayout(layout);
//	setTitle(trUtf8("Edit key binding"));
	
	m_pActionOnChannel->setKeySequence(p_rActionOnChannelKeySequence);
	m_pSelectChannel->setKeySequence(p_rSelectChannelKeySequence);
	m_pActionOnSelectedChannel->setKeySequence(p_rActionOnSelectedChannelKeySequence);

	layout->addWidget(m_pActionOnChannel, 1, 2);
	if (!p_bOnlyDirrect) {
		layout->addWidget(m_pSelectChannel, 2, 2);
		layout->addWidget(m_pActionOnSelectedChannel, 3, 2);
	}

	if (p_bVolume) {
		layout->addWidget(new QLabel(trUtf8("Key to select the %2 of the channel %1").arg(p_sChannel).arg(p_sFunction)), 1, 1);
	}
	else {
		layout->addWidget(new QLabel(trUtf8("Key to %2 the channel %1").arg(p_sChannel).arg(p_sFunction)), 1, 1);
	}
	
	if (!p_bOnlyDirrect) {
		layout->addWidget(new QLabel(trUtf8("Key to select the channel %1").arg(p_sChannel)), 2, 1);
		
		if (p_bVolume) {
			layout->addWidget(new QLabel(trUtf8("Key to select the %1 of selected channel").arg(p_sFunction)), 3, 1);
		}
		else {
			layout->addWidget(new QLabel(trUtf8("Key to %1 the selected channel").arg(p_sFunction)), 3, 1);
		}
	}
	
	QWidget* buttons = new QWidget;
//	layout->addWidget(buttons, 4, 1, 2, 2);
	layout->addWidget(buttons, 4, 1, 1, 2, Qt::AlignHCenter);
	QHBoxLayout* buttonsLayout = new QHBoxLayout;
	buttons->setLayout(buttonsLayout);
	QPushButton* ok = new QPushButton(trUtf8("OK"));
	QPushButton* cancel = new QPushButton(trUtf8("Cancel"));
	buttonsLayout->addWidget(ok);
	buttonsLayout->addWidget(cancel);
	ok->setDefault(true);
	
	connect(ok, SIGNAL( clicked(bool) ), this, SLOT( okClicked(bool) ) );
	connect(cancel, SIGNAL( clicked(bool) ), this, SLOT( cancelClicked(bool) ) );
}

AssigneToPannel::~AssigneToPannel()
{
}

void AssigneToPannel::okClicked(bool /*p_bChecked*/) {
	done(QDialog::Accepted);
}
void AssigneToPannel::cancelClicked(bool /*p_bChecked*/) {
	done(QDialog::Rejected);
}

QKeySequence AssigneToPannel::getActionOnChannelKeySequence() {
	return m_pActionOnChannel->getKeySequence();
}
QKeySequence AssigneToPannel::getSelectChannelKeySequence() {
	return m_pSelectChannel->getKeySequence();
}
QKeySequence AssigneToPannel::getActionOnSelectedChannelKeySequence() {
	return m_pActionOnSelectedChannel->getKeySequence();
}

}
; // LiveMix
