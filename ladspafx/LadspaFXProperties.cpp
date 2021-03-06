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

#include "LadspaFXProperties.h"
#include "LadspaFXSelector.h"

#include "effects.h"
#include "FaderName.h"
#include "backend.h"

#include <QPixmap>
#include <QTimer>
#include <QDebug>
#include <QLabel>
#include <QShowEvent>
#include <QFrame>
#include <QCloseEvent>
#include <QMessageBox>
#include <QMenu>
#include <QGroupBox>

#include <typeinfo>


namespace LiveMix
{

LadspaFXProperties::LadspaFXProperties(QWidget* parent, effect *nLadspaFX, int p_iFaderHeight)
        : QWidget(parent)
        , m_iFaderHeight(p_iFaderHeight)
{
// qDebug() << "INIT";

    m_nLadspaFX = nLadspaFX;

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->setSpacing(0);
    hbox->setMargin(0);
    setLayout(hbox);

    m_pScrollArea = new QGroupBox(NULL);
    hbox->addWidget(m_pScrollArea);

    m_pScrollArea->move(0, 0);
//    m_pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    m_pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pScrollArea->resize(width(), height());

    m_pFrame = new QWidget(m_pScrollArea);
    m_pFrame->resize(width(), height() - 5);

//    ((QGroupBox*)m_pScrollArea)->addWidget(m_pFrame);

    m_pNameLbl = new QLabel(this);
    m_pNameLbl->move(10, 17);
//    m_pNameLbl->resize(270, 24);

    QFont boldFont;
    boldFont.setBold(true);
    m_pNameLbl->setFont(boldFont);

    m_pActivateBtn = ToggleButton::create(this);
    m_pActivateBtn->setText(trUtf8("M"));
    m_pActivateBtn->setToolTip(trUtf8("Mute"));
    connect(m_pActivateBtn, SIGNAL(clicked()), this, SLOT(activateBtnClicked()));

    m_pRemoveBtn = Button::create(this);
    m_pRemoveBtn->setText(trUtf8("X"));
    m_pRemoveBtn->setToolTip(trUtf8("Remove"));
    connect(m_pRemoveBtn, SIGNAL(clicked()), this, SLOT(removeBtnClicked()));

    m_pLeftBtn = Button::create(this);
    m_pLeftBtn->setText(trUtf8("<"));
    m_pLeftBtn->setToolTip(trUtf8("Move left"));
    connect(m_pLeftBtn, SIGNAL(clicked()), this, SLOT(leftBtnClicked()));

    m_pRightBtn = Button::create(this);
    m_pRightBtn->setText(trUtf8(">"));
    m_pRightBtn->setToolTip(trUtf8("Move right"));
    connect(m_pRightBtn, SIGNAL(clicked()), this, SLOT(rightBtnClicked()));

    m_pTimer = new QTimer(this);
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(updateOutputControls()));

    updateControls();
    setFaderHeight(p_iFaderHeight);

    m_pTimer->start(100);
}


LadspaFXProperties::~LadspaFXProperties()
{
// qDebug() << "DESTROY";
    disconnect(m_pRemoveBtn, SIGNAL(clicked()), this, SLOT(removeBtnClicked()));
    disconnect(m_pLeftBtn, SIGNAL(clicked()), this, SLOT(leftBtnClicked()));
    disconnect(m_pRightBtn, SIGNAL(clicked()), this, SLOT(rightBtnClicked()));
    disconnect(m_pTimer, SIGNAL(timeout()), this, SLOT(updateOutputControls()));

    delete m_pScrollArea;
//    delete m_pFrame;
    delete m_pNameLbl;
    delete m_pActivateBtn;
    delete m_pRemoveBtn;
    delete m_pLeftBtn;
    delete m_pRightBtn;
    delete m_pTimer;

    m_pScrollArea = NULL;
//    delete m_pFrame;
    m_pNameLbl = NULL;
    m_pActivateBtn = NULL;
    m_pRemoveBtn = NULL;
    m_pLeftBtn = NULL;
    m_pRightBtn = NULL;
    m_pTimer = NULL;
}

