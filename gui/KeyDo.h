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

#ifndef KEYDO_H_
#define KEYDO_H_


namespace LiveMix
{

class KeyDo
{
public:
    virtual ~KeyDo();
    virtual void action() =0;
    virtual QString name() =0;

protected:
    KeyDo(Widget* p_pMatrix);
    Widget* m_pMatrix;
};
class KeyDoSelectChannel : public KeyDo
{
public:
    KeyDoSelectChannel(Widget* p_pMatrix, Backend::ChannelType p_eType, QString p_sChannelName);
    ~KeyDoSelectChannel();
    void action();
    QString name() {return "KeyDoSelectChannel";};

    Backend::ChannelType m_eType;
    QString m_sChannelName;
};
class KeyDoChannelAction : public KeyDo
{
public:
    KeyDoChannelAction(Widget* p_pMatrix, Backend::ElementType p_eElement, QString p_sReatedChannelName, QString p_sDisplayReatedChannelName);
    ~KeyDoChannelAction();
    void action();
    QString name() {return "KeyDoChannelAction";};

    Backend::ElementType m_eElement;
    QString m_sReatedChannelName;
    QString m_sDisplayReatedChannelName;
};
class KeyDoDirectAction : public KeyDo
{
public:
    KeyDoDirectAction(Widget* p_pMatrix, Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, 
    		QString p_sReatedChannelName, QString p_sDisplayReatedChannelName);
    ~KeyDoDirectAction();
    void action();
    QString name() {return "KeyDoDirectAction " + m_sChannelName + " " + m_sReatedChannelName + " " + m_pMatrix->getDisplayElement(m_eElement);};

    Backend::ChannelType m_eType;
    QString m_sChannelName;
    Backend::ElementType m_eElement;
    QString m_sReatedChannelName;
    QString m_sDisplayReatedChannelName;
};

}
; // LiveMix

#endif /*KEYDO_H_*/
