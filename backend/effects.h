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

#ifndef EFFECTS_H
#define EFFECTS_H

#ifdef LADSPA_SUPPORT

#include <ladspa_fx.h>
#include <QList>

namespace LiveMix
{

/**
 *
 */
class Effects
{
public:
    static Effects* getInstance();
    ~Effects();

// LadspaFX* getLadspaFX( int nFX );
// void  setLadspaFX( LadspaFX* pFX, int nFX );

    QList<LadspaFXInfo*> getPluginList();
    LadspaFXGroup* getLadspaFXGroup();


private:
    static Effects* m_pInstance;
    QList<LadspaFXInfo*> m_pluginList;
    LadspaFXGroup* m_pRootGroup;

// QList<LadspaFX*> m_FXList;

    QList<QString> m_ladspaPathVect;

    Effects();

    void RDFDescend(const QString& sBase, LadspaFXGroup *pGroup, QList<LadspaFXInfo*> pluginList);
    void getRDF(LadspaFXGroup *pGroup, QList<LadspaFXInfo*> pluginList);

};

}
; //LiveMix

#endif

#endif