void LadspaFXProperties::closeEvent(QCloseEvent *ev)
{
    ev->accept();
}

void LadspaFXProperties::toggleChanged(ToggleButton* ref)
{
#ifdef LADSPA_SUPPORT
    for (int i = 0; i < m_pInputControlFaders.size(); i++) {
        if (ref == m_pInputControlFaders[ i ]) {
            LadspaControlPort *pControl = m_nLadspaFX->fx->inputControlPorts[ i ];
            pControl->m_fControlValue = (int)ref->getValue();
        }
    }
#endif
}

void LadspaFXProperties::faderChanged(Volume* ref)
{
    ((Fader*)ref)->setPeak_L(ref->getValue());
    ((Fader*)ref)->setPeak_R(ref->getValue());

#ifdef LADSPA_SUPPORT
    for (int i = 0; i < m_pInputControlFaders.size(); i++) {
        if (ref == m_pInputControlFaders[ i ]) {
            LadspaControlPort *pControl = m_nLadspaFX->fx->inputControlPorts[ i ];

            pControl->m_fControlValue = ((Fader*)ref)->getLinDb() ? ref->getValue() : ref->getDbValue();

            QString sValue;
            if (pControl->m_fControlValue < 1.0) {
                sValue = QString("%1").arg(pControl->m_fControlValue, 0, 'f', 2);
            } else if (pControl->m_fControlValue < 100.0) {
                sValue = QString("%1").arg(pControl->m_fControlValue, 0, 'f', 1);
            } else {
                sValue = QString("%1").arg(pControl->m_fControlValue, 0, 'f', 0);
            }
            m_pInputControlLabel[ i ]->setText(sValue);
        }
    }
#endif
}

