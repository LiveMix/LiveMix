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

#include "KeyDo.h"

namespace LiveMix
{

KeyDo::KeyDo(Widget* p_pMatrix)
        : m_pMatrix(p_pMatrix)
{}
KeyDo::~KeyDo()
{}

KeyDoSelectChannel::KeyDoSelectChannel(Widget* p_pMatrix, ChannelType p_eType, QString p_sChannelName)
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

KeyDoChannelAction::KeyDoChannelAction(Widget* p_pMatrix, ElementType p_eElement, QString p_sReatedChannelName)
        : KeyDo(p_pMatrix)
        , m_eElement(p_eElement)
        , m_sReatedChannelName(p_sReatedChannelName)
{
}
KeyDoChannelAction::~KeyDoChannelAction()
{}
void KeyDoChannelAction::action()
{
    m_pMatrix->action(m_pMatrix->getSelectedChanelType(), m_pMatrix->getSetectedChannelName(), m_eElement, m_sReatedChannelName);
}

KeyDoDirectAction::KeyDoDirectAction(Widget* p_pMatrix, ChannelType p_eType, QString p_sChannelName, ElementType p_eElement,
                                     QString p_sReatedChannelName)
        : KeyDo(p_pMatrix)
        , m_eType(p_eType)
        , m_sChannelName(p_sChannelName)
        , m_eElement(p_eElement)
        , m_sReatedChannelName(p_sReatedChannelName)
{
}
KeyDoDirectAction::~KeyDoDirectAction()
{}
void KeyDoDirectAction::action()
{
    m_pMatrix->action(m_eType, m_sChannelName, m_eElement, m_sReatedChannelName);
}

}
; // LiveMix
