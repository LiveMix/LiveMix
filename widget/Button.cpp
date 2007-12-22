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

#include <QPainter>
#include <QCoreApplication>
#include <QDebug>
#include <QSvgRenderer>

#include "Button.h"

namespace LiveMix
{

Button::Button(QWidget * pParent, const QString& sOnImage, const QString& sOffImage, const QString& sOverImage, const QString& sOffOverImage, QSize size, bool use_skin_style)
        : Toggle(pParent)
        , m_bPressed(false)
        , m_onPixmap(size)
        , m_offPixmap(size)
        , m_overPixmap(size)
        , m_offOverPixmap(size)
        , m_bMouseOver(false)
        , __use_skin_style(use_skin_style)
{
    // draw the background: slower but useful with transparent images!
    //setAttribute(Qt::WA_NoBackground);

    setMinimumSize(size);
    setMaximumSize(size);
    resize(size);

    if (loadImage(sOnImage, m_onPixmap) == false) {
        m_onPixmap.fill(QColor(0, 255, 0));
    }

    if (loadImage(sOffImage, m_offPixmap) == false) {
        m_offPixmap.fill(QColor(0, 100, 0));
    }

    if (loadImage(sOverImage, m_overPixmap) == false) {
        m_overPixmap.fill(QColor(0, 180, 0));
    }

    if (loadImage(sOffOverImage, m_offOverPixmap) == false) {
        m_offOverPixmap.fill(QColor(0, 180, 0));
    }

    // default text font
    m_textFont.setPointSize(9);
// m_textFont.setBold( true );
}
Button::~Button()
{}
QWidget* Button::getWidget()
{
    return this;
}

bool Button::loadImage(const QString& sFilename, QPixmap& pixmap)
{
    // load an image
    if (!pixmap.load(":/data/" + sFilename)) {
        if (sFilename != "") {
            qDebug() << "Error loading image: '" << sFilename << "'";
        }
        return false;
    }
    return true;
}


void Button::mousePressEvent(QMouseEvent*)
{
    m_bPressed = true;
    update();

    emit mousePress(this);
}


void Button::mouseReleaseEvent(QMouseEvent* ev)
{
    setPressed(false);

    if (ev->button() == Qt::LeftButton) {
        emit clicked();
        emit clicked(this);
    } else if (ev->button() == Qt::RightButton) {
        emit rightClicked(this);
        emit rightClick(ev);
    } else if (ev->button() == Qt::MidButton) {
        emit middleClick(ev);
    }
}


void Button::setPressed(bool pressed)
{
    if (pressed != m_bPressed) {
        m_bPressed = pressed;
        update();
    }
}


void Button::enterEvent(QEvent*)
{
    m_bMouseOver = true;
    update();
}


void Button::leaveEvent(QEvent*)
{
    m_bMouseOver = false;
    update();
}


void Button::draw(QPaintEvent *ev, QPainter &painter, QPixmap &pixmap)
{
    if (__use_skin_style) {
        static int w = 5;
        static int h = pixmap.height();

        // central section, scaled
        painter.drawPixmap(QRect(w, 0, width() - w * 2, h), pixmap, QRect(10, 0, w, h));

        // left side
        painter.drawPixmap(QRect(0, 0, w, h), pixmap, QRect(0, 0, w, h));

        // right side
        painter.drawPixmap(QRect(width() - w, 0, w, h), pixmap, QRect(pixmap.width() - w, 0, w, h));
    } else {
        painter.drawPixmap(ev->rect(), pixmap, ev->rect());
    }
}

void Button::paintEvent(QPaintEvent *ev)
{
    QPainter painter(this);

    // background
    if (m_bPressed) {
        if (m_bMouseOver) {
            draw(ev, painter, m_overPixmap);
        } else {
            draw(ev, painter, m_onPixmap);
        }
    } else {
        if (m_bMouseOver) {
            draw(ev, painter, m_offOverPixmap);
        } else {
            draw(ev, painter, m_offPixmap);
        }
    }


    if (m_sText != "") {
        painter.setFont(m_textFont);

        QColor shadow(150, 150, 150, 100);
        QColor text(10, 10, 10);

        if (m_bMouseOver) {
            shadow = QColor(150, 250, 150, 100);
        }

        // shadow
        painter.setPen(shadow);
        painter.drawText(1, 1, width(), height(), Qt::AlignHCenter | Qt::AlignVCenter,  m_sText);

        // text
        painter.setPen(text);
        painter.drawText(0, 0, width(), height(), Qt::AlignHCenter | Qt::AlignVCenter,  m_sText);
    }
}


void Button::setText(const QString& sText)
{
    m_sText = sText;
    update();
}


// :::::::::::::::::::::::::


ToggleButton::ToggleButton(QWidget *pParent, const QString& sOnImg, const QString& sOffImg, const QString& sOverImg, const QString& sOffOverImg, QSize size, bool use_skin_style)
        : Button(pParent, sOnImg, sOffImg, sOverImg, sOffOverImg, size, use_skin_style)
{}


ToggleButton::~ToggleButton()
{}


void ToggleButton::mousePressEvent(QMouseEvent* ev)
{
    if (ev == NULL || ev->button() == Qt::LeftButton) {
        if (m_bPressed) {
            m_bPressed = false;
        } else {
            m_bPressed = true;
        }
        update();

        emit clicked();
        emit clicked(this);
        emit valueChanged(this, 0);
    }
}

bool ToggleButton::getValue()
{
    return m_bPressed;
}
void ToggleButton::setValue(bool p_bValue, bool p_bEmit, int p_iSource)
{
    m_bPressed = p_bValue;
    update();

    if (p_bEmit) {
        emit clicked();
        emit clicked(this);
        emit valueChanged(this, p_iSource);
    }
}


void ToggleButton::mouseReleaseEvent(QMouseEvent* ev)
{
    // do nothing, this method MUST override Button's one
    if (ev->button() == Qt::RightButton) {
        emit rightClick(ev);
    } else if (ev->button() == Qt::MidButton) {
        emit middleClick(ev);
    }
}

Button* Button::create(QWidget* pParent)
{
    return new Button(pParent, "btn_followPH_on.png", "btn_followPH_off.png", "btn_followPH_on.png", "btn_followPH_over.png", QSize(21, 16), true);
}
ToggleButton* ToggleButton::create(QWidget* pParent)
{
    return new ToggleButton(pParent, "btn_followPH_on.png", "btn_followPH_off.png", "btn_followPH_on.png", "btn_followPH_over.png", QSize(21, 16), true);
}

}
; //LiveMix
