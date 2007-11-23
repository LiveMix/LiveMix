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

#ifndef KEYDO_H_
#define KEYDO_H_

#include "mixingmatrix.h"


namespace LiveMix
{

class KeyDo
{
public:
    virtual ~KeyDo();
    virtual void action() =0;
//    virtual QString name() =0;

protected:
    KeyDo(Widget* p_pMatrix);
    Widget* m_pMatrix;
};
class KeyDoSelectChannel : public KeyDo
{
public:
    KeyDoSelectChannel(Widget* p_pMatrix, ChannelType p_eType, QString p_sChannelName);
    ~KeyDoSelectChannel();
    void action();
    QString name() {
        return "KeyDoSelectChannel";
    };

    ChannelType m_eType;
    QString m_sChannelName;
};
class KeyDoChannelAction : public KeyDo
{
public:
    KeyDoChannelAction(Widget* p_pMatrix, ElementType p_eElement, QString p_sReatedChannelName);
    ~KeyDoChannelAction();
    void action();
    QString name() {
        return "KeyDoChannelAction";
    };

    ElementType m_eElement;
    QString m_sReatedChannelName;
};
class KeyDoDirectAction : public KeyDo
{
public:
    KeyDoDirectAction(Widget* p_pMatrix, ChannelType p_eType, QString p_sChannelName, ElementType p_eElement,
                      QString p_sReatedChannelName);
    ~KeyDoDirectAction();
    void action();
    QString name();

    ChannelType m_eType;
    QString m_sChannelName;
    ElementType m_eElement;
    QString m_sReatedChannelName;
};

}
; // LiveMix

#endif /*KEYDO_H_*/