void LadspaFXProperties::updateControls()
{
#ifdef LADSPA_SUPPORT
// qDebug() << "*** [updateControls] ***";
    if (m_nLadspaFX->fx) {
        int nControlsFrameWidth = 40 + 45 * (m_nLadspaFX->fx->inputControlPorts.size() + m_nLadspaFX->fx->outputControlPorts.size());
        if (m_nLadspaFX->fx->inputControlPorts.size() > 0 && m_nLadspaFX->fx->outputControlPorts.size() > 0) {
            nControlsFrameWidth += 10;
        }
        if (nControlsFrameWidth < 100) {
            nControlsFrameWidth = 100;
        }

        setFixedSize(nControlsFrameWidth, height());
        m_pFrame->resize(width(), height() - 5);
        m_pActivateBtn->move(nControlsFrameWidth - 51, 6);
        m_pRemoveBtn->move(nControlsFrameWidth - 27, 6);
        m_pLeftBtn->move(nControlsFrameWidth - 99, 6);
        m_pRightBtn->move(nControlsFrameWidth - 75, 6);
        m_pNameLbl->resize(nControlsFrameWidth - 13, 24);

        m_pNameLbl->setText(m_nLadspaFX->fx->getPluginName());
        m_pNameLbl->setToolTip(m_nLadspaFX->fx->getPluginName());

        // input controls
        uint nInputControl_X = 10;
        for (int i = 0; i < m_nLadspaFX->fx->inputControlPorts.size(); i++) {
            LadspaControlPort *pControlPort = m_nLadspaFX->fx->inputControlPorts[ i ];

            nInputControl_X = 10 + 45 * i;

            LCDDisplay *pLCD = new LCDDisplay(m_pFrame, LCDDigit::SMALL_BLUE, 5);
            pLCD->move(nInputControl_X + 5, 40);
            QPalette lcdPalette;
            lcdPalette.setColor(QPalette::Background, QColor(58, 62, 72));
            pLCD->setPalette(lcdPalette);
            pLCD->setToolTip(pControlPort->m_sName);

            m_pInputControlLabel.push_back(pLCD);

            FaderName *pName = new FaderName(m_pFrame);
            pName->move(nInputControl_X + 8, 60);
            pName->setText(pControlPort->m_sName);
            m_pInputControlNames << pName;
            pName->setToolTip(pControlPort->m_sName);

            if (pControlPort->m_bToggle) {
                pLCD->hide();

                ToggleButton *pToggle = ToggleButton::create(m_pFrame);
                connect(pToggle, SIGNAL(valueChanged(ToggleButton*, int)), this, SLOT(toggleChanged(ToggleButton*)));
                m_pInputControlFaders.push_back(pToggle);
                pToggle->move(nInputControl_X + 24, m_iFaderHeight + 23);

                pToggle->setValue((bool)pControlPort->m_fControlValue);
                pToggle->setToolTip(pControlPort->m_sName);
                toggleChanged(pToggle);
            } else {
                // fader
                Fader *pFader = new Fader(m_pFrame, pControlPort->m_bInteger, !pControlPort->m_bLogarithmic, Fader::FADER_PK);
                WrappFXFader *pWrappFXFader = new WrappFXFader(pFader);

//                connect(pFader, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenu(const QPoint &)));
                connect(pWrappFXFader, SIGNAL(rightClick(QMouseEvent*, Fader*)), this, SLOT(contextMenu(QMouseEvent*, Fader*)));
                connect(pFader, SIGNAL(valueChanged(Volume*, int)), this, SLOT(faderChanged(Volume*)));
                m_pInputControlFaders.push_back(pFader);
                pFader->move(nInputControl_X + 25, 56);
                pFader->setToolTip(pControlPort->m_sName);

                if (pControlPort->m_bLogarithmic) {
                    pFader->setDbMaxValue(pControlPort->m_fUpperBound);
                    pFader->setDbMinValue(pControlPort->m_fLowerBound);
                    pFader->setDbMaxPeak(pControlPort->m_fUpperBound);
                    pFader->setDbMinPeak(pControlPort->m_fLowerBound);
                    pFader->setDbValue(pControlPort->m_fControlValue);
                } else {
                    pFader->setMaxValue(pControlPort->m_fUpperBound);
                    pFader->setMinValue(pControlPort->m_fLowerBound);
                    pFader->setMaxPeak(pControlPort->m_fUpperBound);
                    pFader->setMinPeak(pControlPort->m_fLowerBound);
                    pFader->setValue(pControlPort->m_fControlValue);
                }
                faderChanged(pFader);
            }
        }

        if (m_nLadspaFX->fx->inputControlPorts.size() > 0) {
            nInputControl_X += 55;
        }

        for (int i = 0; i < m_nLadspaFX->fx->outputControlPorts.size(); i++) {
            LadspaControlPort *pControl = m_nLadspaFX->fx->outputControlPorts[ i ];
            uint xPos = nInputControl_X + 10 + 45 * i;

            LCDDisplay *pLCD = new LCDDisplay(m_pFrame, LCDDigit::SMALL_BLUE, 5);
            pLCD->move(xPos + 5, 40);
            QPalette lcdPalette;
            lcdPalette.setColor(QPalette::Background, QColor(58, 62, 72));
            pLCD->setPalette(lcdPalette);
            m_pOutputControlLabel.push_back(pLCD);
            pLCD->setToolTip(pControl->m_sName);

            FaderName *pName = new FaderName(m_pFrame);
            pName->move(xPos + 10, 60);
            pName->setText(pControl->m_sName);
            m_pOutputControlNames.push_back(pName);
            pName->setToolTip(pControl->m_sName);

            // fader
            Fader *pFader = new Fader(m_pFrame, false, true, Fader::PK_VU);   // without knob!
            pFader->move(xPos + 27, 56);
            pFader->setToolTip(pControl->m_sName);

            pFader->setMaxPeak(pControl->m_fUpperBound);
            pFader->setMinPeak(pControl->m_fLowerBound);

            m_pOutputControlFaders.push_back(pFader);
        }
    } else {
        qDebug() << "NULL PLUGIN";
        m_pNameLbl->setText(trUtf8("No plugin"));
        m_pActivateBtn->setEnabled(false);
    }
#endif
}

