/*
 * Copyright 2004 - 2006 Arnold Krille <arnold@arnoldarts.de>
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

#ifndef JACKMIX_MAINWINDOW_H
#define JACKMIX_MAINWINDOW_H

#include "ladspa_fx.h"
#include "backend.h"
//#include "qlash.h"

#include <QtGui/QMainWindow>
#include <QtGui/QDockWidget>
#include <QtCore/QList>
#include <QtGui/QGridLayout>

class QHBox;
class QSettings;
class QDomElement;
class QVBox;
class QAction;
class QMenu;

namespace LiveMix
{

class Backend;
class Widget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* =0);
    MainWindow(QString filename, QWidget* =0);
    ~MainWindow();

public slots:
    void openFile();
    void openFile(QString path);
    void saveFile();
    void saveFile(QString p_rPath);

    void saveLash(QString p_rPath);
    void restoreLash(QString p_rPath);

    void restoreConnexions(QString p_rDir);

private slots:
    void closeEvent(QCloseEvent*);

    void addInputMono();
    void addPreMono();
    void addPostMonoExternal();
    void addPostMonoInternal();
    void addSubMono();

    void addInputStereo();
    void addPreStereo();
    void addPostStereoExternal();
    void addPostStereoInternal();
    void addSubStereo();

    void multipleAddInputMono();
    void multipleAddInputStereo();

    void removeInput();
    void removePre();
    void removePost();
    void removeSub();

    void removeInput(QString);
    void removePre(QString);
    void removePost(QString);
    void removeSub(QString);

    void about();
    void aboutQt();

    void initMatrix();
    void scheduleInit();

    void openDefaultMenu();
    void openEmpty();

    void saveConnexions(QString p_rDir);

private:
    void toEmpty();
    void openDefault();

    void addInput(QString, bool);
// void addOutput( QString, bool );
    void addPre(QString, bool);
    void addPost(QString, bool, bool);
    void addSub(QString, bool);

    void init();

    LadspaFX* openEffect(const QDomElement& elem);
    void saveEffect(QString& xml, effect*);

    void openActionBinding(const QDomElement& binding, const ChannelType p_eType, const QString& p_sChannelName, const QString& p_sTagName, const ElementType p_eElemetType, bool p_bMain =false);
    void openActionBindingList(const QDomElement& binding, const ChannelType p_eType, const QString& p_sChannelName, const QString& p_sTagName, const ElementType p_eElemetType, bool p_bMain =false);
    void openActionBinding(const QDomElement& channel, const ChannelType p_eType, const QString& p_sChannelName, bool p_bMain =false);
    QString saveActionBinding(const ChannelType p_eType, const QString& p_sChannelName);

    void openMidiBinding(const QDomElement& binding, const ChannelType p_eType, const QString& p_sChannelName);
    QString saveMidiBinding(const ChannelType p_eType, const QString& p_sChannelName);

    bool toBool(QString);
    QString fromBool(bool);

    int config_restore_id;
    QMenu *_filemenu, *_editmenu, *_viewmenu, *_settingsmenu, *_helpmenu;
    Widget *_mixerwidget;
    QAction *_new_empty, *_new_default;
    QAction *_add_inchannel_action, *_add_prechannel_action, *_add_postchannel_action, *_add_subchannel_action;
    QAction *_add_stinchannel_action, *_add_stprechannel_action, *_add_stpostchannel_action, *_add_stsubchannel_action;
    QAction *_add_intpostchannel_action, *_add_stintpostchannel_action;
    QAction *_remove_inchannel_action, *_remove_prechannel_action, *_remove_postchannel_action, *_remove_subchannel_action;
    QAction *m_pFaderHeight, *m_pEffectFaderHeight, *m_pShowGain;
    QAction *_debugPrint;
    bool _initScheduled;
    void timerEvent(QTimerEvent*);
//    qLashClient *m_lash;
};

}
; //LiveMix

#endif
