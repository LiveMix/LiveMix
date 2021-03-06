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

#include "LCD.h"

#include <QtGui/QPainter>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

namespace LiveMix
{

QPixmap* LCDDigit::m_pSmallBlueFontSet = NULL;
QPixmap* LCDDigit::m_pSmallRedFontSet = NULL;
QPixmap* LCDDigit::m_pLargeGrayFontSet = NULL;
QPixmap* LCDDigit::m_pSmallGrayFontSet = NULL;

LCDDigit::LCDDigit(QWidget * pParent, LCDType type)
        : QWidget(pParent)
        , m_type(type)
{
    setAttribute(Qt::WA_NoBackground);

    switch (m_type) {
    case SMALL_BLUE:
    case SMALL_RED:
        resize(8, 11);
        break;

    case LARGE_GRAY:
        resize(14, 16);
        break;

    case SMALL_GRAY:
        resize(12, 11);
        break;
    }

    setMinimumSize(width(), height());
    setMaximumSize(width(), height());

    // Small blue FontSet image
    if (m_pSmallBlueFontSet == NULL) {
        QString sSmallBlueFontSet = ":/data/LCDSmallBlueFontSet.png";
        m_pSmallBlueFontSet = new QPixmap();
        bool ok = m_pSmallBlueFontSet->load(sSmallBlueFontSet);
        if (ok == false) {
            qDebug() << "Error loading pixmap:" << sSmallBlueFontSet;
        }
    }

    // Small red FontSet image
    if (m_pSmallRedFontSet == NULL) {
        QString sSmallRedFontSet = ":/data/LCDSmallRedFontSet.png";
        m_pSmallRedFontSet = new QPixmap();
        bool ok = m_pSmallRedFontSet->load(sSmallRedFontSet);
        if (ok == false) {
            qDebug() << "Error loading pixmap:" << sSmallRedFontSet;
        }
    }

    // Large gray FontSet image
    if (m_pLargeGrayFontSet == NULL) {
        QString sLargeGrayFontSet = ":/data/LCDLargeGrayFontSet.png";
        m_pLargeGrayFontSet = new QPixmap();
        bool ok = m_pLargeGrayFontSet->load(sLargeGrayFontSet);
        if (ok == false) {
            qDebug() << "Error loading pixmap:" << sLargeGrayFontSet;
        }
    }

    // Small gray FontSet image
    if (m_pSmallGrayFontSet == NULL) {
        QString sSmallGrayFontSet = ":/data/LCDSmallGrayFontSet.png";
        m_pSmallGrayFontSet = new QPixmap();
        bool ok = m_pSmallGrayFontSet->load(sSmallGrayFontSet);
        if (ok == false) {
            qDebug() << "Error loading pixmap:" << sSmallGrayFontSet;
        }
    }

    set(' ');
}


LCDDigit::~LCDDigit()
{
// delete m_pSmallBlueFontSet;
// m_pSmallBlueFontSet = NULL;

// delete m_pSmallRedFontSet;
// m_pSmallRedFontSet = NULL;
}


void LCDDigit::mouseReleaseEvent(QMouseEvent*)
{
    emit digitClicked();
}


void LCDDigit::paintEvent(QPaintEvent*)
{
    int x = m_nCol * width();
    int y = m_nRow * height();

    QPainter painter(this);

    switch (m_type) {
    case SMALL_BLUE:
        painter.drawPixmap(rect(), *m_pSmallBlueFontSet, QRect(x, y, width(), height()));
        break;

    case SMALL_RED:
        painter.drawPixmap(rect(), *m_pSmallRedFontSet, QRect(x, y, width(), height()));
        break;

    case LARGE_GRAY:
        painter.drawPixmap(rect(), *m_pLargeGrayFontSet, QRect(x, y, width(), height()));
        break;

    case SMALL_GRAY:
        painter.drawPixmap(rect(), *m_pSmallGrayFontSet, QRect(x, y, width(), height()));
        break;

    default:
        ////ERRORLOG( "[paint] Unhandled type" );
        painter.setPen(Qt::blue);
        painter.drawText(rect(), Qt::AlignCenter, "!");
    }
}


void LCDDigit::set(QChar ch)
{
    // remove unsupported accent
    if (QString::fromUtf8("àáâ").contains(ch, Qt::CaseInsensitive)) {
        ch = 'a';
    } else if (QString::fromUtf8("ê").contains(ch, Qt::CaseInsensitive)) {
        ch = 'e';
    } else if (QString::fromUtf8("ìíîï").contains(ch, Qt::CaseInsensitive)) {
        ch = 'i';
    } else if (QString::fromUtf8("òóô").contains(ch, Qt::CaseInsensitive)) {
        ch = 'o';
    } else if (QString::fromUtf8("ùúû").contains(ch, Qt::CaseInsensitive)) {
        ch = 'u';
    } else if (QString::fromUtf8("ýÿ").contains(ch, Qt::CaseInsensitive)) {
        ch = 'y';
    }
    if (QString::fromUtf8("ÀÁÂ").contains(ch, Qt::CaseInsensitive)) {
        ch = 'A';
    } else if (QString::fromUtf8("ËÊ").contains(ch, Qt::CaseInsensitive)) {
        ch = 'E';
    } else if (QString::fromUtf8("ÌÍÎÏ").contains(ch, Qt::CaseInsensitive)) {
        ch = 'I';
    } else if (QString::fromUtf8("ÒÓÔ").contains(ch, Qt::CaseInsensitive)) {
        ch = 'O';
    } else if (QString::fromUtf8("ÙÚÛ").contains(ch, Qt::CaseInsensitive)) {
        ch = 'U';
    } else if (QString::fromUtf8("ÝŸ").contains(ch, Qt::CaseInsensitive)) {
        ch = 'Y';
    } else if (QString::fromUtf8("\"").contains(ch, Qt::CaseInsensitive)) {
        ch = QString::fromUtf8("”").at(0);
    }
    int MAXCOL = 66;
    const QChar keymap[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', ' ',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', ' ',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ' ', ' ',
//                              '-', ':', '/', '\\', ',', ';', '.', '-', ' ', ' ', '#', '~', '+', '*', '!', '?', '$', '?', '%', '&', '(', ')', '[', ']', '{', '}', '=', '<', '>',
//                              '?', '?', ' ', ' ', '?', '?', '?', '@', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',
//                              '|', '?', '?', '\'', '?', ' ', ' ',
//                              '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '.'

        '-', ':', '/', '\\', ',', ';', '.', '-', QString::fromUtf8("÷").at(0), QString::fromUtf8("×").at(0), '#', '~', '+', '*', '!', '?', '$',
        '?', '%', '&', '(', ')', '[', ']', '{', '}', '=', '<', '>', QString::fromUtf8("«").at(0), QString::fromUtf8("»").at(0), ' ', ' ',
        '?', QString::fromUtf8("°").at(0), QString::fromUtf8("ê").at(0), '@', QString::fromUtf8("€").at(0), QString::fromUtf8("£").at(0), QString::fromUtf8("¥").at(0),
        QString::fromUtf8("½").at(0), QString::fromUtf8("⅓").at(0), QString::fromUtf8("¼").at(0), QString::fromUtf8("¾").at(0), QString::fromUtf8("ä").at(0),
        QString::fromUtf8("ö").at(0), QString::fromUtf8("ü").at(0), QString::fromUtf8("ë").at(0), QString::fromUtf8("è").at(0), QString::fromUtf8("é").at(0),
        QString::fromUtf8("Ø").at(0), QString::fromUtf8("ç").at(0), QString::fromUtf8("Ç").at(0), QString::fromUtf8("Ä").at(0), QString::fromUtf8("Ö").at(0), QString::fromUtf8("Ü").at(0), QString::fromUtf8("È").at(0),
        QString::fromUtf8("É").at(0), QString::fromUtf8("ß").at(0), QString::fromUtf8("|").at(0), QString::fromUtf8("“").at(0), QString::fromUtf8("”").at(0),
        QString::fromUtf8("’").at(0), QString::fromUtf8("‚").at(0),
        ' ', ' ', '?', '?', '?', '?', '?', '?', QString::fromUtf8("↑").at(0), QString::fromUtf8("↓").at(0), QString::fromUtf8("←").at(0),
        QString::fromUtf8("→").at(0), QString::fromUtf8("↘").at(0), QString::fromUtf8("↙").at(0), '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',
        QString::fromUtf8("©").at(0), QString::fromUtf8("®").at(0)
        //¤↖↗⅔˝̋̏῀‛„‟
    };
    for (int n = 0; n < 142; n++) {
        if (keymap[ n ] == ch) {
            m_nCol = n % MAXCOL;
            m_nRow = n / MAXCOL;
            break;
        }
    }

    update();
}


void LCDDigit::setSmallRed()
{
    if (m_type != SMALL_RED) {
        m_type = SMALL_RED;
        update();
    }
}


void LCDDigit::setSmallBlue()
{
    if (m_type != SMALL_BLUE) {
        m_type = SMALL_BLUE;
        update();
    }
}



// ::::::::::::::::::




LCDDisplay::LCDDisplay(QWidget * pParent, LCDDigit::LCDType type, int nDigits, bool leftAlign)
        : QWidget(pParent)
        , m_sMsg("")
        , m_bLeftAlign(leftAlign)
{

    for (int n = 0; n < nDigits; n++) {
        LCDDigit *pDigit = new LCDDigit(this, type);
        if ((type == LCDDigit::LARGE_GRAY) || (type == LCDDigit::SMALL_GRAY)) {
            pDigit->move(pDigit->width() * n, 0);
        } else {
            pDigit->move(pDigit->width() * n + 2, 2);
        }
        connect(pDigit, SIGNAL(digitClicked()), this, SLOT(digitClicked()));
        m_pDisplay.push_back(pDigit);
    }

    if ((type == LCDDigit::LARGE_GRAY) || (type == LCDDigit::SMALL_GRAY)) {
        int w = m_pDisplay[ 0 ]->width() * nDigits;
        int h = m_pDisplay[ 0 ]->height();

        resize(w, h);
    } else {
        int w = m_pDisplay[ 0 ]->width() * nDigits + 4;
        int h = m_pDisplay[ 0 ]->height() + 4;

        resize(w, h);
    }
    setMinimumSize(width(), height());
    setMaximumSize(width(), height());

    setText("    ");

    QPalette defaultPalette;
    defaultPalette.setColor(QPalette::Background, QColor(58, 62, 72));
    this->setPalette(defaultPalette);

}




LCDDisplay::~LCDDisplay()
{
// for ( uint i = 0; i < m_pDisplay.size(); i++ ) {
//  delete m_pDisplay[ i ];
// }
}


void LCDDisplay::setText(const QString& sMsg)
{
    if (sMsg == m_sMsg) {
        return;
    }

    m_sMsg = sMsg;
    int nLen = sMsg.length();

    if (nLen > m_pDisplay.size()) {
        setToolTip(sMsg);
    } else {
        setToolTip("");
    }


    if (m_bLeftAlign) {
        for (int i = 0; i < (int)m_pDisplay.size(); ++i) {
            if (i < nLen) {
                m_pDisplay[ i ]->set(sMsg.at(i).toAscii());
            } else {
                m_pDisplay[ i ]->set(' ');
            }
        }
    } else {
        // right aligned
        int nPadding = 0;
        if (nLen < (int)m_pDisplay.size()) {
            nPadding = m_pDisplay.size() - nLen;
        } else {
            nLen = m_pDisplay.size();
        }

        for (int i = 0; i < nPadding; i++) {
            m_pDisplay[ i ]->set(' ');
        }

        for (int i = 0; i < nLen; i++) {
            m_pDisplay[ i + nPadding ]->set(sMsg.at(i).toAscii());
        }
    }
}



void LCDDisplay::setSmallRed()
{
    for (int i = 0; i < m_pDisplay.size(); i++) {
        m_pDisplay[ i ]->setSmallRed();
    }
}

void LCDDisplay::setSmallBlue()
{
    for (int i = 0; i < m_pDisplay.size(); i++) {
        m_pDisplay[ i ]->setSmallBlue();
    }
}

void LCDDisplay::digitClicked()
{
    emit displayClicked(this);
}

// :::::::::::::::::::



// used in PlayerControl
LCDSpinBox::LCDSpinBox(QWidget *pParent, int nDigits, LCDSpinBoxType type, int nMin, int nMax)
        : QWidget(pParent)
        , m_type(type)
        , m_fValue(0)
        , m_nMinValue(nMin)
        , m_nMaxValue(nMax)
{
    m_pDisplay = new LCDDisplay(this, LCDDigit::LARGE_GRAY, nDigits);
    connect(m_pDisplay, SIGNAL(displayClicked(LCDDisplay*)), this, SLOT(displayClicked(LCDDisplay*)));

    resize(m_pDisplay->width(), m_pDisplay->height());
    setMinimumSize(width(), height());
    setMaximumSize(width(), height());

    setValue(0);
}



LCDSpinBox::~LCDSpinBox()
{
    delete m_pDisplay;
// delete m_pUpBtn;
// delete m_pDownBtn;
}


void LCDSpinBox::upBtnClicked()
{
    switch (m_type) {
    case INTEGER:
        if (m_nMaxValue != -1 && m_fValue < m_nMaxValue) {
            setValue(m_fValue + 1);
        }
        break;
    case FLOAT:
        if (m_nMaxValue != -1 && m_fValue < (float)m_nMaxValue) {
            setValue(m_fValue + 1.0);
        }
        break;
    }

    emit changed(this);
}

void LCDSpinBox::downBtnClicked()
{
    switch (m_type) {
    case INTEGER:
        if (m_nMinValue != -1 && m_fValue > m_nMinValue) {
            setValue(m_fValue -1);
        }
        break;
    case FLOAT:
        if (m_nMinValue != -1 && m_fValue > m_nMinValue) {
            setValue(m_fValue - 1.0);
        }
        break;
    }
    emit changed(this);
}


void LCDSpinBox::setValue(float nValue)
{
    switch (m_type) {
    case INTEGER:
        if (nValue != m_fValue) {
            m_fValue = (int)nValue;
            m_pDisplay->setText(QString("%1").arg(m_fValue));
        }
        break;

    case FLOAT:
        if (nValue != m_fValue) {
            m_fValue = nValue;
            char tmp[20];
            sprintf(tmp, "%#.2f", m_fValue);
            m_pDisplay->setText(QString("%1").arg(tmp));
        }
        break;
    }
}

void LCDSpinBox::displayClicked(LCDDisplay */*pRef*/)
{
    emit spinboxClicked();
}


void LCDSpinBox::wheelEvent(QWheelEvent *ev)
{
    ev->accept();

    if (ev->delta() > 0) {
        switch (m_type) {
        case INTEGER:
            setValue(m_fValue + 1);
            break;
        case FLOAT:
            setValue(m_fValue + 1.0);
            break;
        }

        emit changed(this);
    } else {
        switch (m_type) {
        case INTEGER:
            setValue(m_fValue -1);
            break;
        case FLOAT:
            setValue(m_fValue - 1.0);
            break;
        }
        emit changed(this);
    }
}

}
; //LiveMix