LadspaFX* LadspaFXProperties::getFXSelector(LadspaFX* oldFx)
{
#ifdef LADSPA_SUPPORT
    LadspaFXSelector fxSelector(oldFx);
    if (fxSelector.exec() == QDialog::Accepted) {
        QString sSelectedFX = fxSelector.getSelectedFX();
        if (sSelectedFX != "") {
//   LadspaFX *m_nLadspaFX->fx = NULL;

            QList<LadspaFXInfo*> pluginList = Effects::getInstance()->getPluginList();
            for (int i = 0; i < pluginList.size(); i++) {
                LadspaFXInfo *fxInfo = pluginList[i];
                if (fxInfo->m_sName == sSelectedFX) {
                    return LadspaFX::load(fxInfo->m_sFilename, fxInfo->m_sLabel, Backend::instance()->getSampleRate());
//     m_nLadspaFX->fx->setEnabled( true );
                    break;
                }
            }
        } else { // no plugin selected
            qDebug() << "no plugin selected";
            return NULL;
        }
    }
#endif
    return NULL;
}

void LadspaFXProperties::updateOutputControls()
{
#ifdef LADSPA_SUPPORT

// qDebug() << "[updateOutputControls]";

    if (m_nLadspaFX->fx) {
        QString title = trUtf8("%1 (%2 %)").arg(m_nLadspaFX->fx->getPluginName()).arg(m_nLadspaFX->m_fCpuUse * 100, 0, 'f', 2);
        m_pNameLbl->setText(title);
        m_pNameLbl->setToolTip(title);

        if (m_nLadspaFX->fx->isEnabled()) {
            m_pActivateBtn->setToolTip(trUtf8("Deactivate"));
            m_pActivateBtn->setPressed(false);
        } else {
            m_pActivateBtn->setToolTip(trUtf8("Activate"));
            m_pActivateBtn->setPressed(true);
        }

        for (int i = 0; i < m_nLadspaFX->fx->outputControlPorts.size(); i++) {
            LadspaControlPort *pControl = m_nLadspaFX->fx->outputControlPorts[i];

            {
                QList<Fader*>::iterator it = m_pOutputControlFaders.begin() + i;
                if (it != m_pOutputControlFaders.end()) {
                    Fader *pFader = *it;
                    if (pFader == NULL) {
                        qDebug() << "[updateOutputControls] pFader = NULL";
                        continue;
                    }

                    //    float fValue = pControl->m_fControlValue;
                    //    float fInterval = pControl->m_fUpperBound - pControl->m_fLowerBound;
                    float fValue = pControl->m_fControlValue;// / fInterval;
                    //    if (fValue < 0) fValue = -fValue;

                    pFader->setPeak_L(fValue);
                    pFader->setPeak_R(fValue);
                }
            }
            {
                QList<LCDDisplay*>::iterator it = m_pOutputControlLabel.begin() + i;
                if (it != m_pOutputControlLabel.end()) {
                    LCDDisplay *pLabel = *it;
                    if (pLabel == NULL) {
                        qDebug() << "[updateOutputControls] pLabel = NULL";
                        continue;
                    }

                    QString sValue;
                    if (pControl->m_fControlValue < 1.0) {
                        sValue = QString("%1").arg(pControl->m_fControlValue, 0, 'f', 2);
                    } else if (pControl->m_fControlValue < 100.0) {
                        sValue = QString("%1").arg(pControl->m_fControlValue, 0, 'f', 1);
                    } else {
                        sValue = QString("%1").arg(pControl->m_fControlValue, 0, 'f', 0);
                    }
                    pLabel->setText(sValue);
                }
            }
        }
    } else {
        m_pActivateBtn->setEnabled(true);
    }
#endif
}

