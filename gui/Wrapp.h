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

#ifndef WRAPP_H_
#define WRAPP_H_

#include "mixingmatrix.h"
#include "backend.h"
#include "Action.h"
#include "Button.h"

namespace LiveMix
{

class Widget;

class Wrapp : public QObject
{
    Q_OBJECT
public:
    Wrapp(Widget* p_pMatrix, Action* p_pWidget, ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, 
    		QString p_sReatedChannelName);

    virtual bool exec();

public slots:
    void leftClick(QMouseEvent* p_ev);
    void middleClick(QMouseEvent* p_ev);
    void rightClick(QMouseEvent* p_ev);

protected:
    Widget* m_pMatrix;

    ChannelType m_eType;
    QString m_sChannelName;
    ElementType m_eElement;
    QString m_sReatedChannelName;
};

class WrappVolume : public Wrapp
{
    Q_OBJECT
public:
    WrappVolume(Widget* p_pMatrix, Volume* p_pWidget, ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_sReatedChannelName);
    Volume* getVolume();

private:
    Volume* m_pWidget;

private slots:
    void valueChanged(Volume* p_pVolume);
};

class WrappToggle : public Wrapp
{
    Q_OBJECT
public:
    WrappToggle(Widget* p_pMatrix, ToggleButton* p_pWidget, ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_sReatedChannelName);

    bool exec();

private:
    ToggleButton* m_pWidget;

private slots:
    void valueChanged(ToggleButton* p_pToggle);
};

}
; // LiveMix

#endif /*WRAPP_H_*/
