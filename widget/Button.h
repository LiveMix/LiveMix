/*
 * Hydrogen
 * Copyright(c) 2002-2007 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 * http://www.hydrogen-music.org
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


#ifndef BUTTON_H
#define BUTTON_H

#include "Action.h"
//#include "PixmapWidget.h"

#include <QtGui/QMouseEvent>
#include <QtCore/QEvent>
#include <QtGui/QPixmap>
#include <QtGui/QWidget>

namespace LiveMix
{

/**
 * Generic Button with pixmaps and text.
 */
class Button : public Toggle
{
    Q_OBJECT

public:
    Button(
        QWidget *pParent,
        const QString& sOnImg,
        const QString& sOffImg,
        const QString& sOverImg,
        const QString& sOffOverImg,
        QSize size,
        bool use_skin_style
    );
    virtual ~Button();
    virtual QWidget* getWidget();

    bool isPressed() {
        return m_bPressed;
    }
    void setPressed(bool pressed);

    void setText(const QString& sText);

    static Button* create(QWidget* =NULL);

    virtual bool getValue() {
        return false;
    };
    virtual void setValue(bool, bool = false) {};

signals:
    void clicked();
    void clicked(Button *pBtn);
    void rightClicked(Button *pBtn);
    void mousePress(Button *pBtn);

protected:
    QString _channel;
    QString _channel2;

    bool m_bPressed;

    QFont m_textFont;
    QString m_sText;

    QPixmap m_onPixmap;
    QPixmap m_offPixmap;
    QPixmap m_overPixmap;
    QPixmap m_offOverPixmap;

private:
    bool m_bMouseOver;
    bool __use_skin_style;

    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void enterEvent(QEvent *ev);
    void leaveEvent(QEvent *ev);
    void paintEvent(QPaintEvent* ev);

    void draw(QPaintEvent *ev, QPainter &painter, QPixmap &pixmap);
    bool loadImage(const QString &sFilename, QPixmap &pixmap);
};



/**
 * A ToggleButton (On/Off).
 */
class ToggleButton : public Button
{
    Q_OBJECT

public:
    ToggleButton(QWidget *pParent, const QString& sOnImg, const QString& sOffImg, const QString& sOverImg, const QString& sOffOverImg, QSize size, bool use_skin_style);
    ~ToggleButton();

    static ToggleButton* create(QWidget* =NULL);

    virtual bool getValue();
    virtual void setValue(bool p_bValue, bool p_bEmit = false);

    void mousePressEvent(QMouseEvent *ev);

signals:
    void valueChanged(ToggleButton* ref);

private:
    void mouseReleaseEvent(QMouseEvent *ev);
};

}
; //LiveMix

#endif
