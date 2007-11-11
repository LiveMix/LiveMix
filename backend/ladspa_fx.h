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

#ifndef LADSPA_FX_H
#define LADSPA_FX_H

#ifdef LADSPA_SUPPORT

#include <QtCore/QLibrary>
#include "ladspa.h"

namespace LiveMix
{

class LadspaFXInfo
{
public:
    LadspaFXInfo( const QString& sName );
    ~LadspaFXInfo();

    QString m_sFilename; ///< plugin filename
    int m_sID;
    QString m_sLabel;
    QString m_sName;
    QString m_sMaker;
    QString m_sCopyright;
    unsigned m_nICPorts; ///< input control port
    unsigned m_nOCPorts; ///< output control port
    unsigned m_nIAPorts; ///< input audio port
    unsigned m_nOAPorts; ///< output audio port
};


class LadspaFXGroup
{
public:
    LadspaFXGroup( const QString& sName );
    ~LadspaFXGroup();

    const QString& getName()
    {
        return m_sName;
    }

    void addLadspaInfo( LadspaFXInfo *pInfo );
    QList<LadspaFXInfo*> getLadspaInfo()
    {
        return m_ladspaList;
    }

    void addChild( LadspaFXGroup *pChild );
    QList<LadspaFXGroup*> getChildList()
    {
        return m_childGroups;
    }

private:
    QString m_sName;
    QList<LadspaFXInfo*> m_ladspaList;
    QList<LadspaFXGroup*> m_childGroups;
};



class LadspaControlPort
{
public:
    QString m_sName;
    bool m_bToggle;
    bool m_bLogarithmic;
    bool m_bInteger;
    LADSPA_Data m_fDefaultControlValue;
    LADSPA_Data m_fControlValue;
    LADSPA_Data m_fLowerBound;
    LADSPA_Data m_fUpperBound;

    LadspaControlPort()
    { }
};



class LadspaFX
{
public:
    float* m_pInBufferL;
    float* m_pInBufferR;
    float* m_pOutBufferL;
    float* m_pOutBufferR;

    QList<LadspaControlPort*> inputControlPorts;
    QList<LadspaControlPort*> outputControlPorts;

    ~LadspaFX();

    void connectAudioPorts();
    void activate();
    void deactivate();
    void processFX(unsigned nFrames, bool stereo);


    const QString& getPluginLabel()
    {
        return m_sLabel;
    }

    const QString& getPluginName()
    {
        return m_sName;
    }
    void setPluginName( const QString& sName )
    {
        m_sName = sName;
    }

    const QString& getLibraryPath()
    {
        return m_sLibraryPath;
    }

    bool isEnabled()
    {
        return m_bEnabled;
    }
    void setEnabled( bool value )
    {
        m_bEnabled = value;
    }

    unsigned getInputAudio()
    {
        return m_nIAPorts;
    };
    unsigned getOutputAudio()
    {
        return m_nOAPorts;
    };

    static LadspaFX* load( const QString& sLibraryPath,  const QString& sPluginLabel, long nSampleRate );


private:
    bool m_bEnabled;
    QString m_sLabel;
    QString m_sName;
    QString m_sLibraryPath;

    QLibrary *m_pLibrary;

    const LADSPA_Descriptor* m_d;
    LADSPA_Handle m_handle;
    LADSPA_Handle m_handleBis;
// float m_fVolume;

    unsigned m_nICPorts; ///< input control port
    unsigned m_nOCPorts; ///< output control port
    unsigned m_nIAPorts; ///< input audio port
    unsigned m_nOAPorts; ///< output audio port


    LadspaFX( const QString& sLibraryPath, const QString& sPluginLabel );
    void connectAudioPorts(float* m_pInBufferL, float* m_pInBufferR, float* m_pOutBufferL, float* m_pOutBufferR, LADSPA_Handle p_handle);
};

}
; //LiveMix

#endif // LADSPA_SUPPORT

#endif
