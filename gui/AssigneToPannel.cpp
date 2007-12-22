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

#import "backend.h"

#import <QLabel>
#import <QDebug>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>

namespace LiveMix
{

AssigneToPannel::AssigneToPannel(QString p_sChannel, QString p_sFunction, bool p_bVolume, bool p_bOnlyDirrect, QKeySequence p_rActionOnChannelKeySequence
                                 , QKeySequence p_rSelectChannelKeySequence, QKeySequence p_rActionOnSelectedChannelKeySequence
                                 , unsigned char p_iChannel, unsigned int p_iController)
        : QDialog()
        , m_pActionOnChannel(new GetKeyField)
        , m_pSelectChannel(new GetKeyField)
        , m_pActionOnSelectedChannel(new GetKeyField)
        , m_iChannel(p_iChannel)
        , m_iController(p_iController)
{
    QGridLayout* layout = new QGridLayout();
    setLayout(layout);
// setTitle(trUtf8("Edit key binding"));

    m_pActionOnChannel->setKeySequence(p_rActionOnChannelKeySequence);
    m_pSelectChannel->setKeySequence(p_rSelectChannelKeySequence);
    m_pActionOnSelectedChannel->setKeySequence(p_rActionOnSelectedChannelKeySequence);

    layout->addWidget(m_pActionOnChannel, 1, 2);
    if (!p_bOnlyDirrect) {
        layout->addWidget(m_pSelectChannel, 2, 2);
        layout->addWidget(m_pActionOnSelectedChannel, 3, 2);
    }

    if (p_bVolume) {
        layout->addWidget(new QLabel(trUtf8("Key to select the %2 of the channel %1").arg(p_sFunction).arg(p_sChannel)), 1, 1);
    } else {
        layout->addWidget(new QLabel(trUtf8("Key to %2 the channel %1").arg(p_sFunction).arg(p_sChannel)), 1, 1);
    }

    if (!p_bOnlyDirrect) {
        layout->addWidget(new QLabel(trUtf8("Key to select the channel %1").arg(p_sChannel)), 2, 1);

        if (p_bVolume) {
            layout->addWidget(new QLabel(trUtf8("Key to select the %1 of selected channel").arg(p_sFunction)), 3, 1);
        } else {
            layout->addWidget(new QLabel(trUtf8("Key to %1 the selected channel").arg(p_sFunction)), 3, 1);
        }
    }
    m_pMidiLabel = new QLabel(m_iChannel == (unsigned char)-1 ? trUtf8("MIDI event: -") : trUtf8("MIDI event: %1 / %2").arg(m_iChannel).arg(m_iController));
    layout->addWidget(m_pMidiLabel, 4, 1);


    QWidget* buttons = new QWidget;
// layout->addWidget(buttons, 4, 1, 2, 2);
    layout->addWidget(buttons, 5, 1, 1, 2, Qt::AlignHCenter);
    QHBoxLayout* buttonsLayout = new QHBoxLayout;
    buttons->setLayout(buttonsLayout);
    QPushButton* ok = new QPushButton(trUtf8("OK"));
    QPushButton* cancel = new QPushButton(trUtf8("Cancel"));
    buttonsLayout->addWidget(ok);
    buttonsLayout->addWidget(cancel);
    ok->setDefault(true);

    connect(ok, SIGNAL(clicked(bool)), this, SLOT(okClicked(bool)));
    connect(cancel, SIGNAL(clicked(bool)), this, SLOT(cancelClicked(bool)));

    m_iTimer = startTimer(50);
}

AssigneToPannel::~AssigneToPannel()
{
    killTimer(m_iTimer);
}

void AssigneToPannel::timerEvent(QTimerEvent*)
{
    while (Backend::instance()->hasMidiEvent()) {
        snd_seq_event_t *ev = Backend::instance()->readMidiEvent();
        if (ev->type == SND_SEQ_EVENT_CONTROLLER) {
            m_iChannel = ev->data.control.channel;
            m_iController = ev->data.control.param;
            m_pMidiLabel->setText(trUtf8("MIDI event: %1 / %2").arg(m_iChannel).arg(m_iController));
        }
    }
}

void AssigneToPannel::done(int r)
{
    QDialog::done(r);
    killTimer(m_iTimer);
}

void AssigneToPannel::okClicked(bool /*p_bChecked*/)
{
    done(QDialog::Accepted);
}
void AssigneToPannel::cancelClicked(bool /*p_bChecked*/)
{
    done(QDialog::Rejected);
}

QKeySequence AssigneToPannel::getActionOnChannelKeySequence()
{
    return m_pActionOnChannel->getKeySequence();
}
QKeySequence AssigneToPannel::getSelectChannelKeySequence()
{
    return m_pSelectChannel->getKeySequence();
}
QKeySequence AssigneToPannel::getActionOnSelectedChannelKeySequence()
{
    return m_pActionOnSelectedChannel->getKeySequence();
}
unsigned char AssigneToPannel::getChannel()
{
    return m_iChannel;
}
unsigned int AssigneToPannel::getController()
{
    return m_iController;
}
}
; // LiveMix
