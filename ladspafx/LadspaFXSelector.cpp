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
 */

#include "LadspaFXSelector.h"
//#include "LadspaFXSelector.moc"

#include "Effects.h"
#include "globals.h"

#include <QtCore/QString>
#include <QtCore/QDebug>
#include <QtGui/QPixmap>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QListWidget>
#include <QtGui/QListWidgetItem>

namespace LiveMix
{

LadspaFXSelector::LadspaFXSelector(LadspaFX* nLadspaFX)
        : QDialog()
{
    //qDebug() << "INIT" );

    setupUi( this );

    setFixedSize( width(), height() );

    setWindowTitle( trUtf8( "Select LADSPA FX" ) );
    setWindowIcon( QPixmap( ":/data/icon16.png" ) );

    m_sSelectedPluginName = "";

    m_nameLbl->setText( QString("") );
    m_labelLbl->setText( QString("") );
    m_typeLbl->setText( QString("") );
    m_pIDLbl->setText( QString("") );
    m_pMakerLbl->setText( QString("") );
    m_pCopyrightLbl->setText( QString("") );
    m_pPluginsListBox->clear();
    m_pOkBtn->setEnabled(false);

    m_pGroupsListView->setHeaderLabels( QStringList( trUtf8( "Groups" ) ) );

#ifdef LADSPA_SUPPORT
    //Song *pSong = Hydrogen::getInstance()->getSong();
// LadspaFX *pFX = Effects::getInstance()->getLadspaFX(nLadspaFX);
    if (nLadspaFX) {
        m_sSelectedPluginName = nLadspaFX->getPluginName();
    }
    buildLadspaGroups();

    m_pGroupsListView->sortItems( 0 , Qt::AscendingOrder);
    m_pGroupsListView->setItemHidden( m_pGroupsListView->headerItem(), true );


// LadspaFXGroup* nLadspaFX->fxGroup = LadspaFX::getLadspaFXGroup();
// vector<LadspaFXInfo*> list = findPluginsInGroup( m_sSelectedPluginName, nLadspaFX->fxGroup );
// for (uint i = 0; i < list.size(); i++) {
//  m_pPluginsListBox->addItem( list[i]->m_sName.c_str() );
// }
#endif

    connect( m_pPluginsListBox, SIGNAL( itemSelectionChanged () ), this, SLOT( pluginSelected() ) );
}



LadspaFXSelector::~LadspaFXSelector()
{
    //qDebug() << "DESTROY" );
}



void LadspaFXSelector::buildLadspaGroups()
{
#ifdef LADSPA_SUPPORT
    m_pGroupsListView->clear();

    QTreeWidgetItem* pRootItem = new QTreeWidgetItem( );
    pRootItem->setText( 0, trUtf8("Groups") );
    m_pGroupsListView->addTopLevelItem( pRootItem );
    m_pGroupsListView->setItemExpanded( pRootItem, true );

    LadspaFXGroup* nFxGroup = Effects::getInstance()->getLadspaFXGroup();
    for (int i = 0; i < nFxGroup->getChildList().size(); i++) {
        LadspaFXGroup *pNewGroup = ( nFxGroup->getChildList() )[ i ];
        addGroup( pRootItem, pNewGroup );
    }
#endif
}



#ifdef LADSPA_SUPPORT
void LadspaFXSelector::addGroup( QTreeWidgetItem *pItem, LadspaFXGroup *pGroup )
{
    QString sGroupName = pGroup->getName();
    if (sGroupName == "Uncategorized") {
        sGroupName = trUtf8("Uncategorized");
    } else if (sGroupName == "Categorized(LRDF)") {
        sGroupName = trUtf8("Categorized (LRDF)");
    }
    QTreeWidgetItem* pNewItem = new QTreeWidgetItem( pItem );
    pNewItem->setText( 0, sGroupName );


    for (int i = 0; i < pGroup->getChildList().size(); i++ ) {
        LadspaFXGroup *pNewGroup = ( pGroup->getChildList() )[ i ];

        addGroup( pNewItem, pNewGroup );
    }
    for (int i = 0; i < pGroup->getLadspaInfo().size(); i++) {
        LadspaFXInfo* pInfo = (pGroup->getLadspaInfo())[i];
        if (pInfo->m_sName == m_sSelectedPluginName) {
            m_pGroupsListView->setItemSelected(pNewItem, true);
            break;
        }
    }
}
#endif



QString LadspaFXSelector::getSelectedFX()
{
    return m_sSelectedPluginName;
}



void LadspaFXSelector::pluginSelected()
{
#ifdef LADSPA_SUPPORT
    qDebug() << "[pluginSelected]";

    if (!m_pPluginsListBox->selectedItems().isEmpty()) {
        m_sSelectedPluginName = m_pPluginsListBox->selectedItems().first()->text();

        QList<LadspaFXInfo*> pluginList = Effects::getInstance()->getPluginList();
        for (int i = 0; i < pluginList.size(); i++) {
            LadspaFXInfo *nFxInfo = pluginList[i];
            if (nFxInfo->m_sName == m_sSelectedPluginName ) {

                m_nameLbl->setText(  nFxInfo->m_sName );
                m_labelLbl->setText(  nFxInfo->m_sLabel );

                if ( ( nFxInfo->m_nIAPorts == 2 ) && ( nFxInfo->m_nOAPorts == 2 ) ) {  // Stereo plugin
                    m_typeLbl->setText( trUtf8("Stereo") );
                } else if ( ( nFxInfo->m_nIAPorts == 1 ) && ( nFxInfo->m_nOAPorts == 1 ) ) { // Mono plugin
                    m_typeLbl->setText( trUtf8("Mono") );
                } else {
                    // not supported
                    m_typeLbl->setText( trUtf8("Not supported") );
                }

                m_pIDLbl->setText(  QString("%1").arg(nFxInfo->m_sID) );
                m_pMakerLbl->setText(  nFxInfo->m_sMaker );
                m_pCopyrightLbl->setText(  nFxInfo->m_sCopyright );

                break;
            }
        }
        m_pOkBtn->setEnabled(true);
    } else {
        m_nameLbl->setText("");
        m_labelLbl->setText("");
        m_typeLbl->setText("");
        m_pIDLbl->setText("");
        m_pMakerLbl->setText("");
        m_pCopyrightLbl->setText("");
        m_pOkBtn->setEnabled(false);
    }
#endif
}



void LadspaFXSelector::on_m_pGroupsListView_currentItemChanged( QTreeWidgetItem * currentItem, QTreeWidgetItem * previous )
{
    UNUSED(previous);
#ifdef LADSPA_SUPPORT
    //qDebug() << "new selection: " + currentItem->text(0) );

    m_pOkBtn->setEnabled(false);
    m_nameLbl->setText( "" );
    m_labelLbl->setText( "" );
    m_typeLbl->setText( "" );
    m_pIDLbl->setText( "" );
    m_pMakerLbl->setText( "" );
    m_pCopyrightLbl->setText( "" );

    // nothing was selected
    if ( m_pGroupsListView->selectedItems().size() == 0 ) {
        return;
    }


    QString itemText = currentItem->text( 0 );

    //m_pPluginsListBox->clear();

    while( m_pPluginsListBox->count() != 0) {
        m_pPluginsListBox->takeItem( 0 );
    }

    LadspaFXGroup* nFxGroup = Effects::getInstance()->getLadspaFXGroup();

    QList<LadspaFXInfo*> pluginList = findPluginsInGroup( itemText, nFxGroup );
    for (int i = 0; i < pluginList.size(); i++) {
        //qDebug() << "adding plugin: " + pluginList[ i ]->m_sName );
        m_pPluginsListBox->addItem( pluginList[ i ]->m_sName );
        if ( pluginList[ i ]->m_sName == m_sSelectedPluginName ) {
            m_pPluginsListBox->setCurrentRow( i );
        }
    }
    m_pPluginsListBox->sortItems();
#endif
}


#ifdef LADSPA_SUPPORT
QList<LadspaFXInfo*> LadspaFXSelector::findPluginsInGroup( const QString& sSelectedGroup, LadspaFXGroup *pGroup )
{
    //qDebug() << "group: " + sSelectedGroup );
    QList<LadspaFXInfo*> list;

    if ( pGroup->getName() == sSelectedGroup ) {
        //qDebug() << "found..." );
        for (int i = 0; i < pGroup->getLadspaInfo().size(); ++i ) {
            LadspaFXInfo *pInfo = ( pGroup->getLadspaInfo() )[i];
            list.push_back( pInfo );
        }
        return list;
    } else {
        //qDebug() << "not found...searching in the child groups" );
        for (int i = 0; i < pGroup->getChildList().size(); ++i ) {
            LadspaFXGroup *pNewGroup = ( pGroup->getChildList() )[ i ];
            list = findPluginsInGroup( sSelectedGroup, pNewGroup );
            if (list.size() != 0) {
                return list;
            }
        }
    }

    //WARNINGLOG( "[findPluginsInGroup] no group found ('" + sSelectedGroup + "')" );
    return list;
}

}
; //LiveMix

#endif
