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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "KeyDo.h"

namespace LiveMix
{

KeyDo::KeyDo(Widget* p_pMatrix)
        : m_pMatrix(p_pMatrix)
{}
KeyDo::~KeyDo()
{}

KeyDoSelectChannel::KeyDoSelectChannel(Widget* p_pMatrix, Backend::ChannelType p_eType, QString p_sChannelName)
        : KeyDo(p_pMatrix)
        , m_eType(p_eType)
        , m_sChannelName(p_sChannelName)
{
}
KeyDoSelectChannel::~KeyDoSelectChannel()
{}
void KeyDoSelectChannel::action()
{
    m_pMatrix->doSelect(m_eType, m_sChannelName);
}

KeyDoChannelAction::KeyDoChannelAction(Widget* p_pMatrix, Backend::ElementType p_eElement, 
		QString p_sReatedChannelName, QString p_sDisplayReatedChannelName)
        : KeyDo(p_pMatrix)
        , m_eElement(p_eElement)
        , m_sReatedChannelName(p_sReatedChannelName)
        , m_sDisplayReatedChannelName(p_sDisplayReatedChannelName)
{
}
KeyDoChannelAction::~KeyDoChannelAction()
{}
void KeyDoChannelAction::action()
{
    m_pMatrix->action(m_pMatrix->getSelectedChanelType(), m_pMatrix->getSetectedChannelName(), m_eElement, m_sReatedChannelName, m_sDisplayReatedChannelName);
}

KeyDoDirectAction::KeyDoDirectAction(Widget* p_pMatrix, Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, 
		QString p_sReatedChannelName, QString p_sDisplayReatedChannelName)
        : KeyDo(p_pMatrix)
        , m_eType(p_eType)
        , m_sChannelName(p_sChannelName)
        , m_eElement(p_eElement)
        , m_sReatedChannelName(p_sReatedChannelName)
        , m_sDisplayReatedChannelName(p_sDisplayReatedChannelName)
{
}
KeyDoDirectAction::~KeyDoDirectAction()
{}
void KeyDoDirectAction::action()
{
    m_pMatrix->action(m_eType, m_sChannelName, m_eElement, m_sReatedChannelName, m_sDisplayReatedChannelName);
}
//QString KeyDoDirectAction::name() {
//	QString getDisplayFunction(Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, QString p_sReatedChannelName, bool p_bUpperFirst =true);
//	return "KeyDoDirectAction " + m_sChannelName + " " + m_sReatedChannelName + " " + m_pMatrix->getDisplayElement(m_eElement);
//}

}
; // LiveMix
