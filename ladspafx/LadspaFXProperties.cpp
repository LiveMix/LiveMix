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

#include "LadspaFXProperties.h"
#include "LadspaFXSelector.h"

#include "Effects.h"
#include "FaderName.h"
#include "backend.h"

#include <QtGui/QPixmap>
#include <QtCore/QTimer>
#include <QtCore/QDebug>
#include <QtGui/QLabel>
#include <QtGui/QShowEvent>
#include <QtGui/QFrame>
#include <QtGui/QCloseEvent>

namespace JackMix
{

LadspaFXProperties::LadspaFXProperties(QWidget* parent, struct effect *nLadspaFX)
        : QWidget(parent)
{
// qDebug() << "INIT";

    m_nLadspaFX = nLadspaFX;

// resize( 500, 200 );
// setMinimumSize( width(), height() );
// setFixedHeight( height() );
    setFixedHeight( 259 );
// setWindowIcon( QPixmap( QCoreApplication::applicationDirPath() + "/data/icon16.png" ) );

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->setSpacing( 0 );
    hbox->setMargin( 0 );
    setLayout( hbox );


    // Background image
// QPixmap background;
// bool ok = background.load( QCoreApplication::applicationDirPath() + "/data/mixerPanel/mixer_background.png" );
// if( !ok ){
//  qDebug() << "Error loading pixmap";
// }


    m_pScrollArea = new QScrollArea( NULL );
    hbox->addWidget( m_pScrollArea );

    m_pScrollArea->move( 0, 0 );
    m_pScrollArea->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    m_pScrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    m_pScrollArea->resize( width(), height() );

    m_pFrame = new QFrame( this );
    m_pFrame->resize( width(), height() - 5 );

    m_pScrollArea->setWidget( m_pFrame );

    m_pNameLbl = new QLabel(this);
    m_pNameLbl->move( 10, 17 );
    m_pNameLbl->resize( 270, 24 );

    QFont boldFont;
    boldFont.setBold(true);
    m_pNameLbl->setFont( boldFont );

// m_pSelectFXBtn = new QPushButton( trUtf8("Select FX"), this);
// m_pSelectFXBtn->move( 280, 10 );
// m_pSelectFXBtn->resize( 100, 24 );
// connect( m_pSelectFXBtn, SIGNAL(clicked()), this, SLOT(selectFXBtnClicked()) );

// m_pActivateBtn = new QPushButton( trUtf8("Activate"), this);
// m_pActivateBtn->move( 390, 10 );
// m_pActivateBtn->resize( 100, 24 );
    m_pActivateBtn = ToggleButton::create(this);
    m_pActivateBtn->setText(trUtf8("M"));
    m_pActivateBtn->setToolTip(trUtf8("Mute"));
    connect( m_pActivateBtn, SIGNAL(clicked()), this, SLOT(activateBtnClicked()) );

    m_pRemoveBtn = Button::create(this);
    m_pRemoveBtn->setText(trUtf8("X"));
    m_pRemoveBtn->setToolTip(trUtf8("Remove"));
    connect(m_pRemoveBtn, SIGNAL(clicked()), this, SLOT(removeBtnClicked()));

    m_pTimer = new QTimer( this );
    connect(m_pTimer, SIGNAL( timeout() ), this, SLOT( updateOutputControls() ) );
}


LadspaFXProperties::~LadspaFXProperties()
{
// qDebug() << "DESTROY";
}



void LadspaFXProperties::showEvent ( QShowEvent* )
{
    updateControls();
}



void LadspaFXProperties::closeEvent( QCloseEvent *ev )
{
    ev->accept();
}

void LadspaFXProperties::toggleChanged(ToggleButton* ref)
{
#ifdef LADSPA_SUPPORT
    for ( int i = 0; i < m_pInputControlFaders.size(); i++ ) {
        if (ref == m_pInputControlFaders[ i ] ) {
            LadspaControlPort *pControl = m_nLadspaFX->fx->inputControlPorts[ i ];
            pControl->m_fControlValue = (int)ref->getValue();
        }
    }
#endif
}

void LadspaFXProperties::faderChanged( Fader * ref )
{
    ref->setPeak_L( ref->getValue() );
    ref->setPeak_R( ref->getValue() );

#ifdef LADSPA_SUPPORT
    for ( int i = 0; i < m_pInputControlFaders.size(); i++ ) {
        if (ref == m_pInputControlFaders[ i ] ) {
            LadspaControlPort *pControl = m_nLadspaFX->fx->inputControlPorts[ i ];

            pControl->m_fControlValue = ref->getLinDb() ? ref->getValue() : ref->getDbValue();
            //float fInterval = pControl->fUpperBound - pControl->fLowerBound;
            //pControl->fControlValue = pControl->fLowerBound + fValue * fInterval;

            QString sValue;
            if (pControl->m_fControlValue < 1.0 ) {
                sValue = QString("%1").arg( pControl->m_fControlValue, 0, 'f', 2 );
            } else if ( pControl->m_fControlValue < 100.0 ) {
                sValue = QString("%1").arg( pControl->m_fControlValue, 0, 'f', 1 );
            } else {
                sValue = QString("%1").arg( pControl->m_fControlValue, 0, 'f', 0 );
            }
            m_pInputControlLabel[ i ]->setText( sValue );
        }
    }
#endif
}



void LadspaFXProperties::updateControls()
{
#ifdef LADSPA_SUPPORT
// qDebug() << "*** [updateControls] ***";
    m_pTimer->stop();

// LadspaFX *pFX = Effects::getInstance()->getLadspaFX( m_nLadspaFX );

    // svuoto i vettori..
    if ( m_pInputControlNames.size() != 0 ) {
        for (int i = 0; i < m_pInputControlNames.size(); i++) {
            delete m_pInputControlNames[ i ];
        }
        m_pInputControlNames.clear();
    }
    if ( m_pInputControlLabel.size() != 0 ) {
        for (int i = 0; i < m_pInputControlLabel.size(); i++) {
            delete m_pInputControlLabel[ i ];
        }
        m_pInputControlLabel.clear();
    }
    if ( m_pInputControlFaders.size() != 0 ) {
        for (int i = 0; i < m_pInputControlFaders.size(); i++) {
            delete m_pInputControlFaders[ i ];
        }
        m_pInputControlFaders.clear();
    }

    if ( m_pOutputControlFaders.size() != 0 ) {
        for (int i = 0; i < m_pOutputControlFaders.size(); i++) {
            delete m_pOutputControlFaders[ i ];
        }
        m_pOutputControlFaders.clear();
    }
    if ( m_pOutputControlNames.size() != 0 ) {
        for (int i = 0; i < m_pOutputControlNames.size(); i++) {
            delete m_pOutputControlNames[ i ];
        }
        m_pOutputControlNames.clear();
    }

    if (m_nLadspaFX->fx) {
        QString sPluginName = m_nLadspaFX->fx->getPluginLabel();
        setWindowTitle( trUtf8( "[%1] LADSPA FX Properties" ).arg( sPluginName ) );

        int nControlsFrameWidth = 40 + 45 * (m_nLadspaFX->fx->inputControlPorts.size() + m_nLadspaFX->fx->outputControlPorts.size());
        if (m_nLadspaFX->fx->inputControlPorts.size() > 0 && m_nLadspaFX->fx->outputControlPorts.size() > 0) {
            nControlsFrameWidth += 10;
        }
        if ( nControlsFrameWidth < 100 ) {
            nControlsFrameWidth = 100;
        }
//  m_pFrame->resize( nControlsFrameWidth, height() );
//qDebug() << nControlsFrameWidth;
        setFixedSize( nControlsFrameWidth, height() );
        m_pActivateBtn->move(nControlsFrameWidth - 51, 6 );
        m_pRemoveBtn->move(nControlsFrameWidth - 27, 6 );
        m_pNameLbl->resize(nControlsFrameWidth - 17, 24 );

//  m_pActivateBtn->setEnabled(true);
        if (m_nLadspaFX->fx->isEnabled()) {
            m_pActivateBtn->setToolTip( trUtf8("Deactivate") );
            m_pActivateBtn->setPressed(false);
        } else {
            m_pActivateBtn->setToolTip( trUtf8("Activate") );
            m_pActivateBtn->setPressed(true);
        }

        /*  QString mixerline_text_path = QCoreApplication::applicationDirPath() + "/data/mixerPanel/mixer_background.png";
          QPixmap textBackground;
          if( textBackground.load( mixerline_text_path ) == false ){
           qDebug() << "Error loading pixmap";
          }*/

        m_pNameLbl->setText( m_nLadspaFX->fx->getPluginName() );
        m_pNameLbl->setToolTip( m_nLadspaFX->fx->getPluginName() );

        // input controls
        uint nInputControl_X = 10;
        for (int i = 0; i < m_nLadspaFX->fx->inputControlPorts.size(); i++) {
            LadspaControlPort *pControlPort = m_nLadspaFX->fx->inputControlPorts[ i ];

            nInputControl_X = 10 + 45 * i;

            // peak volume label
            QString sValue;
            if (pControlPort->m_fControlValue < 1.0 ) {
                sValue = QString("%1").arg( pControlPort->m_fControlValue, 0, 'f', 2);
            } else if ( pControlPort->m_fControlValue < 100.0 ) {
                sValue = QString("%1").arg( pControlPort->m_fControlValue, 0, 'f', 1);
            } else {
                sValue = QString("%1").arg( pControlPort->m_fControlValue, 0, 'f', 0);
            }

            LCDDisplay *pLCD = new LCDDisplay( m_pFrame, LCDDigit::SMALL_BLUE, 4 );
            pLCD->move( nInputControl_X + 10, 40 );
            pLCD->setText( sValue );
            pLCD->show();
            QPalette lcdPalette;
            lcdPalette.setColor( QPalette::Background, QColor( 58, 62, 72 ) );
            pLCD->setPalette( lcdPalette );

            m_pInputControlLabel.push_back( pLCD );

            FaderName *pName = new FaderName( m_pFrame );
            pName->setFixedHeight(174);
            pName->resize(pName->width(), 174);
            pName->move( nInputControl_X + 3, 60 );
            pName->show();
            pName->setText( pControlPort->m_sName );
            m_pInputControlNames << pName;
            pName->setToolTip( pName->text() );

            if (pControlPort->m_bToggle) {
                pLCD->hide();

                ToggleButton *pToggle = ToggleButton::create(m_pFrame);
                connect( pToggle, SIGNAL( valueChanged(ToggleButton*) ), this, SLOT( toggleChanged(ToggleButton*) ) );
                m_pInputControlFaders.push_back( pToggle );
                pToggle->move( nInputControl_X + 20, 60 + 174 - 16 );
                pToggle->show();
                pToggle->setValue( (bool)pControlPort->m_fControlValue );

                toggleChanged( pToggle );
            } else {
                // fader
                Fader *pFader = new Fader( m_pFrame, pControlPort->m_bInteger, false, NULL, Backend::IN, !pControlPort->m_bLogarithmic );
                pFader->setFixedHeight(174);
                pFader->resize(pFader->width(), 174);
                connect( pFader, SIGNAL( valueChanged(Fader*) ), this, SLOT( faderChanged(Fader*) ) );
                m_pInputControlFaders.push_back( pFader );
                pFader->move( nInputControl_X + 20, 60 );
                pFader->show();
                if (pControlPort->m_bLogarithmic) {
                    pFader->setDbMaxValue( pControlPort->m_fUpperBound );
                    pFader->setDbMinValue( pControlPort->m_fLowerBound );
                    pFader->setDbMaxPeak( pControlPort->m_fUpperBound );
                    pFader->setDbMinPeak( pControlPort->m_fLowerBound );
                    pFader->setDbValue( pControlPort->m_fControlValue );
                    pFader->setDbPeak_L( pControlPort->m_fControlValue );
                    pFader->setDbPeak_R( pControlPort->m_fControlValue );
                } else {
                    pFader->setMaxValue( pControlPort->m_fUpperBound );
                    pFader->setMinValue( pControlPort->m_fLowerBound );
                    pFader->setMaxPeak( pControlPort->m_fUpperBound );
                    pFader->setMinPeak( pControlPort->m_fLowerBound );
                    pFader->setValue( pControlPort->m_fControlValue );
                    pFader->setPeak_L( pControlPort->m_fControlValue );
                    pFader->setPeak_R( pControlPort->m_fControlValue );
                }

                //float fInterval = pControlPort->m_fUpperBound - pControlPort->m_fLowerBound;
                //float fValue = ( pControlPort->m_fControlValue - pControlPort->m_fLowerBound ) / fInterval;
                //pFader->setValue( fValue );
                //pFader->setPeak_L( fValue );
                //pFader->setPeak_R( fValue );

                faderChanged( pFader );
            }
        }

        if (m_nLadspaFX->fx->inputControlPorts.size() > 0) {
            nInputControl_X += 55;
        }

        for (int i = 0; i < m_nLadspaFX->fx->outputControlPorts.size(); i++) {
            LadspaControlPort *pControl = m_nLadspaFX->fx->outputControlPorts[ i ];
            uint xPos = nInputControl_X + 10 + 45 * i;

            LCDDisplay *pLCD = new LCDDisplay( m_pFrame, LCDDigit::SMALL_BLUE, 4 );
            pLCD->move( xPos + 10, 40 );
//            pLCD->setText( sValue );
            pLCD->show();
            QPalette lcdPalette;
            lcdPalette.setColor( QPalette::Background, QColor( 58, 62, 72 ) );
            pLCD->setPalette( lcdPalette );
            m_pOutputControlLabel.push_back( pLCD );

            FaderName *pName = new FaderName( m_pFrame );
            pName->setFixedHeight(174);
            pName->resize(pName->width(), 174);
            pName->move( xPos + 3, 60 );
            pName->show();
            pName->setText( pControl->m_sName );
            m_pOutputControlNames.push_back( pName );
            pName->setToolTip( pName->text() );

            // fader
            Fader *pFader = new Fader( m_pFrame, false, true ); // without knob!
            pFader->setFixedHeight(174);
            pFader->resize(pFader->width(), 174);
            pFader->move( xPos + 20, 60 );
            //float fInterval = pControl->m_fUpperBound - pControl->m_fLowerBound;
            //float fValue = pControl->m_fControlValue / fInterval;
            pFader->show();
            pFader->setMaxValue( pControl->m_fUpperBound );
            pFader->setMinValue( pControl->m_fLowerBound );
            pFader->setMaxPeak( pControl->m_fUpperBound );
            pFader->setMinPeak( pControl->m_fLowerBound );
            pFader->setValue( pControl->m_fControlValue );
            pFader->setPeak_L( pControl->m_fControlValue );
            pFader->setPeak_R( pControl->m_fControlValue );

            m_pOutputControlFaders.push_back( pFader );
        }
    } else {
        qDebug() << "NULL PLUGIN";
//  setWindowTitle( trUtf8( "LADSPA FX %1 Properties" ).arg( m_nLadspaFX ) );
        m_pNameLbl->setText( trUtf8("No plugin") );
        m_pActivateBtn->setEnabled(false);
    }

    m_pTimer->start(100);
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
                if (fxInfo->m_sName == sSelectedFX ) {
                    return LadspaFX::load( fxInfo->m_sFilename, fxInfo->m_sLabel, Backend::instance()->getSampleRate() );
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
        if (m_nLadspaFX->fx->isEnabled()) {
            m_pActivateBtn->setToolTip( trUtf8("Deactivate") );
            m_pActivateBtn->setPressed(false);
        } else {
            m_pActivateBtn->setToolTip( trUtf8("Activate") );
            m_pActivateBtn->setPressed(true);
        }

        for (int i = 0; i < m_nLadspaFX->fx->outputControlPorts.size(); i++) {
            LadspaControlPort *pControl = m_nLadspaFX->fx->outputControlPorts[i];

            {
                QList<Fader*>::iterator it = m_pOutputControlFaders.begin() + i;
                if (it != m_pOutputControlFaders.end() ) {
                    Fader *pFader = *it;
                    if (pFader == NULL) {
                        qDebug() << "[updateOutputControls] pFader = NULL";
                        continue;
                    }

                    //    float fValue = pControl->m_fControlValue;
                    //    float fInterval = pControl->m_fUpperBound - pControl->m_fLowerBound;
                    float fValue = pControl->m_fControlValue;// / fInterval;
                    //    if (fValue < 0) fValue = -fValue;

                    pFader->setPeak_L( fValue );
                    pFader->setPeak_R( fValue );
                }
            }
            {
                QList<LCDDisplay*>::iterator it = m_pOutputControlLabel.begin() + i;
                if (it != m_pOutputControlLabel.end() ) {
                    LCDDisplay *pLabel = *it;
                    if (pLabel == NULL) {
                        qDebug() << "[updateOutputControls] pLabel = NULL";
                        continue;
                    }

                    QString sValue;
                    if (pControl->m_fControlValue < 1.0 ) {
                        sValue = QString("%1").arg( pControl->m_fControlValue, 0, 'f', 2 );
                    } else if ( pControl->m_fControlValue < 100.0 ) {
                        sValue = QString("%1").arg( pControl->m_fControlValue, 0, 'f', 1 );
                    } else {
                        sValue = QString("%1").arg( pControl->m_fControlValue, 0, 'f', 0 );
                    }
                    pLabel->setText( sValue );
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
        m_nLadspaFX->fx->setEnabled( !m_nLadspaFX->fx->isEnabled() );
    }
#endif
}

}
; //JackMix