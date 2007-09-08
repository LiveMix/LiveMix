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

#ifndef ACTION_H_
#define ACTION_H_

#include <QtGui/QWidget>

namespace LiveMix
{

class Action : public QWidget
{
public:
    Action(QWidget* p_pParent =NULL);
    virtual ~Action();

signals:
    void rightClick(QKeyEvent * p_pEvent);
    void middleClick(QKeyEvent * p_pEvent);

};

class Volume : public Action
{
public:
    Volume(QWidget* p_pParent =NULL);
    virtual ~Volume();

    virtual void setValue(float fValue, bool emit = false) =0;
    virtual float getValue() =0;
    // in fact the external value is standanrd and internal in dB
    virtual void setDbValue(float fValue) =0;
    virtual float getDbValue() =0;
    virtual void incValue(bool p_bDirection) =0;
};

}
; // LiveMix

#endif /*ACTION_H_*/
