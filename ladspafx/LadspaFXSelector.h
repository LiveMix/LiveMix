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

#ifndef LADSPA_FX_SELECTOR_H
#define LADSPA_FX_SELECTOR_H

#include "ui_LadspaFXSelector_UI.h"
#include "ladspa_fx.h"

#include <QtGui/QDialog>

namespace LiveMix
{

class LadspaFXSelector : public QDialog, public Ui_LadspaFXSelector_UI
{
    Q_OBJECT

public:
    LadspaFXSelector(LadspaFX* nLadspaFX);
    ~LadspaFXSelector();

    QString getSelectedFX();

private slots:
    void on_m_pGroupsListView_currentItemChanged( QTreeWidgetItem * current, QTreeWidgetItem * previous );
    void pluginSelected();

private:
    QString m_sSelectedPluginName;

    void buildLadspaGroups();
#ifdef LADSPA_SUPPORT
//  void addGroup(QTreeWidgetItem* pItem, TargetType target, QString pName);
    void addGroup(QTreeWidgetItem *pItem, LadspaFXGroup *pGroup);

    QList<LadspaFXInfo*> findPluginsInGroup( const QString& sSelectedGroup, LadspaFXGroup *pGroup );
#endif

};

}
; //LiveMix

#endif
