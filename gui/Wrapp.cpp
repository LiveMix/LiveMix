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

#include "Wrapp.h"
#include "db.h"

namespace LiveMix
{

Wrapp::Wrapp(Widget* p_pMatrix, Action* p_pWidget, ChannelType p_eType, QString p_sChannelName, ElementType p_eElement,
             QString p_sReatedChannelName)
        : QObject()
        , m_pMatrix(p_pMatrix)
        , m_eType(p_eType)
        , m_sChannelName(p_sChannelName)
        , m_eElement(p_eElement)
        , m_sReatedChannelName(p_sReatedChannelName)
{
    connect(p_pWidget->getWidget(), SIGNAL(leftClick(QMouseEvent*)), this, SLOT(leftClick(QMouseEvent*)));
    connect(p_pWidget->getWidget(), SIGNAL(middleClick(QMouseEvent*)), this, SLOT(middleClick(QMouseEvent*)));
    connect(p_pWidget->getWidget(), SIGNAL(rightClick(QMouseEvent*)), this, SLOT(rightClick(QMouseEvent*)));
    connect(p_pWidget->getWidget(), SIGNAL(emitMouseDoubleClickEvent(QMouseEvent*)), this, SLOT(emitMouseDoubleClickEvent(QMouseEvent*)));
    /*    connect(p_pWidget, SIGNAL( leftClick(QMouseEvent*) ), this, SLOT( leftClick(QMouseEvent*) ) );
        connect(p_pWidget, SIGNAL( middleClick(QMouseEvent*) ), this, SLOT( middleClick(QMouseEvent*) ) );
        connect(p_pWidget, SIGNAL( rightClick(QMouseEvent*) ), this, SLOT( rightClick(QMouseEvent*) ) );*/
};
bool Wrapp::exec()
{
    return false;
}
void Wrapp::leftClick(QMouseEvent* p_ev)
{
    m_pMatrix->leftClick(m_eType, m_sChannelName, m_eElement, m_sReatedChannelName, p_ev);
};
void Wrapp::middleClick(QMouseEvent* p_ev)
{
    m_pMatrix->middleClick(m_eType, m_sChannelName, m_eElement, m_sReatedChannelName, p_ev);
};
void Wrapp::rightClick(QMouseEvent* p_ev)
{
    m_pMatrix->rightClick(m_eType, m_sChannelName, m_eElement, m_sReatedChannelName, p_ev);
};
void Wrapp::emitMouseDoubleClickEvent(QMouseEvent *p_pEvent)
{
    m_pMatrix->mouseDoubleClickEvent(m_eType, m_sChannelName, m_eElement, m_sReatedChannelName, p_pEvent);
}

WrappVolume::WrappVolume(Widget* p_pMatrix, Volume* p_pWidget, ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_sReatedChannelName)
        : Wrapp(p_pMatrix, p_pWidget, p_eType, p_sChannelName, p_eElement, p_sReatedChannelName)
        , m_pWidget(p_pWidget)
{
    connect(p_pWidget->getWidget(), SIGNAL(valueChanged(Volume*)), this, SLOT(valueChanged(Volume*)));
}
void WrappVolume::valueChanged(Volume* p_pVolume)
{
    Backend::instance()->getChannel(m_eType, m_sChannelName)->setFloatAttribute(m_eElement == PAN_BAL ? p_pVolume->getValue() : p_pVolume->getDbValue(), m_eElement, m_sReatedChannelName);

    if (m_eType == OUT && m_eElement == FADER) {
        m_pMatrix->showMessage(trUtf8("%1 value: %2.")
                               .arg(m_pMatrix->getDisplayFunction(m_eType, m_sChannelName, m_eElement, m_sReatedChannelName))
                               .arg(displayDb(p_pVolume->getValue(), p_pVolume->getMinValue())));
    } else {
        QString value;
        if (m_eElement == PAN_BAL) {
            char tmp[20];
            sprintf(tmp, "%#.2f", p_pVolume->getValue());
            value = tmp;
        } else {
            value = displayDb(p_pVolume->getValue(), p_pVolume->getMinValue());
        }
        m_pMatrix->showMessage(trUtf8("%1 \"%2\" %3 value: %4.").arg(m_pMatrix->getDisplayChannelType(m_eType))
                               .arg(m_pMatrix->getDisplayNameOfChannel(m_eType, m_sChannelName))
                               .arg(m_pMatrix->getDisplayFunction(m_eType, m_sChannelName, m_eElement, m_sReatedChannelName, false)).arg(value));
    }
}

Volume* WrappVolume::getVolume()
{
    return m_pWidget;
}

WrappToggle::WrappToggle(Widget* p_pMatrix, Toggle* p_pWidget, ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_sReatedChannelName)
        : Wrapp(p_pMatrix, p_pWidget, p_eType, p_sChannelName, p_eElement, p_sReatedChannelName)
        , m_pWidget(p_pWidget)
{
    connect(p_pWidget->getWidget(), SIGNAL(valueChanged(ToggleButton*)), this, SLOT(valueChanged(ToggleButton*)));
}
Toggle* WrappToggle::getToggle()
{
    return m_pWidget;
}
bool WrappToggle::exec()
{
    m_pWidget->setValue(!m_pWidget->getValue(), true);
    return true;
}
void WrappToggle::valueChanged(ToggleButton* p_pToggle)
{
    Backend::instance()->getChannel(m_eType, m_sChannelName)->setBoolAttribute(p_pToggle->getValue(), m_eElement, m_sReatedChannelName);
}

}
; // LiveMix