void LadspaFXProperties::removeBtnClicked()
{
    emit removeClicked(this, m_nLadspaFX);
}

void LadspaFXProperties::activateBtnClicked()
{
#ifdef LADSPA_SUPPORT
    if (m_nLadspaFX->fx) {
        m_nLadspaFX->fx->setEnabled(!m_nLadspaFX->fx->isEnabled());
    }
#endif
}

void LadspaFXProperties::leftBtnClicked()
{
    emit leftClicked(this, m_nLadspaFX);
}

void LadspaFXProperties::rightBtnClicked()
{
    emit rightClicked(this, m_nLadspaFX);
}

void LadspaFXProperties::setFaderHeight(int p_iHeight)   //195
{
//    m_pNameLbl->move(10, 17);
//    m_pNameLbl->resize(270, 24);

    m_iFaderHeight = p_iHeight;
    setFixedHeight(p_iHeight + 64);
    m_pFrame->resize(width(), height() - 5);
//    m_pNameLbl->resize(p_iHeight + 75, 24);
    foreach(FaderName* label, m_pInputControlNames) {
        label->setFixedHeight(p_iHeight - 15);
    }
    foreach(QWidget* fader, m_pInputControlFaders) {
        if (typeid(*fader).name() == typeid(Fader).name()) {
            ((Fader*)fader)->setFixedHeight(p_iHeight);
        } else {
            fader->move(fader->x(), p_iHeight + 23);
        }
    }
    foreach(FaderName* label, m_pOutputControlNames) {
        label->setFixedHeight(p_iHeight - 15);
    }
    foreach(Fader* fader, m_pOutputControlFaders) {
        fader->setFixedHeight(p_iHeight);
    }
}
void LadspaFXProperties::contextMenu(QMouseEvent *p_pEvent, Fader *p_pFader)
{
    QMenu menu(this);

//    QAction* assigne = new QAction(trUtf8("Assigne key"), this);
//    connect(assigne, SIGNAL(triggered()), this, SLOT(assigneKey()));
//    menu.addAction(assigne);

    QAction* set = new QAction(trUtf8("Set value..."), this);
    connect(set, SIGNAL(triggered()), p_pFader , SLOT(openSetValue()));
    menu.addAction(set);

    QAction* reset = new QAction(trUtf8("Reset the effect"), this);
    connect(reset, SIGNAL(triggered()), this, SLOT(reset()));
    menu.addAction(reset);

    menu.exec(p_pEvent->globalPos());
}
void LadspaFXProperties::reset()
{
    for (int i = 0; i < m_pInputControlFaders.size(); i++) {
        QWidget *w = m_pInputControlFaders[i];
        LadspaControlPort *pControl = m_nLadspaFX->fx->inputControlPorts[ i ];
        if (typeid(*w).name() == typeid(Fader).name()) {
            Fader *f = (Fader*)w;
            if (f->getLinDb()) {
                f->setValue(pControl->m_fDefaultControlValue);
            } else {
                f->setDbValue(pControl->m_fDefaultControlValue);
            }
            faderChanged(f);
        } else {
            ToggleButton *t = (ToggleButton*)w;
            t->setValue(pControl->m_fDefaultControlValue);
            toggleChanged(t);
        }
    }
}

WrappFXFader::WrappFXFader(Fader* p_pFader)
        : m_pFader(p_pFader)
{
    connect(m_pFader, SIGNAL(rightClick(QMouseEvent*)), this, SLOT(rightClicked(QMouseEvent*)));
}
void WrappFXFader::rightClicked(QMouseEvent *p_pEvent)
{
    emit rightClick(p_pEvent, m_pFader);
}

}
; //LiveMix
