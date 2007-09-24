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

#include <effects.h>

#ifdef LADSPA_SUPPORT

#include <ladspa_fx.h>

#include <QDir>
#include <QLibrary>
#include <QDebug>
#include <QCharRef>

#ifdef LRDF_SUPPORT
#include <lrdf.h>
#endif

namespace LiveMix
{

// static data
Effects* Effects::m_pInstance = NULL;


Effects::Effects()
        : m_pRootGroup(NULL)
{
    //qDebug( "INIT" );

    char * ladpath = getenv("LADSPA_PATH"); // read the Environment variable LADSPA_PATH
    if (ladpath) {
        qDebug( "Found LADSPA_PATH enviroment variable" );
        std::string sLadspaPath = std::string(ladpath);
        int pos;
        while ( (pos = sLadspaPath.find(":")) != -1) {
            std::string sPath = sLadspaPath.substr(0, pos);
            m_ladspaPathVect << sLadspaPath.c_str();
            sLadspaPath = sLadspaPath.substr( pos + 1, sLadspaPath.length() );
        }
        m_ladspaPathVect << sLadspaPath.c_str();
    } else {
#ifdef Q_OS_MACX
        m_ladspaPathVect << qApp->applicationDirPath().toStdString() + "/../Resources/plugins";
        m_ladspaPathVect << "/Library/Audio/Plug-Ins/LADSPA/";
        m_ladspaPathVect << QDir::homePath().append("/Library/Audio/Plug-Ins/LADSPA");
#else
        m_ladspaPathVect << "/usr/lib/ladspa";
        m_ladspaPathVect << "/usr/local/lib/ladspa";
#endif

    }

    getPluginList();
}


Effects* Effects::getInstance()
{
    if ( m_pInstance == NULL ) {
        m_pInstance = new Effects();
    }
    return m_pInstance;
}


Effects::~Effects()
{
    //qDebug( "DESTROY" );
    delete getLadspaFXGroup();

    //qDebug( "destroying " + toString( m_pluginList.size() ) + " LADSPA plugins" );
    foreach(LadspaFXInfo* fxinfo, m_pluginList) {
        delete fxinfo;
    }
    m_pluginList.clear();

// foreach (LadspaFX* fx, m_FXList) {
//  delete fx;
// }
}

/*
LadspaFX* Effects::getLadspaFX( int nFX )
{
 return m_FXList[ nFX ];
}
 
 
void  Effects::setLadspaFX( LadspaFX* pFX, int nFX )
{
 //qDebug( "[setLadspaFX] FX: " + pFX->getPluginLabel() + ", " + toString( nFX ) );
 
 if ( m_FXList[ nFX ] ) {
  m_FXList[ nFX ]->deactivate();
  m_FXList.removeAt(nFX);
  delete m_FXList[ nFX ];
 }
 
 m_FXList[ nFX ] = pFX;
}
*/

///
/// Loads only usable plugins
///
QList<LadspaFXInfo*> Effects::getPluginList()
{
    if ( m_pluginList.size() != 0 ) {
        //Logger::getInstance()->log( "skipping" );
        return m_pluginList;
    }



    qDebug() << "PATHS: " + m_ladspaPathVect.size();
    foreach (QString sPluginDir, m_ladspaPathVect) {
        qDebug() << "*** [getPluginList] reading directory: " + sPluginDir;

        QDir dir(sPluginDir);
        if (!dir.exists()) {
            qDebug() << "Directory " + sPluginDir + " not found";
            continue;
        }

        QFileInfoList list = dir.entryInfoList();
        for ( int i = 0; i < list.size(); ++i ) {
            QString sPluginName = list.at( i ).fileName();

            if ( (sPluginName == ".") || (sPluginName == ".." ) ) {
                continue;
            }

            // if the file ends with .so or .dll is a plugin, else...
#ifdef WIN32
            bool pos = sPluginName.endsWith( ".dll" );
#else
#ifdef Q_OS_MACX
            bool pos = sPluginName.endsWith( ".dylib" );
#else
            bool pos = sPluginName.endsWith( ".so" );
#endif
#endif
            if ( !pos ) {
                continue;
            }
            //warningLog( "[getPluginList] Loading: " + sPluginName  );

            QString sAbsPath = sPluginDir + "/" + sPluginName;

            QLibrary lib( sAbsPath );
            LADSPA_Descriptor_Function desc_func = (LADSPA_Descriptor_Function)lib.resolve( "ladspa_descriptor" );
            if ( desc_func == NULL ) {
                qDebug() << "Error loading the library. (" + sAbsPath + ")";
                continue;
            }
            const LADSPA_Descriptor * d;
            if ( desc_func ) {
                for ( unsigned i = 0; (d = desc_func (i)) != NULL; i++) {
                    if (!LADSPA_IS_HARD_RT_CAPABLE(d->Properties)) {
                        qDebug() << QString("Plugin not real-time: %1, Label: %2").arg(sPluginName).arg(d->Label);
                        continue;
                    }


                    LadspaFXInfo* pFX = new LadspaFXInfo( d->Name );
                    pFX->m_sFilename = sAbsPath;
                    pFX->m_sLabel = d->Label;
                    pFX->m_sID = d->UniqueID;
                    pFX->m_sMaker = d->Maker;
                    pFX->m_sCopyright = d->Copyright;

                    //qDebug( "Loading: " + pFX->m_sLabel;

                    for (unsigned j = 0; j < d->PortCount; j++) {
                        LADSPA_PortDescriptor pd = d->PortDescriptors[j];
                        if ( LADSPA_IS_PORT_INPUT( pd ) && LADSPA_IS_PORT_CONTROL( pd ) ) {
                            pFX->m_nICPorts++;
                        } else if ( LADSPA_IS_PORT_INPUT( pd ) && LADSPA_IS_PORT_AUDIO( pd ) ) {
                            pFX->m_nIAPorts++;
                        } else if ( LADSPA_IS_PORT_OUTPUT( pd ) && LADSPA_IS_PORT_CONTROL( pd ) ) {
                            pFX->m_nOCPorts++;
                        } else if ( LADSPA_IS_PORT_OUTPUT( pd ) && LADSPA_IS_PORT_AUDIO( pd ) ) {
                            pFX->m_nOAPorts++;
                        } else {
//       QString sPortName = d->PortNames[ j ];
                            QString sPortName = "";
                            qDebug() << pFX->m_sLabel + "::" + sPortName + "  UNKNOWN port type";
                        }
                    }
//     if (pFX->m_nIAPorts >= 1 && pFX->m_nOAPorts >= 1) {
//                    if (pFX->m_nIAPorts <= 2 && pFX->m_nOAPorts <= 2) {
                        m_pluginList.push_back( pFX );
//                    }
                    /*     if ( ( pFX->m_nIAPorts == 2 ) && ( pFX->m_nOAPorts == 2 ) ) { // Stereo plugin
                          m_pluginList.push_back( pFX );
                         }
                         else if ( ( pFX->m_nIAPorts == 1 ) && ( pFX->m_nOAPorts == 1 ) ) { // Mono plugin
                          m_pluginList.push_back( pFX );
                         }*/
/*                    else { // not supported plugin
                        qDebug() << QString("Plugin not supported: %1, Label: %2, nb in: %3, nb out: %4")
                        .arg(sPluginName).arg(pFX->m_sLabel).arg(pFX->m_nIAPorts).arg(pFX->m_nOAPorts);
                        delete pFX;
                    }*/
                }
            } else {
                qDebug() << "Error loading: " + sPluginName ;
            }
        }
    }

    qDebug() << "Loaded " + QString(m_pluginList.size()) + "  LADSPA plugins";

    return m_pluginList;
}


LadspaFXGroup* Effects::getLadspaFXGroup()
{
//    qDebug() << "[getLadspaFXGroup]";

// LadspaFX::getPluginList(); // load the list

    if ( m_pRootGroup  ) {
        return m_pRootGroup;
    }

    m_pRootGroup = new LadspaFXGroup( "Root" );

    LadspaFXGroup *pUncategorizedGroup = new LadspaFXGroup(QObject::trUtf8("Uncategorized"));
    m_pRootGroup->addChild( pUncategorizedGroup );

    QMap<LadspaFXInfo*, QString> fxGroupMap;

    // build alphabetical list
    for (int i = 0; i < m_pluginList.size(); i++) {
        LadspaFXInfo *pInfo = m_pluginList[ i ];
        QCharRef ch = pInfo->m_sName[0];
        fxGroupMap[ pInfo ] = ch;
    }

    for (QMap<LadspaFXInfo*, QString>::iterator it = fxGroupMap.begin(); it != fxGroupMap.end(); it++) {
        QString sGroup = it.value();
        LadspaFXInfo *pInfo = it.key();

        LadspaFXGroup *pGroup = NULL;
        for (int i = 0; i < pUncategorizedGroup->getChildList().size(); i++) {
            LadspaFXGroup *pChild = ( pUncategorizedGroup->getChildList() )[ i ];
            if (pChild->getName() == sGroup) {
                pGroup = pChild;
                break;
            }
        }
        if (!pGroup) {
            pGroup = new LadspaFXGroup( sGroup );
            pUncategorizedGroup->addChild( pGroup );
        }
        pGroup->addLadspaInfo( pInfo );
    }

#ifdef LRDF_SUPPORT
    LadspaFXGroup *pLRDFGroup = new LadspaFXGroup( QObject::trUtf8("Categorized (LRDF)") );
    m_pRootGroup->addChild( pLRDFGroup );
    getRDF(pLRDFGroup, m_pluginList);
#endif

    return m_pRootGroup;
}


#ifdef LRDF_SUPPORT


void Effects::getRDF(LadspaFXGroup *pGroup, QList<LadspaFXInfo*> pluginList)
{
    lrdf_init();

    QString sDir = "/usr/share/ladspa/rdf";

    QDir dir( sDir );
    if (!dir.exists()) {
        qDebug() << "Directory " + sDir + " not found";
        return;
    }

    QFileInfoList list = dir.entryInfoList();
    for ( int i = 0; i < list.size(); ++i ) {
        QString sFilename = list.at( i ).fileName();
        if (!(sFilename.endsWith(".rdf") || sFilename.endsWith(".rdfs"))) {
            continue;
        }

        QString sRDFFile = "file://" + sDir + "/" + sFilename;

        int err = lrdf_read_file( sRDFFile.toStdString().c_str() );
        if (err) {
            qDebug() << "Error parsing rdf file " + sFilename;
        }

        RDFDescend( "http://ladspa.org/ontology#Plugin", pGroup, pluginList );
    }
}


// funzione ricorsiva
void Effects::RDFDescend( const QString& sBase, LadspaFXGroup *pGroup, QList<LadspaFXInfo*> pluginList )
{
// cout << "LadspaFX::RDFDescend " << sBase << endl;

    lrdf_uris* uris = lrdf_get_subclasses( sBase.toStdString().c_str() );
    if (uris) {
        for (int i = 0; i < (int)uris->count; i++) {
            QString sGroup = lrdf_get_label( uris->items[ i ] );

            LadspaFXGroup *pNewGroup = NULL;
            // verifico se esiste gia una categoria con lo stesso nome
            QList<LadspaFXGroup*> childGroups = pGroup-> getChildList();
            for (int nGroup = 0; nGroup < childGroups.size(); nGroup++) {
                LadspaFXGroup *pOldGroup = childGroups[nGroup];
                if (pOldGroup->getName() == sGroup) {
                    pNewGroup = pOldGroup;
                    break;
                }
            }
            if (pNewGroup == NULL) { // il gruppo non esiste, lo creo
                pNewGroup = new LadspaFXGroup( sGroup );
                pGroup->addChild( pNewGroup );
            }
            RDFDescend( uris->items[i], pNewGroup, pluginList );
        }
        lrdf_free_uris (uris);
    }

    uris = lrdf_get_instances( sBase.toStdString().c_str() );
    if (uris) {
        for (int i = 0; i < (int)uris->count; i++) {
            int uid = lrdf_get_uid (uris->items[i]);

            // verifico che il plugin non sia gia nella lista
            bool bExists = false;
            QList<LadspaFXInfo*> fxVect = pGroup->getLadspaInfo();
            for (int nFX = 0; nFX < fxVect.size(); nFX++) {
                LadspaFXInfo *pFX = fxVect[nFX];
                if (pFX->m_sID == uid) {
                    bExists = true;
                    continue;
                }
            }

            if ( bExists == false ) {
                // find the ladspaFXInfo
                for (int i = 0; i < pluginList.size(); i++) {
                    LadspaFXInfo *pInfo = pluginList[i];
                    if (pInfo->m_sID == uid) {
                        pGroup->addLadspaInfo( pInfo ); // copy the LadspaFXInfo
                    }
                }
            }
        }
        lrdf_free_uris (uris);
    }
}

#endif // LRDF_SUPPORT

}
; //LiveMix

#endif // LADSPA SUPPORT
