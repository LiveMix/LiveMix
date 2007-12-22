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

#ifndef ACTION_H_
#define ACTION_H_

#include <QtGui/QWidget>

namespace LiveMix
{

class Action : public QWidget
{
    Q_OBJECT
public:
    Action(QWidget* p_pParent =NULL);
    virtual ~Action();

    virtual QWidget* getWidget() =0;
    virtual void mouseDoubleClickEvent(QMouseEvent *p_pEvent);

signals:
    void leftClick(QMouseEvent * p_pEvent);
    void rightClick(QMouseEvent * p_pEvent);
    void middleClick(QMouseEvent * p_pEvent);
    void emitMouseDoubleClickEvent(QMouseEvent *p_pEvent);
};

class Volume : public Action
{
    Q_OBJECT
public:
    Volume(QWidget* p_pParent =NULL);
    virtual ~Volume();

    virtual void setValue(float fValue, bool emit = false, int p_iSource =0) =0;
    virtual float getValue() =0;
    // in fact the external value is standanrd and internal in dB
    virtual void setDbValue(float fValue) =0;
    virtual float getDbValue() =0;
    virtual float getMinValue() =0;
    virtual float getMaxValue() =0;
    virtual void incValue(bool p_bDirection, int p_iStep =1) =0;

signals:
    void valueChanged(Volume* p_pVolume, int p_iSource);

};

class Toggle : public Action
{
    Q_OBJECT
public:
    Toggle(QWidget* p_pParent =NULL);
    virtual ~Toggle();

    virtual bool getValue() =0;
    virtual void setValue(bool p_bValue, bool p_bEmit = false, int p_iSource =0) =0;
};

}
; // LiveMix

#endif /*ACTION_H_*/
