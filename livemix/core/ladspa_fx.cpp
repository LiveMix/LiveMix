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

#ifdef LADSPA_SUPPORT

#include "LadspaFX.h"
#include "backend.h"
#include "db.h"

#include <ladspa.h>

#include <QtCore/QDir>
#include <QtCore/QDebug>

#define LADSPA_IS_CONTROL_INPUT(x) (LADSPA_IS_PORT_INPUT(x) && LADSPA_IS_PORT_CONTROL(x))
#define LADSPA_IS_AUDIO_INPUT(x) (LADSPA_IS_PORT_INPUT(x) && LADSPA_IS_PORT_AUDIO(x))
#define LADSPA_IS_CONTROL_OUTPUT(x) (LADSPA_IS_PORT_OUTPUT(x) && LADSPA_IS_PORT_CONTROL(x))
#define LADSPA_IS_AUDIO_OUTPUT(x) (LADSPA_IS_PORT_OUTPUT(x) && LADSPA_IS_PORT_AUDIO(x))

namespace JackMix
{

LadspaFXGroup::LadspaFXGroup( const QString& sName )
{
// qDebug() << "INIT - " + sName );
    m_sName = sName;
}


LadspaFXGroup::~LadspaFXGroup()
{
// qDebug() << "DESTROY - " + m_sName );

    for ( int i = 0; i < (int)m_childGroups.size(); ++i ) {
        delete m_childGroups[ i ];
    }
}



void LadspaFXGroup::addLadspaInfo(LadspaFXInfo *pInfo)
{
    m_ladspaList.push_back( pInfo );
}


void LadspaFXGroup::addChild( LadspaFXGroup *pChild )
{
    m_childGroups.push_back( pChild );
}



////////////////


LadspaFXInfo::LadspaFXInfo( const QString& sName )
{
// qDebug() << "INIT - " + sName;
    m_sFilename = "";
    m_sLabel = "";
    m_sName = sName;
    m_nICPorts = 0;
    m_nOCPorts = 0;
    m_nIAPorts = 0;
    m_nOAPorts = 0;
}


LadspaFXInfo::~LadspaFXInfo()
{
// qDebug() << "DESTROY " + m_sName );
}


///////////////////



// ctor
LadspaFX::LadspaFX( const QString& sLibraryPath, const QString& sPluginLabel )
//, m_nBufferSize( 0 )
        :
        m_pInBufferL(NULL)
        , m_pInBufferR(NULL)
        , m_pOutBufferL(NULL)
        , m_pOutBufferR(NULL)
        , m_bEnabled( false )
        , m_sLabel( sPluginLabel )
        , m_sLibraryPath( sLibraryPath )
        , m_pLibrary( NULL )
        , m_d( NULL )
        , m_handle( NULL )
        , m_handleBis( NULL )
        , m_nICPorts( 0 )
        , m_nOCPorts( 0 )
        , m_nIAPorts( 0 )
        , m_nOAPorts( 0 )
{
// qDebug() << "INIT - " + sLibraryPath + " - " + sPluginLabel;
}


// dtor
LadspaFX::~LadspaFX()
{
    // dealloca il plugin
// qDebug() << "DESTROY - " + m_sLibraryPath + " - " + m_sLabel;

    if (m_d) {
        /*  if (m_d->deactivate) {
        //    qDebug() << "deactivate";
           if ( m_handle ) {
            m_d->deactivate( m_handle );
           }
           if ( m_handleBis ) {
            m_d->deactivate( m_handleBis );
           }
          }*/

        if (m_d->cleanup) {
//    qDebug() << "Cleanup";
            if ( m_handle ) {
                m_d->cleanup( m_handle );
            }
            if ( m_handleBis ) {
                m_d->cleanup( m_handleBis );
            }
        }
    }
    delete m_pLibrary;

    for (int i = 0; i < inputControlPorts.size(); i++) {
        delete inputControlPorts[i];
    }
    for (int i = 0; i < outputControlPorts.size(); i++) {
        delete outputControlPorts[i];
    }
}




// Static
LadspaFX* LadspaFX::load( const QString& sLibraryPath, const QString& sPluginLabel, long nSampleRate )
{
    LadspaFX* pFX = new LadspaFX( sLibraryPath, sPluginLabel);

// qDebug() << "INIT - " + sLibraryPath + " - " + sPluginLabel;

    pFX->m_pLibrary = new QLibrary( sLibraryPath );
    LADSPA_Descriptor_Function desc_func = (LADSPA_Descriptor_Function)pFX->m_pLibrary->resolve( "ladspa_descriptor" );
    if ( desc_func == NULL ) {
        qDebug() << "Error loading the library. (" + sLibraryPath + ")";
        delete pFX;
        return NULL;
    }
    if ( desc_func ) {
        for ( unsigned i = 0; ( pFX->m_d = desc_func( i ) ) != NULL; i++ ) {
            QString sName = pFX->m_d->Name;
            QString sLabel = pFX->m_d->Label;

            if (sLabel != sPluginLabel) {
                continue;
            }
            pFX->setPluginName( sName );

            for (unsigned j = 0; j < pFX->m_d->PortCount; j++) {
                LADSPA_PortDescriptor pd = pFX->m_d->PortDescriptors[j];
                if ( LADSPA_IS_PORT_INPUT( pd ) && LADSPA_IS_PORT_CONTROL( pd ) ) {
                    pFX->m_nICPorts++;
                } else if ( LADSPA_IS_PORT_INPUT( pd ) && LADSPA_IS_PORT_AUDIO( pd ) ) {
                    pFX->m_nIAPorts++;
                } else if ( LADSPA_IS_PORT_OUTPUT( pd ) && LADSPA_IS_PORT_CONTROL( pd ) ) {
                    pFX->m_nOCPorts++;
                } else if ( LADSPA_IS_PORT_OUTPUT( pd ) && LADSPA_IS_PORT_AUDIO( pd ) ) {
                    pFX->m_nOAPorts++;
                } else {
                    qDebug() << "Unknown port type";
                }
            }
            break;
        }
    } else {
        qDebug() << "Error in dlsym";
        delete pFX;
        return NULL;
    }

    /* if ( ( pFX->m_nIAPorts == 2 ) && ( pFX->m_nOAPorts == 2 ) ) {  // Stereo plugin
      pFX->m_pluginType = STEREO_FX;
     }
     else if ( ( pFX->m_nIAPorts == 1 ) && ( pFX->m_nOAPorts == 1 ) ) { // Mono plugin
      pFX->m_pluginType = MONO_FX;
     }*/
    if (pFX->m_nIAPorts <= 2 || pFX->m_nOAPorts <= 2) {}
    else {
        qDebug() << "Wrong number of ports";
        qDebug() << "in audio = " + pFX->m_nIAPorts;
        qDebug() << "out audio = " + pFX->m_nOAPorts;
    }

    //pFX->qDebug() << "[LadspaFX::load] instantiate " + pFX->getPluginName() );
    pFX->m_handle = pFX->m_d->instantiate( pFX->m_d, nSampleRate);
    if (pFX->m_nIAPorts == 1 && pFX->m_nOAPorts == 1) {
        pFX->m_handleBis = pFX->m_d->instantiate( pFX->m_d, nSampleRate);
    }

    for ( unsigned nPort = 0; nPort < pFX->m_d->PortCount; nPort++) {
        LADSPA_PortDescriptor pd = pFX->m_d->PortDescriptors[ nPort ];

        if ( LADSPA_IS_CONTROL_INPUT( pd ) ) {
            QString sName = pFX->m_d->PortNames[ nPort ];
            float fMin = 0.0;
            float fMax = 0.0;
            float fDefault = 0.0;
            bool bToggle = false;
            bool bInteger = false;
            bool bLogarithmic = false;

            LADSPA_PortRangeHint rangeHints = pFX->m_d->PortRangeHints[ nPort ];
            if ( LADSPA_IS_HINT_BOUNDED_BELOW( rangeHints.HintDescriptor ) ) {
                fMin = ( pFX->m_d->PortRangeHints[ nPort ] ).LowerBound;
            }
            if ( LADSPA_IS_HINT_BOUNDED_ABOVE( rangeHints.HintDescriptor ) ) {
                fMax = ( pFX->m_d->PortRangeHints[ nPort ] ).UpperBound;
            }
            if ( LADSPA_IS_HINT_TOGGLED( rangeHints.HintDescriptor ) ) {
                bToggle = true;

                // this way the fader will act like a toggle (0, 1)
                bInteger = true;
                fMin = 0.0;
                fMax = 1.0;
            }
            if ( LADSPA_IS_HINT_SAMPLE_RATE( rangeHints.HintDescriptor ) ) {
//    qDebug() << "samplerate hint not implemented yet";
                fMin *= Backend::instance()->getSampleRate();
                fMax *= Backend::instance()->getSampleRate();
            }
            if ( LADSPA_IS_HINT_LOGARITHMIC( rangeHints.HintDescriptor ) ) {
//                qDebug() <<  "logarithmic hint not implemented yet";
                bLogarithmic = fMin > 0 && fMax > 0; // 0 can't be logaritmique
            }
            if ( LADSPA_IS_HINT_INTEGER( rangeHints.HintDescriptor ) ) {
                bInteger = true;
            }
            if ( LADSPA_IS_HINT_HAS_DEFAULT( rangeHints.HintDescriptor ) ) {
                if ( LADSPA_IS_HINT_DEFAULT_MINIMUM( rangeHints.HintDescriptor ) ) {
                    fDefault = fMin;
                }
                if ( LADSPA_IS_HINT_DEFAULT_LOW( rangeHints.HintDescriptor ) ) {
                    if (bLogarithmic) {
                        fDefault = db2lin(lin2db(fMin) * 0.75 + lin2db(fMax) * 0.25);
                    } else {
                        fDefault = fMin * 0.75 + fMax * 0.25;
                    }
                }
                if ( LADSPA_IS_HINT_DEFAULT_MIDDLE( rangeHints.HintDescriptor ) ) {
                    if (bLogarithmic) {
                        fDefault = db2lin((lin2db(fMin) + lin2db(fMax)) * 0.5);
                    } else {
                        fDefault = (fMax + fMin) * 0.5;
                    }
                }
                if ( LADSPA_IS_HINT_DEFAULT_HIGH( rangeHints.HintDescriptor ) ) {
                    if (bLogarithmic) {
                        fDefault = db2lin(lin2db(fMin) * 0.25 + lin2db(fMax) * 0.75);

                    } else {
                        fDefault = fMin * 0.25 + fMax * 0.75;
                    }
                }
                if ( LADSPA_IS_HINT_DEFAULT_MAXIMUM( rangeHints.HintDescriptor ) ) {
                    fDefault = fMax;
                }
                if ( LADSPA_IS_HINT_DEFAULT_0( rangeHints.HintDescriptor ) ) {
                    fDefault = 0.0;
                }
                if ( LADSPA_IS_HINT_DEFAULT_1( rangeHints.HintDescriptor ) ) {
                    fDefault = 1.0;
                }
                if ( LADSPA_IS_HINT_DEFAULT_100( rangeHints.HintDescriptor ) ) {
                    fDefault = 100.0;
                }
                if ( LADSPA_IS_HINT_DEFAULT_440( rangeHints.HintDescriptor ) ) {
                    fDefault = 440.0;
                }
            }

            /*qDebug()<<sName<<LADSPA_IS_HINT_BOUNDED_BELOW( rangeHints.HintDescriptor )<<LADSPA_IS_HINT_BOUNDED_ABOVE( rangeHints.HintDescriptor )
            <<111<<LADSPA_IS_HINT_TOGGLED( rangeHints.HintDescriptor )<<LADSPA_IS_HINT_SAMPLE_RATE( rangeHints.HintDescriptor )
            <<222<<LADSPA_IS_HINT_LOGARITHMIC( rangeHints.HintDescriptor )<<LADSPA_IS_HINT_INTEGER( rangeHints.HintDescriptor )
            <<333<<LADSPA_IS_HINT_HAS_DEFAULT( rangeHints.HintDescriptor )<<LADSPA_IS_HINT_DEFAULT_MINIMUM( rangeHints.HintDescriptor )
            <<444<<LADSPA_IS_HINT_DEFAULT_LOW( rangeHints.HintDescriptor )<<LADSPA_IS_HINT_DEFAULT_MIDDLE( rangeHints.HintDescriptor )
            <<555<<LADSPA_IS_HINT_DEFAULT_HIGH( rangeHints.HintDescriptor )<<LADSPA_IS_HINT_DEFAULT_MAXIMUM( rangeHints.HintDescriptor )
            <<666<<LADSPA_IS_HINT_DEFAULT_0( rangeHints.HintDescriptor )<<LADSPA_IS_HINT_DEFAULT_1( rangeHints.HintDescriptor )
            <<777<<LADSPA_IS_HINT_DEFAULT_100( rangeHints.HintDescriptor )<<LADSPA_IS_HINT_DEFAULT_440( rangeHints.HintDescriptor );
             
            qDebug()<<sName
            <<QString("%1").arg((pFX->m_d->PortRangeHints[ nPort ] ).LowerBound)
            <<QString("%1").arg(( pFX->m_d->PortRangeHints[ nPort ] ).UpperBound)
            <<QString("%1").arg(fMin)<<QString("%1").arg(fMax)<<QString("%1").arg(fDefault);
            qDebug();*/


            LadspaControlPort* pControl = new LadspaControlPort();
            pControl->m_sName = sName;
            pControl->m_fLowerBound = fMin;
            pControl->m_fUpperBound = fMax;
            pControl->m_fControlValue = fDefault;
            pControl->m_bToggle = bToggle;
            pControl->m_bLogarithmic = bLogarithmic;
            pControl->m_bInteger = bInteger;

//   qDebug() << "Input control port\t[" + sName + "]\tmin=" + QString().setNum(fMin) + ",\tmax=" + QString().setNum(fMax) +
//     ",\tcontrolValue=" + QString().setNum(pControl->fControlValue);

            pFX->inputControlPorts.push_back( pControl );
            pFX->m_d->connect_port( pFX->m_handle, nPort, &(pControl->m_fControlValue) );
            if (pFX->m_handleBis) {
                pFX->m_d->connect_port(pFX->m_handleBis, nPort, &(pControl->m_fControlValue) );
            }
        } else if ( LADSPA_IS_CONTROL_OUTPUT( pd ) ) {
            QString sName = pFX->m_d->PortNames[ nPort ];
            float fMin = 0.0;
            float fMax = 0.0;
            float fDefault = 0.0;

            LADSPA_PortRangeHint rangeHints = pFX->m_d->PortRangeHints[ nPort ];
            if ( LADSPA_IS_HINT_BOUNDED_BELOW( rangeHints.HintDescriptor ) ) {
                fMin = ( pFX->m_d->PortRangeHints[ nPort ] ).LowerBound;
            }
            if ( LADSPA_IS_HINT_BOUNDED_ABOVE( rangeHints.HintDescriptor ) ) {
                fMax = ( pFX->m_d->PortRangeHints[ nPort ] ).UpperBound;
            }

            /*   LadspaControlPort* pControl = new LadspaControlPort();
               pControl->sName = pFX->m_d->PortNames[ nPort ];
               pControl->fLowerBound = ( pFX->m_d->PortRangeHints[ nPort ] ).LowerBound;
               pControl->fUpperBound = ( pFX->m_d->PortRangeHints[ nPort ] ).UpperBound;
               pControl->fControlValue = pControl->fUpperBound / 2.0;
            */
            // always middle
//   fDefault = (fMax - fMin) / 2.0 + fMin;
            // always min
            fDefault = fMin;

            LadspaControlPort* pControl = new LadspaControlPort();
            pControl->m_sName = sName;
            pControl->m_fLowerBound = fMin;
            pControl->m_fUpperBound = fMax;
            pControl->m_fControlValue = fDefault;
            //pFX->qDebug() << "[LadspaFX::load] Output control port\t[" + sName + "]\tmin=" + fMin) + ",\tmax=" + fMax) + ",\tcontrolValue=" + pControl->fControlValue) );

            pFX->outputControlPorts.push_back( pControl );
            pFX->m_d->connect_port( pFX->m_handle, nPort, &(pControl->m_fControlValue) );
            if (pFX->m_handleBis) {
                pFX->m_d->connect_port(pFX->m_handleBis, nPort, &(pControl->m_fControlValue));
            }
        } else if ( LADSPA_IS_AUDIO_INPUT( pd ) ) {}
        else if ( LADSPA_IS_AUDIO_OUTPUT( pd ) ) {}
        else {
            qDebug() << "unknown port";
        }
    }

    return pFX;
}



void LadspaFX::connectAudioPorts()
{
    if (m_nIAPorts == 1 && m_nOAPorts == 1) {
        connectAudioPorts(m_pInBufferL, NULL, m_pOutBufferL, NULL, m_handle);
        connectAudioPorts(m_pInBufferR, NULL, m_pOutBufferR, NULL, m_handleBis);
    } else {
        connectAudioPorts(m_pInBufferL, m_pInBufferR, m_pOutBufferL, m_pOutBufferR, m_handle);
    }
}
void LadspaFX::connectAudioPorts(float* p_pInL, float* p_pInR, float* p_pOutL, float* p_pOutR, LADSPA_Handle p_handle)
{
// qDebug() << "[connectAudioPorts]";

    unsigned nAIConn = 0;
    unsigned nAOConn = 0;
    for ( unsigned nPort = 0; nPort < m_d->PortCount; nPort++) {
        LADSPA_PortDescriptor pd = m_d->PortDescriptors[ nPort ];
        if ( LADSPA_IS_CONTROL_INPUT( pd ) ) {}
        else if ( LADSPA_IS_CONTROL_OUTPUT( pd ) ) {}
        else if ( LADSPA_IS_AUDIO_INPUT( pd ) ) {
            if (nAIConn == 0) {
                m_d->connect_port( p_handle, nPort, p_pInL );
                //qDebug() << "connect input port (L): " + string( m_d->PortNames[ nPort ] ) );
            } else if (nAIConn == 1) {
                m_d->connect_port( p_handle, nPort, p_pInR );
                //qDebug() << "connect input port (R): " + string( m_d->PortNames[ nPort ] ) );
            } else {
                qDebug() << "too many input ports..";
            }
            nAIConn++;
        } else if ( LADSPA_IS_AUDIO_OUTPUT( pd ) ) {
            if (nAOConn == 0) {
                m_d->connect_port( p_handle, nPort, p_pOutL );
                //qDebug() << "connect output port (L): " + string( m_d->PortNames[ nPort ] ) );
            } else if (nAOConn == 1) {
                m_d->connect_port( p_handle, nPort, p_pOutR );
                //qDebug() << "connect output port (R): " + string( m_d->PortNames[ nPort ] ) );
            } else {
                qDebug() << "too many output ports..";
            }
            nAOConn++;
        } else {
            qDebug() << "unknown port";
        }
    }
}



void LadspaFX::processFX( unsigned nFrames )
{
    m_d->run( m_handle, nFrames );
    if (m_nIAPorts == 1 && m_nOAPorts == 1) {
        m_d->run( m_handleBis, nFrames );
    }
//qDebug() << QString("processFX - Label: %1, nb in: %2, nb out: %3, in l: %4, in r: %5, out l: %6, out r: %7")
//  .arg(m_sLabel).arg(m_nIAPorts).arg(m_nOAPorts).arg(m_pInBufferL[0]).arg(m_pInBufferR[0]).arg(m_pOutBufferL[0]).arg(m_pOutBufferR[0]);
}

void LadspaFX::activate()
{
    if ( m_d->activate ) {
//        qDebug() << "activate " + getPluginName();
        m_d->activate( m_handle );
        if (m_nIAPorts == 1 && m_nOAPorts == 1) {
            m_d->activate( m_handleBis );
        }
    }
}


void LadspaFX::deactivate()
{
    if ( m_d->deactivate ) {
//  qDebug() << "deactivate " + getPluginName();
        m_d->deactivate( m_handle );
        if (m_nIAPorts == 1 && m_nOAPorts == 1) {
            m_d->deactivate( m_handleBis );
        }
    }
}

}
; //JackMix

#endif // LADSPA_SUPPORT
