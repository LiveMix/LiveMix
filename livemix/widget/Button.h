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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#ifndef BUTTON_H
#define BUTTON_H

//#include "PixmapWidget.h"

#include <QtGui/QMouseEvent>
#include <QtCore/QEvent>
#include <QtGui/QPixmap>
#include <QtGui/QWidget>

namespace JackMix
{

/**
 * Generic Button with pixmaps and text.
 */
class Button : public QWidget
{
    Q_OBJECT

public:
    Button(
        QWidget *pParent,
        const QString& sOnImg,
        const QString& sOffImg,
        const QString& sOverImg,
        QSize size,
        bool use_skin_style,
        QString channel ="",
        QString channel2 =""
    );
    virtual ~Button();

    bool isPressed()
    {
        return m_bPressed;
    }
    void setPressed(bool pressed);

    void setText( const QString& sText );

    static Button* create(QWidget* =NULL, QString channel =NULL, QString channel2 =NULL);

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

private:
    bool m_bMouseOver;
    bool __use_skin_style;

    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void enterEvent(QEvent *ev);
    void leaveEvent(QEvent *ev);
    void paintEvent( QPaintEvent* ev);

    bool loadImage( const QString& sFilename, QPixmap& pixmap );
};




/**
 * A ToggleButton (On/Off).
 */
class ToggleButton : public Button
{
    Q_OBJECT

public:
    ToggleButton( QWidget *pParent, const QString& sOnImg, const QString& sOffImg, const QString& sOverImg, QSize size, bool use_skin_style, QString channel, QString channel2 ="" );
    ~ToggleButton();

    static ToggleButton* create(QWidget* =NULL, QString channel ="", QString channel2 ="");
    static ToggleButton* createEdit(QWidget*, QString channel ="");
    static ToggleButton* createMute(QWidget*, QString channel ="");
    static ToggleButton* createNew(QWidget*, QString channel ="");
    static ToggleButton* createPlay(QWidget*, QString channel ="");
    static ToggleButton* createSolo(QWidget*, QString channel ="");
    static ToggleButton* createByPass(QWidget*, QString channel ="");

    bool getValue();
    void setValue(bool value);

signals:
    void valueChanged(ToggleButton* ref);
    void valueChanged(QString channel, bool value);
    void valueChanged(QString channel, QString channel2, bool value);

private:
    void mousePressEvent( QMouseEvent *ev );
    void mouseReleaseEvent( QMouseEvent *ev );
};

}
; //JackMix

#endif
