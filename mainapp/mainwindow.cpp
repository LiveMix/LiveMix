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

#include "mainwindow.h"

#include "mixingmatrix.h"
#include "channelselector.h"
#include "graphicalguiserver.h"
#include "qlash.h"

#include <QDebug>
#include <QMenu>
#include <QMenuBar>
#include <QLayout>
#include <QInputDialog>
#include <QSettings>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QAction>
#include <QTimer>
#include <QCloseEvent>
#include <QStatusBar>
#include <QCoreApplication>
#include <QDomDocument>

#include <typeinfo>


namespace LiveMix
{

MainWindow::MainWindow(QWidget* p) : QMainWindow(p), _initScheduled(true)
{
    qDebug() << "MainWindow::MainWindow()";
    Backend::init(new GraphicalGuiServer(this));
    init();

//    openDefault();

    _initScheduled = false;
    scheduleInit();

    qDebug() << "MainWindow::MainWindow() finished...";
}

MainWindow::MainWindow(QString filename, QWidget* p) : QMainWindow(p), _initScheduled(true)
{
    qDebug() << "MainWindow::MainWindow(" << filename << "," << p << ")";
    Backend::init(new GraphicalGuiServer(this));
    init();

    openFile(filename);

    QStringList ins = Backend::instance()->inchannels();
    QStringList outs = Backend::instance()->outchannels();
    if (ins.empty() || outs.empty()) {
        statusBar()->showMessage(trUtf8("No Channels available :-("));
    }

    _initScheduled = false;
    scheduleInit();

//    qDebug() << "MainWindow::MainWindow() finished...";
}

void MainWindow::saveConnexions(QString p_rDir)
{
//    qDebug() << "MainWindow::saveConnexions(" << p_rDir << ")";
    saveFile(QString("%1/table.lm").arg(p_rDir));

    Backend::instance()->saveConnexions(QString("%1/connexions.xml").arg(p_rDir));
}

void MainWindow::restoreConnexions(QString p_rDir)
{
//    qDebug() << "MainWindow::restoreConnexions(" << p_rDir << ")";
    openFile(QString("%1/table.lm").arg(p_rDir));

    Backend::instance()->restoreConnexions(QString("%1/connexions.xml").arg(p_rDir));
//    _lashclient->setJackName( "LiveMix" );
//    qDebug() << "MainWindow::restoreConnexions() finished";
}

void MainWindow::init()
{
    m_lash = new qLashClient("Livemix", 0, NULL, this);
    connect(m_lash, SIGNAL(quitApp()), this, SLOT(close()));
    connect(m_lash, SIGNAL(saveToDir(QString)), this, SLOT(saveLash(QString)));
    connect(m_lash, SIGNAL(restoreFromDir(QString)), this, SLOT(restoreLash(QString)));

    layout()->setSizeConstraint(QLayout::SetMinimumSize);

    _filemenu = menuBar()->addMenu(trUtf8("&File"));
//    _filemenu->addAction(trUtf8("Open File..."), this, SLOT( openFile() ), Qt::CTRL+Qt::Key_O );
//    _filemenu->addAction(trUtf8("Save File..."), this, SLOT( saveFile() ), Qt::CTRL+Qt::Key_S );
    _filemenu->addAction(trUtf8("&Open File..."), this, SLOT(openFile()));
    _filemenu->addAction(trUtf8("&Save File..."), this, SLOT(saveFile()));
    _filemenu->addSeparator();
    _filemenu->addAction(trUtf8("Open &default"), this, SLOT(openDefaultMenu()));
    _filemenu->addAction(trUtf8("&Empty table"), this, SLOT(openEmpty()));
    _filemenu->addSeparator();
    _filemenu->addAction(trUtf8("&Quit"), this, SLOT(close()), Qt::CTRL+Qt::Key_Q);

    _editmenu = menuBar()->addMenu(trUtf8("&Edit"));
// _select_action = new QAction( "Select Mode"), this );
// _select_action->setCheckable( true );
// connect( _select_action, SIGNAL( triggered() ), this, SLOT( toggleselectmode() ) );
    //_editmenu->addAction( _select_action );
    //_select_action->addTo( new QToolBar( this ) );
// _editmenu->addAction(trUtf8("&Fill empty spaces"), this, SLOT( scheduleInit() ) );
// _editmenu->addSeparator();
    // INPUT
    QMenu* editInput = _editmenu->addMenu(trUtf8("&Input"));

    _add_inchannel_action = new QAction(trUtf8("Add &mono..."), this);
    connect(_add_inchannel_action, SIGNAL(triggered()), this, SLOT(addInputMono()));
    editInput->addAction(_add_inchannel_action);

    _add_stinchannel_action = new QAction(trUtf8("Add &stereo ..."), this);
    connect(_add_stinchannel_action, SIGNAL(triggered()), this, SLOT(addInputStereo()));
    editInput->addAction(_add_stinchannel_action);

    _add_inchannel_action = new QAction(trUtf8("Add multiple mono..."), this);
    connect(_add_inchannel_action, SIGNAL(triggered()), this, SLOT(multipleAddInputMono()));
    editInput->addAction(_add_inchannel_action);

    _add_stinchannel_action = new QAction(trUtf8("Add multiple stereo ..."), this);
    connect(_add_stinchannel_action, SIGNAL(triggered()), this, SLOT(multipleAddInputStereo()));
    editInput->addAction(_add_stinchannel_action);

    _remove_inchannel_action = new QAction(trUtf8("&Remove..."), this);
    connect(_remove_inchannel_action, SIGNAL(triggered()), this, SLOT(removeInput()));
    editInput->addAction(_remove_inchannel_action);

    // PRE
    QMenu* editPre = _editmenu->addMenu(trUtf8("&Pre"));
    _add_prechannel_action = new QAction(trUtf8("Add &mono..."), this);
    connect(_add_prechannel_action, SIGNAL(triggered()), this, SLOT(addPreMono()));
    editPre->addAction(_add_prechannel_action);
    _add_stprechannel_action = new QAction(trUtf8("Add &stereo..."), this);
    connect(_add_stprechannel_action, SIGNAL(triggered()), this, SLOT(addPreStereo()));
    editPre->addAction(_add_stprechannel_action);
    _remove_prechannel_action = new QAction(trUtf8("&Remove..."), this);
    connect(_remove_prechannel_action, SIGNAL(triggered()), this, SLOT(removePre()));
    editPre->addAction(_remove_prechannel_action);

    // POST
    QMenu* editPost = _editmenu->addMenu(trUtf8("P&ost"));
    _add_intpostchannel_action = new QAction(trUtf8("Add Internal &mono..."), this);
    connect(_add_intpostchannel_action, SIGNAL(triggered()), this, SLOT(addPostMonoInternal()));
    editPost->addAction(_add_intpostchannel_action);
    _add_stintpostchannel_action = new QAction(trUtf8("Add Internal &stereo..."), this);
    connect(_add_stintpostchannel_action, SIGNAL(triggered()), this, SLOT(addPostStereoInternal()));
    editPost->addAction(_add_stintpostchannel_action);
    _add_postchannel_action = new QAction(trUtf8("Add &External mono..."), this);
    connect(_add_postchannel_action, SIGNAL(triggered()), this, SLOT(addPostMonoExternal()));
    editPost->addAction(_add_postchannel_action);
    _add_stpostchannel_action = new QAction(trUtf8("Add External stereo..."), this);
    connect(_add_stpostchannel_action, SIGNAL(triggered()), this, SLOT(addPostStereoExternal()));
    editPost->addAction(_add_stpostchannel_action);
    _remove_postchannel_action = new QAction(trUtf8("&Remove..."), this);
    connect(_remove_postchannel_action, SIGNAL(triggered()), this, SLOT(removePost()));
    editPost->addAction(_remove_postchannel_action);

    // SUB
    QMenu* editSub = _editmenu->addMenu(trUtf8("&Sub"));
    _add_subchannel_action = new QAction(trUtf8("Add &Mono..."), this);
    connect(_add_subchannel_action, SIGNAL(triggered()), this, SLOT(addSubMono()));
    editSub->addAction(_add_subchannel_action);
    _add_stsubchannel_action = new QAction(trUtf8("Add &Stereo..."), this);
    connect(_add_stsubchannel_action, SIGNAL(triggered()), this, SLOT(addSubStereo()));
    editSub->addAction(_add_stsubchannel_action);
    _remove_subchannel_action = new QAction(trUtf8("&Remove..."), this);
    connect(_remove_subchannel_action, SIGNAL(triggered()), this, SLOT(removeSub()));
    editSub->addAction(_remove_subchannel_action);

    _mixerwidget = new Widget(this);

    QMenu* preferences = _editmenu->addMenu(trUtf8("Pre&ferences"));
//    m_pShowGain = new QAction(trUtf8("Show/hide &gain"), this );
//    connect(m_pShowGain, SIGNAL(triggered()), _mixerwidget, SLOT(showGain()));
//    preferences->addAction(m_pShowGain);

    QAction *showAll = new QAction(trUtf8("&Show all part"), this);
    connect(showAll, SIGNAL(triggered()), _mixerwidget, SLOT(showAll()));
    preferences->addAction(showAll);
    QAction *hideAll = new QAction(trUtf8("&Hide all part"), this);
    connect(hideAll, SIGNAL(triggered()), _mixerwidget, SLOT(hideAll()));
    preferences->addAction(hideAll);

    m_pFaderHeight = new QAction(trUtf8("Set &fader height..."), this);
    connect(m_pFaderHeight, SIGNAL(triggered()), _mixerwidget, SLOT(faderHeight()));
    preferences->addAction(m_pFaderHeight);
    m_pEffectFaderHeight = new QAction(trUtf8("Set &effect fader height..."), this);
    connect(m_pEffectFaderHeight, SIGNAL(triggered()), _mixerwidget, SLOT(effectFaderHeight()));
    preferences->addAction(m_pEffectFaderHeight);

    _helpmenu = menuBar()->addMenu(trUtf8("&Help"));
    _helpmenu->addAction(trUtf8("About &LiveMix"), this, SLOT(about()));
    _helpmenu->addAction(trUtf8("About &Qt"), this, SLOT(aboutQt()));

    setCentralWidget(_mixerwidget);

    startTimer(1000);   // Fire every seconds.
}

MainWindow::~MainWindow()
{
    qDebug() << "MainWindow::~MainWindow()";
    /* delete Backend::instance();
     delete _filemenu;
     delete _editmenu;
     delete _viewmenu;
     delete _settingsmenu;
     delete _helpmenu;
    // delete _mixerwidget;
     delete _add_inchannel_action;
     delete _add_prechannel_action;
     delete _add_postchannel_action;
     delete _add_subchannel_action;
     delete _add_stinchannel_action;
     delete _add_stprechannel_action;
     delete _add_stpostchannel_action;
     delete _add_stsubchannel_action;
     delete _remove_inchannel_action;
     delete _remove_prechannel_action;
     delete _remove_postchannel_action;
     delete _remove_subchannel_action;
     delete _debugPrint;*/
}

void MainWindow::closeEvent(QCloseEvent* e)
{
//    qDebug() << "MainWindow::closeEvent( QCloseEvent " << e << " )";
    e->accept();
}

void MainWindow::openFile()
{
    QString path = QFileDialog::getOpenFileName(this, 0, 0, trUtf8("LiveMix (*.lm)"));
    openFile(path);
}

bool MainWindow::toBool(QString value)
{
    bool result = false;
    result |= value.compare("yes", Qt::CaseInsensitive) == 0;
    result |= value.compare("on", Qt::CaseInsensitive) == 0;
    result |= value.compare("true", Qt::CaseInsensitive) == 0;
    return result;
}
QString MainWindow::fromBool(bool value)
{
    return value ? "yes" : "no";
}
void MainWindow::openDefaultMenu()
{
    if (QMessageBox::question(this, trUtf8("New mix table"), trUtf8("Are you shure that you want to lost the actual mix table"),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
        openDefault();
    }
}
void MainWindow::toEmpty()
{
    while (Backend::instance()->inchannels().size() > 0) {
        removeInput(Backend::instance()->inchannels()[0]);
    }
    while (Backend::instance()->prechannels().size() > 0) {
        removePre(Backend::instance()->prechannels()[0]);
    }
    while (Backend::instance()->postchannels().size() > 0) {
        removePost(Backend::instance()->postchannels()[0]);
    }
    while (Backend::instance()->subchannels().size() > 0) {
        removeSub(Backend::instance()->subchannels()[0]);
    }
    while (Backend::instance()->getOutEffects(MAIN)->size() > 0) {
        effect *fx = *Backend::instance()->getOutEffects(MAIN)->begin();
        _mixerwidget->removeFX(fx->gui, fx, OUT, MAIN);
    }

    _mixerwidget->clearAll();
}
void MainWindow::openEmpty()
{
    if (QMessageBox::question(this, trUtf8("New mix table"), trUtf8("Are you shure that you want to lost the actual mix table"),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
        toEmpty();
        _mixerwidget->doSelect(OUT, MAIN, false);
    }
}
void MainWindow::openDefault()
{
    toEmpty();

    if (QFile(QCoreApplication::applicationDirPath() + "/default.lm").exists()) {
        openFile(QCoreApplication::applicationDirPath() + "/default.lm");
    } else if (QFile(QCoreApplication::applicationDirPath() + "/../share/livemix/sample/default.lm").exists()) {
        openFile(QCoreApplication::applicationDirPath() + "/../share/livemix/sample/default.lm");
    } else {
        QStringList ins = QStringList() << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8";

        foreach(QString in, ins) {
            Backend::instance()->addInput(in, false);
        }
        Backend::instance()->addInput("9-10", true);
        Backend::instance()->addInput("11-12", true);
        Backend::instance()->addPre("pre1", true);
        Backend::instance()->addPre("pre2", false);
        Backend::instance()->addPost("post1", true, true);
        Backend::instance()->addPost("post2", false, true);
        Backend::instance()->addSub("sub1", true);
        Backend::instance()->addSub("sub2", false);
        _mixerwidget->doSelect(OUT, MAIN, false);
    }
}
void MainWindow::openFile(QString path)
{
    //qDebug() << "MainWindow::openFile(" << path << ")";
    Backend::instance()->run(false);
    if (path.isEmpty()) {
        openDefault();
        return;
    }

    // delay autofill at least until all saved elements are created:
    bool save_initScheduled = _initScheduled;
    _initScheduled = true;

    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        toEmpty();

        QDomDocument doc("livemix");
        doc.setContent(&file);

        QDomElement livemix = doc.documentElement();
        QString version = livemix.attribute("version", "0.5");

        if (version == "0.5" || version == "0.4") {
            _mixerwidget->setFaderHeight(livemix.attribute("faderHeight", "200").toInt());
            _mixerwidget->setEffectFaderHeight(livemix.attribute("effectFaderHeight", "200").toInt());
            _mixerwidget->setVisible(toBool(livemix.attribute("gainVisible", "yes")), GAIN);
            _mixerwidget->setVisible(toBool(livemix.attribute("muteVisible", "yes")), MUTE);
            _mixerwidget->setVisible(toBool(livemix.attribute("pflVisible", "yes")), TO_PFL);
            _mixerwidget->setVisible(toBool(livemix.attribute("toMainVisible", "yes")), TO_MAIN);
            _mixerwidget->setVisible(toBool(livemix.attribute("balVisible", "yes")), PAN_BAL);
            for (QDomElement visible = livemix.firstChildElement("preVisible"); !visible.isNull(); visible = visible.nextSiblingElement("preVisible")) {
                QString name = visible.attribute("name");
                _mixerwidget->setVisible(toBool(visible.attribute("visible", "yes")), TO_PRE, name);
            }
            for (QDomElement visible = livemix.firstChildElement("postVisible"); !visible.isNull(); visible = visible.nextSiblingElement("postVisible")) {
                QString name = visible.attribute("name");
                _mixerwidget->setVisible(toBool(visible.attribute("visible", "yes")), TO_POST, name);
            }
            for (QDomElement visible = livemix.firstChildElement("subVisible"); !visible.isNull(); visible = visible.nextSiblingElement("subVisible")) {
                QString name = visible.attribute("name");
                _mixerwidget->setVisible(toBool(visible.attribute("visible", "yes")), TO_SUB, name);
            }

            openActionBinding(livemix, IN, "", true);

            for (QDomElement in = livemix.firstChildElement("in"); !in.isNull(); in = in.nextSiblingElement("in")) {
                QString name = in.attribute("name");
                openActionBinding(in, IN, name);
                openMidiBinding(in, IN, name);
                Backend::instance()->addInput(name, toBool(in.attribute("stereo")));
                Backend::instance()->getChannel(IN, name)->display_name = in.attribute("display");
                Backend::instance()->setInGain(name, in.attribute("gain").toFloat());
                Backend::instance()->setInVolume(name, in.attribute("volume").toFloat());
                Backend::instance()->setInBal(name, in.attribute("bal").toFloat());
                Backend::instance()->setInMute(name, toBool(in.attribute("mute")));
                Backend::instance()->setInPfl(name, toBool(in.attribute("pfl")));
                Backend::instance()->setInMain(name, toBool(in.attribute("main")));
                for (QDomElement pre = in.firstChildElement("pre"); !pre.isNull(); pre = pre.nextSiblingElement("pre")) {
                    Backend::instance()->setInPreVolume(name, pre.attribute("name"), pre.attribute("volume").toFloat());
                }
                for (QDomElement post = in.firstChildElement("post"); !post.isNull(); post = post.nextSiblingElement("post")) {
                    Backend::instance()->setInPostVolume(name, post.attribute("name"), post.attribute("volume").toFloat());
                }
                for (QDomElement sub = in.firstChildElement("sub"); !sub.isNull(); sub = sub.nextSiblingElement("sub")) {
                    Backend::instance()->setInSub(name, sub.attribute("name"), toBool(sub.attribute("mute")));
                }

                for (QDomElement effect = in.firstChildElement("effect"); !effect.isNull(); effect = effect.nextSiblingElement("effect")) {
                    Backend::instance()->addInEffect(name, openEffect(effect));
                }
            }

            QDomElement out = livemix.firstChildElement("out");
            if (!out.isNull()) {
                QDomElement binding = out.firstChildElement("actionbinding");
                if (!binding.isNull()) {
                    QDomElement select = binding.firstChildElement("select");
                    if (!select.isNull()) {
                        _mixerwidget->insertKeyToWrapp(QKeySequence(select.attribute("key")), new KeyDoSelectChannel(_mixerwidget, OUT, MAIN));
                    }
                    openActionBinding(binding, OUT, MAIN, "bal", PAN_BAL);
                    openActionBindingList(binding, OUT, MAIN, "fader", FADER);
                    openActionBinding(binding, OUT, MAIN, "mute", MUTE);
                    openActionBinding(binding, OUT, MAIN, "afl", TO_AFL);
                    openActionBinding(binding, OUT, MAIN, "effect", MUTE_EFFECT);
                }

                QDomElement main = out.firstChildElement("main");
                if (!main.isNull()) {
                    openMidiBinding(main, OUT, MAIN);
                }
                QDomElement mono = out.firstChildElement("mono");
                if (!mono.isNull()) {
                    openMidiBinding(mono, OUT, MONO);
                }
                QDomElement pfl = out.firstChildElement("pfl");
                if (!pfl.isNull()) {
                    openMidiBinding(pfl, OUT, PFL);
                }

                Backend::instance()->setOutVolume(MAIN, out.attribute("volume").toDouble());
                Backend::instance()->setOutMute(MAIN, toBool(out.attribute("mute")));
                Backend::instance()->setOutAfl(MAIN, toBool(out.attribute("afl")));
                Backend::instance()->setOutBal(MAIN, out.attribute("bal").toDouble());
                Backend::instance()->setOutVolume(MONO, out.attribute("monovolume").toDouble());
                Backend::instance()->setOutVolume(PFL, out.attribute("phonevolume").toDouble());

                for (QDomElement effect = out.firstChildElement("effect"); !effect.isNull(); effect = effect.nextSiblingElement("effect")) {
                    Backend::instance()->addOutEffect(MAIN, openEffect(effect));
                }
            }

            for (QDomElement pre = livemix.firstChildElement("pre"); !pre.isNull(); pre = pre.nextSiblingElement("pre")) {
                QString name = pre.attribute("name");
                openActionBinding(pre, PRE, name);
                openMidiBinding(pre, PRE, name);
                Backend::instance()->addPre(name, toBool(pre.attribute("stereo")));
                Backend::instance()->getChannel(PRE, name)->display_name = pre.attribute("display");
                Backend::instance()->setPreVolume(name, pre.attribute("volume").toDouble());
                Backend::instance()->setPreMute(name, toBool(pre.attribute("mute")));
                Backend::instance()->setPreAfl(name, toBool(pre.attribute("afl")));

                for (QDomElement effect = pre.firstChildElement("effect"); !effect.isNull(); effect = effect.nextSiblingElement("effect")) {
                    Backend::instance()->addPreEffect(name, openEffect(effect));
                }
            }

            for (QDomElement post = livemix.firstChildElement("post"); !post.isNull(); post = post.nextSiblingElement("post")) {
                QString name = post.attribute("name");
                openActionBinding(post, POST, name);
                openMidiBinding(post, POST, name);
                Backend::instance()->addPost(name, toBool(post.attribute("stereo")), toBool(post.attribute("external")));
                Backend::instance()->getChannel(POST, name)->display_name = post.attribute("display");
                Backend::instance()->setPostPreVolume(name, post.attribute("pre-volume").toDouble());
                Backend::instance()->setPostPostVolume(name, post.attribute("post-volume").toDouble());
                Backend::instance()->setPostMute(name, toBool(post.attribute("mute")));
                Backend::instance()->setPostBal(name, post.attribute("bal").toFloat());
                Backend::instance()->setPostAfl(name, toBool(post.attribute("afl")));
                Backend::instance()->setPostPfl(name, toBool(post.attribute("pfl")));
                Backend::instance()->setPostMain(name, toBool(post.attribute("main")));

                for (QDomElement sub = post.firstChildElement("sub"); !sub.isNull(); sub = sub.nextSiblingElement("sub")) {
                    Backend::instance()->setPostSub(name, sub.attribute("name"), toBool(sub.attribute("mute")));
                }

                for (QDomElement effect = post.firstChildElement("effect"); !effect.isNull(); effect = effect.nextSiblingElement("effect")) {
                    Backend::instance()->addPostEffect(name, openEffect(effect));
                }
            }

            for (QDomElement sub = livemix.firstChildElement("sub"); !sub.isNull(); sub = sub.nextSiblingElement("sub")) {
                QString name = sub.attribute("name");
                openActionBinding(sub, SUB, name);
                openMidiBinding(sub, SUB, name);
                Backend::instance()->addSub(name, toBool(sub.attribute("stereo")));
                Backend::instance()->getChannel(SUB, name)->display_name = sub.attribute("display");
                Backend::instance()->setSubVolume(name, sub.attribute("volume").toDouble());
                Backend::instance()->setSubMute(name, toBool(sub.attribute("mute")));
                Backend::instance()->setSubAfl(name, toBool(sub.attribute("afl")));
                Backend::instance()->setSubMain(name, toBool(sub.attribute("main")));
                Backend::instance()->setSubBal(name, sub.attribute("bal").toFloat());

                for (QDomElement effect = sub.firstChildElement("effect"); !effect.isNull(); effect = effect.nextSiblingElement("effect")) {
                    Backend::instance()->addSubEffect(name, openEffect(effect));
                }
            }
        }

        file.close();
        // force update
        _mixerwidget->doSelect(OUT, PFL, false);
        _mixerwidget->doSelect(OUT, MAIN, false);
    } else {
        openDefault();
    }


    _initScheduled = save_initScheduled;
    scheduleInit();
    Backend::instance()->run(true);
    //qDebug() << "MainWindow::openFile() finished";
}
void MainWindow::timerEvent(QTimerEvent*)
{
    QDir(QDir::homePath()).mkdir(".livemix");
    saveFile(QDir::homePath().append("/.livemix/table.lm"));
    saveConnexions(QDir::homePath().append("/.livemix"));
}
void MainWindow::saveLash(QString p_rPath) {
    saveFile(p_rPath.append("/.livemix/table.lm"));
    saveConnexions(p_rPath.append("/.livemix"));
}
void MainWindow::restoreLash(QString p_rPath){
    openFile(p_rPath.append("/.livemix/table.lm"));
    restoreConnexions(p_rPath.append("/.livemix"));
}
void MainWindow::saveFile()
{
    QString path = QFileDialog::getSaveFileName(this, 0, 0, trUtf8("LiveMix (*.lm)"));
    if (path.isEmpty())
        return;

    if (! path.endsWith(".lm"))
        path += ".lm";

    saveFile(path);
}
void MainWindow::saveFile(QString p_rPath)
{
    QString xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    xml += QString("<livemix version=\"0.5\" faderHeight=\"%1\" effectFaderHeight=\"%2\" gainVisible=\"%3\" muteVisible=\"%4\" pflVisible=\"%5\" toMainVisible=\"%6\" balVisible=\"%7\">")
           .arg(_mixerwidget->getFaderHeight()).arg(_mixerwidget->getEffectFaderHeight()).arg(fromBool(_mixerwidget->isVisible(GAIN)))
           .arg(fromBool(_mixerwidget->isVisible(MUTE))).arg(fromBool(_mixerwidget->isVisible(TO_PFL)))
           .arg(fromBool(_mixerwidget->isVisible(TO_MAIN))).arg(fromBool(_mixerwidget->isVisible(PAN_BAL)));

    foreach(QString name, Backend::instance()->prechannels()) {
        xml += QString("  <preVisible name=\"%1\" visible=\"%2\"/>").arg(name).arg(fromBool(_mixerwidget->isVisible(TO_PRE, name)));
    }
    foreach(QString name, Backend::instance()->postchannels()) {
        xml += QString("  <postVisible name=\"%1\" visible=\"%2\"/>").arg(name).arg(fromBool(_mixerwidget->isVisible(TO_POST, name)));
    }
    foreach(QString name, Backend::instance()->subchannels()) {
        xml += QString("  <subVisible name=\"%1\" visible=\"%2\"/>").arg(name).arg(fromBool(_mixerwidget->isVisible(TO_SUB, name)));
    }
    xml += QString("  <actionbinding>");
    foreach(QKeySequence rKey, _mixerwidget->getKeyToWrapp()->keys()) {
        KeyDo* pKeyDo = (*_mixerwidget->getKeyToWrapp())[rKey];
        if (typeid(*pKeyDo).name() == typeid(KeyDoChannelAction).name()) {
            KeyDoChannelAction* pKD = (KeyDoChannelAction*)pKeyDo;
            QString tagname;
            switch (pKD->m_eElement) {
            case GAIN:
                tagname = "gain";
                break;
            case PAN_BAL:
                tagname = "bal";
                break;
            case TO_PRE:
                tagname = "pre";
                break;
            case TO_POST:
                tagname = "post";
                break;
            case FADER:
                tagname = "fader";
                break;
            case PRE_VOL:
                tagname = "prevol";
                break;
            case MUTE:
                tagname = "mute";
                break;
            case TO_SUB:
                tagname = "sub";
                break;
            case TO_MAIN:
                tagname = "main";
                break;
            case TO_AFL:
                tagname = "afl";
                break;
            case TO_PFL:
                tagname = "pfl";
                break;
            case MUTE_EFFECT:
                tagname = "effect";
                break;
            }
            if (pKD->m_sReatedChannelName == "") {
                xml += QString("    <%1 key=\"%2\" />").arg(tagname).arg(rKey.toString());
            } else {
                xml += QString("    <%1 sub=\"%3\" key=\"%2\" />").arg(tagname).arg(rKey.toString()).arg(pKD->m_sReatedChannelName);
            }
        }
    }
    xml += QString("  </actionbinding>");

    foreach(QString name, Backend::instance()->inchannels()) {
        const in* elem = Backend::instance()->getInput(name);
        xml += QString("  <in name=\"%1\" display=\"%9\" gain=\"%2\" volume=\"%3\" mute=\"%4\" pfl=\"%5\" bal=\"%6\" stereo=\"%7\" main=\"%8\">")
               .arg(name).arg(elem->gain).arg(elem->volume).arg(fromBool(elem->mute)).arg(fromBool(elem->pfl))
               .arg(elem->bal).arg(fromBool(elem->stereo)).arg(fromBool(elem->main)).arg(elem->display_name);
        xml += saveActionBinding(IN, name);
        xml += saveMidiBinding(IN, name);

        foreach(QString pre, Backend::instance()->prechannels()) {
			map<QString, float> map = elem->pre;
            xml += QString("    <pre name=\"%1\" volume=\"%2\" />").arg(pre).arg(map[pre]);
        }
        foreach(QString post, Backend::instance()->postchannels()) {
			map<QString, float> map = elem->post;
            xml += QString("    <post name=\"%1\" volume=\"%2\" />").arg(post).arg(map[post]);
        }
        foreach(QString sub, Backend::instance()->subchannels()) {
			map<QString, bool> map = elem->sub;
            xml += QString("    <sub name=\"%1\" mute=\"%2\" />").arg(sub).arg(fromBool(map[sub]));
        }

		for (list<effect*>::const_iterator i = elem->effects.begin() ; i != elem->effects.end() ; i++) {
			effect* effect = *i;
            saveEffect(xml, effect);
        }
        xml += "  </in>";
    }

    {
        const out* elem = Backend::instance()->getOutput(MAIN);
        xml += QString("  <out volume=\"%1\" mute=\"%2\" afl=\"%3\" bal=\"%4\" monovolume=\"%5\" phonevolume=\"%6\">")
               .arg(elem->volume).arg(fromBool(elem->mute)).arg(fromBool(elem->afl)).arg(elem->bal)
               .arg(Backend::instance()->getOutput(MONO)->volume).arg(Backend::instance()->getOutput(PFL)->volume);
        xml += QString("    <actionbinding>");
        foreach(QKeySequence rKey, _mixerwidget->getKeyToWrapp()->keys()) {
            KeyDo* pKeyDo = (*_mixerwidget->getKeyToWrapp())[rKey];
            if (typeid(*pKeyDo).name() == typeid(KeyDoSelectChannel).name()) {
                KeyDoSelectChannel* pKD = (KeyDoSelectChannel*)pKeyDo;
                if (pKD->m_eType == OUT && pKD->m_sChannelName == MAIN) {
                    xml += QString("      <select key=\"%1\" />").arg(rKey.toString());
                }
            } else if (typeid(*pKeyDo).name() == typeid(KeyDoDirectAction).name()) {
                KeyDoDirectAction* pKD = (KeyDoDirectAction*)pKeyDo;
                if (pKD->m_eType == OUT && pKD->m_sChannelName == MAIN) {
                    QString tagname;
                    switch (pKD->m_eElement) {
                    case PAN_BAL:
                        tagname = "bal";
                        break;
                    case FADER:
                        tagname = "fader";
                        break;
                    case MUTE:
                        tagname = "mute";
                        break;
                    case TO_AFL:
                        tagname = "afl";
                        break;
                    case MUTE_EFFECT:
                        tagname = "effect";
                        break;
                    default:
                        break;
                    }
                    if (pKD->m_sReatedChannelName == "") {
                        xml += QString("      <%1 key=\"%2\" />").arg(tagname).arg(rKey.toString());
                    } else {
                        xml += QString("      <%1 sub=\"%3\" key=\"%2\" />").arg(tagname).arg(rKey.toString()).arg(pKD->m_sReatedChannelName);
                    }
                }
            }
        }
        xml += QString("    </actionbinding>");
        xml += QString("   <main>");
        xml += saveMidiBinding(OUT, MAIN);
        xml += QString("   </main>");
        xml += QString("   <mono>");
        xml += saveMidiBinding(OUT, MONO);
        xml += QString("   </mono>");
        xml += QString("   <pfl>");
        xml += saveMidiBinding(OUT, PFL);
        xml += QString("   </pfl>");

        for (list<effect*>::const_iterator effect = elem->effects.begin() ; effect != elem->effects.end() ; ++effect) {
            saveEffect(xml, *effect);
        }
        xml += "  </out>";
    }

    foreach(QString name, Backend::instance()->prechannels()) {
        const pre* elem = Backend::instance()->getPre(name);
        xml += QString("  <pre name=\"%1\" display=\"%6\" volume=\"%2\" mute=\"%3\" afl=\"%4\" stereo=\"%5\">")
               .arg(name).arg(elem->volume).arg(fromBool(elem->mute)).arg(fromBool(elem->afl))
               .arg(fromBool(elem->stereo)).arg(elem->display_name);
        xml += saveActionBinding(PRE, name);
        xml += saveMidiBinding(PRE, name);

        for (list<effect*>::const_iterator effect = elem->effects.begin() ; effect != elem->effects.end() ; ++effect) {
            saveEffect(xml, *effect);
        }
        xml += "  </pre>";
    }

    foreach(QString name, Backend::instance()->postchannels()) {
        const post* elem = Backend::instance()->getPost(name);
        xml += QString("  <post name=\"%1\" display=\"%11\" pre-volume=\"%2\" post-volume=\"%3\" mute=\"%4\" afl=\"%5\" stereo=\"%6\" main=\"%7\" bal=\"%8\" external=\"%9\" pfl=\"%10\">")
               .arg(name).arg(elem->prevolume).arg(elem->postvolume).arg(fromBool(elem->mute)).arg(fromBool(elem->m_bAfl))
               .arg(fromBool(elem->stereo)).arg(fromBool(elem->main)).arg(elem->bal).arg(fromBool(elem->external))
               .arg(fromBool(elem->m_bPfl)).arg(elem->display_name);
        xml += saveActionBinding(POST, name);
        xml += saveMidiBinding(POST, name);

        foreach(QString sub, Backend::instance()->subchannels()) {
            xml += QString("    <sub name=\"%1\" mute=\"%2\" />").arg(sub).arg(fromBool(elem->sub[ sub ]));
        }

        for (list<effect*>::const_iterator effect = elem->effects.begin() ; effect != elem->effects.end() ; ++effect) {
            saveEffect(xml, *effect);
        }
        xml += "  </post>";
    }

    foreach(QString name, Backend::instance()->subchannels()) {
        const sub* elem = Backend::instance()->getSub(name);
        xml += QString("  <sub name=\"%1\" display=\"%8\" volume=\"%2\" mute=\"%3\" afl=\"%4\" stereo=\"%5\" main=\"%6\" bal=\"%7\">")
               .arg(name).arg(elem->volume).arg(fromBool(elem->mute)).arg(fromBool(elem->afl)).arg(fromBool(elem->stereo))
               .arg(fromBool(elem->main)).arg(elem->bal).arg(elem->display_name);
        xml += saveActionBinding(SUB, name);
        xml += saveMidiBinding(SUB, name);

        for (list<effect*>::const_iterator effect = elem->effects.begin() ; effect != elem->effects.end() ; ++effect) {
            saveEffect(xml, *effect);
        }
        xml += "  </sub>";
    }

    xml += "</livemix>";

    QFile file(p_rPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << xml.replace(">", ">\n");
        file.close();
    }
}

LadspaFX* MainWindow::openEffect(const QDomElement& effect)
{
    LadspaFX* pFX = LadspaFX::load(effect.attribute("filename"), effect.attribute("name"), 44100);

    if (pFX == NULL) {
	return NULL; // no effect found
    }

    pFX->setEnabled(toBool(effect.attribute("enabled")));
    for (QDomElement attr = effect.firstChildElement("attribute"); !attr.isNull(); attr = attr.nextSiblingElement("attribute")) {
        QString sName = attr.attribute("name");
        float fValue = attr.attribute("value").toFloat();

        for (int nPort = 0; nPort < pFX->inputControlPorts.size(); nPort++) {
            LadspaControlPort *port = pFX->inputControlPorts[ nPort ];
            if (port->m_sName == sName) {
                port->m_fControlValue = fValue;
            }
        }
    }
    return pFX;
}

void MainWindow::openMidiBinding(const QDomElement& channel, const ChannelType p_eType, const QString& p_sChannelName)
{
    QDomElement binding = channel.firstChildElement("midibinding");
    if (!binding.isNull()) {
        for (QDomElement bind = binding.firstChildElement("bind"); !bind.isNull(); bind = bind.nextSiblingElement("bind")) {
            ElementType type = FADER;
            QString sType(bind.attribute("element"));
            if (sType == "gain") {
                type = GAIN;
            } else if (sType == "bal") {
                type = PAN_BAL;
            } else if (sType == "pre") {
                type = TO_PRE;
            } else if (sType == "post") {
                type = TO_POST;
            } else if (sType == "fader") {
                type = FADER;
            } else if (sType == "prevol") {
                type = PRE_VOL;
            } else if (sType == "mute") {
                type = MUTE;
            } else if (sType == "sub") {
                type = TO_SUB;
            } else if (sType == "main") {
                type = TO_MAIN;
            } else if (sType == "afl") {
                type = TO_AFL;
            } else if (sType == "pfl") {
                type = TO_PFL;
            } else if (sType == "effect") {
                type = MUTE_EFFECT;
            }

            _mixerwidget->insertMidiToWrapp((unsigned char)bind.attribute("channel").toUInt(),
                                            bind.attribute("controller").toUInt(), new KeyDoDirectAction(_mixerwidget, p_eType,
                                                    p_sChannelName, type, bind.attribute("channelto")));
        }
    }
}
void MainWindow::openActionBinding(const QDomElement& channel, const ChannelType p_eType, const QString& p_sChannelName, bool p_bMain)
{
    QDomElement binding = channel.firstChildElement("actionbinding");
    if (!binding.isNull()) {
        if (!p_bMain) {
            QDomElement select = binding.firstChildElement("select");
            if (!select.isNull()) {
                _mixerwidget->insertKeyToWrapp(QKeySequence(select.attribute("key")), new KeyDoSelectChannel(_mixerwidget, p_eType, p_sChannelName));
            }
        }
        openActionBinding(binding, p_eType, p_sChannelName, "gain", GAIN, p_bMain);
        openActionBinding(binding, p_eType, p_sChannelName, "bal", PAN_BAL, p_bMain);
        openActionBindingList(binding, p_eType, p_sChannelName, "pre", TO_PRE, p_bMain);
        openActionBindingList(binding, p_eType, p_sChannelName, "post", TO_POST, p_bMain);
        openActionBinding(binding, p_eType, p_sChannelName, "fader", FADER, p_bMain);
        openActionBinding(binding, p_eType, p_sChannelName, "prevol", PRE_VOL, p_bMain);
        openActionBinding(binding, p_eType, p_sChannelName, "mute", MUTE, p_bMain);
        openActionBindingList(binding, p_eType, p_sChannelName, "sub", TO_SUB, p_bMain);
        openActionBinding(binding, p_eType, p_sChannelName, "main", TO_MAIN, p_bMain);
        openActionBinding(binding, p_eType, p_sChannelName, "afl", TO_AFL, p_bMain);
        openActionBinding(binding, p_eType, p_sChannelName, "pfl", TO_PFL, p_bMain);
        openActionBindingList(binding, p_eType, p_sChannelName, "effect", MUTE_EFFECT, p_bMain);
    }
}

void MainWindow::openActionBinding(const QDomElement& binding, const ChannelType p_eType, const QString& p_sChannelName, const QString& p_sTagName, const ElementType p_eElemetType, bool p_bMain)
{
    QDomElement select = binding.firstChildElement(p_sTagName);
    if (!select.isNull()) {
        if (p_bMain) {
            _mixerwidget->insertKeyToWrapp(QKeySequence(select.attribute("key")), new KeyDoChannelAction(_mixerwidget, p_eElemetType, select.attribute("sub")));
        } else {
            _mixerwidget->insertKeyToWrapp(QKeySequence(select.attribute("key")), new KeyDoDirectAction(_mixerwidget, p_eType, p_sChannelName, p_eElemetType, binding.attribute("sub")));
        }
    }
}
void MainWindow::openActionBindingList(const QDomElement& binding, const ChannelType p_eType, const QString& p_sChannelName, const QString& p_sTagName, const ElementType p_eElemetType, bool p_bMain)
{
    for (QDomElement attr = binding.firstChildElement(p_sTagName); !attr.isNull(); attr = attr.nextSiblingElement(p_sTagName)) {
        if (p_bMain) {
            _mixerwidget->insertKeyToWrapp(QKeySequence(attr.attribute("key")), new KeyDoChannelAction(_mixerwidget, p_eElemetType, attr.attribute("sub")));
        } else {
            _mixerwidget->insertKeyToWrapp(QKeySequence(attr.attribute("key")), new KeyDoDirectAction(_mixerwidget, p_eType, p_sChannelName, p_eElemetType, attr.attribute("sub")));
        }
    }
}
QString MainWindow::saveActionBinding(ChannelType p_eType, const QString& p_sChannelName)
{
    QString xml("    <actionbinding>");
    foreach(QKeySequence rKey, _mixerwidget->getKeyToWrapp()->keys()) {
        KeyDo* pKeyDo = (*_mixerwidget->getKeyToWrapp())[rKey];
        if (typeid(*pKeyDo).name() == typeid(KeyDoSelectChannel).name()) {
            KeyDoSelectChannel* pKD = (KeyDoSelectChannel*)pKeyDo;
            if (pKD->m_eType == p_eType && pKD->m_sChannelName == p_sChannelName) {
                xml += QString("      <select key=\"%1\" />").arg(rKey.toString());
            }
        } else if (typeid(*pKeyDo).name() == typeid(KeyDoDirectAction).name()) {
            KeyDoDirectAction* pKD = (KeyDoDirectAction*)pKeyDo;
            if (pKD->m_eType == p_eType && pKD->m_sChannelName == p_sChannelName) {
                QString tagname;
                switch (pKD->m_eElement) {
                case GAIN:
                    tagname = "gain";
                    break;
                case PAN_BAL:
                    tagname = "bal";
                    break;
                case TO_PRE:
                    tagname = "pre";
                    break;
                case TO_POST:
                    tagname = "post";
                    break;
                case FADER:
                    tagname = "fader";
                    break;
                case PRE_VOL:
                    tagname = "prevol";
                    break;
                case MUTE:
                    tagname = "mute";
                    break;
                case TO_SUB:
                    tagname = "sub";
                    break;
                case TO_MAIN:
                    tagname = "main";
                    break;
                case TO_AFL:
                    tagname = "afl";
                    break;
                case TO_PFL:
                    tagname = "pfl";
                    break;
                case MUTE_EFFECT:
                    tagname = "effect";
                    break;
                }
                if (pKD->m_sReatedChannelName == "") {
                    xml += QString("      <%1 key=\"%2\" />").arg(tagname).arg(rKey.toString());
                } else {
                    xml += QString("      <%1 sub=\"%3\" key=\"%2\" />").arg(tagname).arg(rKey.toString()).arg(pKD->m_sReatedChannelName);
                }
            }
        }
    }
    xml += QString("    </actionbinding>");
    return xml;
}

QString MainWindow::saveMidiBinding(ChannelType p_eType, const QString& p_sChannelName)
{
    QString xml("    <midibinding>");
    foreach(unsigned char ch, _mixerwidget->getMidiToWrapp()->keys()) {
        foreach(unsigned int co, (*_mixerwidget->getMidiToWrapp())[ch]->keys()) {
            KeyDoDirectAction* pKeyDo = (*(*_mixerwidget->getMidiToWrapp())[ch])[co];
            if (pKeyDo->m_eType == p_eType && pKeyDo->m_sChannelName == p_sChannelName) {
                QString type = NULL;
                switch (pKeyDo->m_eElement) {
                case GAIN:
                    type = "gain";
                    break;
                case PAN_BAL:
                    type = "bal";
                    break;
                case TO_PRE:
                    type = "pre";
                    break;
                case TO_POST:
                    type = "post";
                    break;
                case FADER:
                    type = "fader";
                    break;
                case PRE_VOL:
                    type = "prevol";
                    break;
                case MUTE:
                    type = "mute";
                    break;
                case TO_SUB:
                    type = "sub";
                    break;
                case TO_MAIN:
                    type = "main";
                    break;
                case TO_AFL:
                    type = "afl";
                    break;
                case TO_PFL:
                    type = "pfl";
                    break;
                case MUTE_EFFECT:
                    type = "effect";
                    break;
                }
                //            xml += QString("      <bind channel=\"%1\" controller=\"%2\" type=\"%3\" name=\"%4\" element=\"%5\" channelto=\"%6\" />")
                //                    .arg(ch).arg(co).arg(channelType).arg(pKeyDo->m_sChannelName).arg(type).arg(pKeyDo->m_sReatedChannelName);
                xml += QString("      <bind channel=\"%1\" controller=\"%2\" element=\"%3\" channelto=\"%4\" />")
                       .arg(ch).arg(co).arg(type).arg(pKeyDo->m_sReatedChannelName);
            }
        }
    }
    xml += QString("    </midibinding>");
    return xml;
}
void MainWindow::saveEffect(QString& xml, effect* effect)
{
    xml += QString("    <effect name=\"%1\" filename=\"%2\" enabled=\"%3\" >").arg(effect->fx->getPluginLabel())
           .arg(effect->fx->getLibraryPath()).arg(fromBool(effect->fx->isEnabled()));
    foreach(LadspaControlPort* control, effect->fx->inputControlPorts) {
        xml += QString("      <attribute name=\"%1\" value=\"%2\" />").arg(control->m_sName).arg(control->m_fControlValue);
    }
    xml += "    </effect>";
}
void MainWindow::about()
{
    QMessageBox::about(this, trUtf8("LiveMix: About LiveMix"), trUtf8("<qt> \
                       <p>Maintainer <b>St&eacute;phane Brunner</b> &lt;stephane.brunner@gmail.com&gt;</p> \
                       <p>LiveMix is a mixer application for live performance using Jack (<a href=\"http://www.jackaudio.org/\">www.jackaudio.org</a>) and LADSPA (<a href=\"http://www.ladspa.org/\">www.ladspa.org</a>). \
                       Check out <a href=\"http://livemix.codingteam.net/\">livemix.codingteam.net</a> for more information and new versions of LiveMix.</p> \
                       <p>This application and all its components are licensed under the GPL.</p> \
                       </qt>"));
}
void MainWindow::aboutQt()
{
    QMessageBox::aboutQt(this, trUtf8("LiveMix: About Qt"));
}

void MainWindow::addInputMono()
{
    QString tmp = QInputDialog::getText(this, trUtf8("Mono in channel name"), trUtf8("Channel name"), QLineEdit::Normal, trUtf8("(empty)"));
    if (tmp != trUtf8("(empty)")) {
        addInput(tmp, false);
    }
}
void MainWindow::addInputStereo()
{
    QString tmp = QInputDialog::getText(this, trUtf8("Stereo in channel name"), trUtf8("Channel name"), QLineEdit::Normal, trUtf8("(empty)"));
    if (tmp != trUtf8("(empty)")) {
        addInput(tmp, true);
    }
}
void MainWindow::addInput(QString name, bool stereo)
{
    if (Backend::instance()->inchannels().contains(name)) {
        QMessageBox::critical(this, trUtf8("Unable to add input channel"), trUtf8("The name \"%1\" allready exists").arg(name));
    } else if (Backend::instance()->addInput(name, stereo)) {
        _mixerwidget->addinchannel(name);
    }
}
void MainWindow::multipleAddInputMono()
{
    bool ok;
    int nb = QInputDialog::getInteger(this, trUtf8("Mono in channel name"), trUtf8("Channel numbers"), 4, 0, 100, 1, &ok);
    if (ok) {
        int n = 1;
        for (int i = 0 ; i < nb ; i++) {
            while (Backend::instance()->inchannels().contains(QString("%1").arg(n))) {
                n++;
            }
            if (Backend::instance()->addInput(QString("%1").arg(n), false)) {
                _mixerwidget->addinchannel(QString("%1").arg(n));
            }
        }
    }
}
void MainWindow::multipleAddInputStereo()
{
    bool ok;
    int nb = QInputDialog::getInteger(this, trUtf8("Stereo in channel name"), trUtf8("Channel numbers"), 2, 0, 100, 1, &ok);
    if (ok) {
        int n = 1;
        while (Backend::instance()->inchannels().contains(QString("%1").arg(n))) {
            n += 2;
        }
        for (int i = 0 ; i < nb ; i++) {
            while (Backend::instance()->inchannels().contains(QString("%1-%2").arg(n).arg(n+1))) {
                n += 2;
            }
            if (Backend::instance()->addInput(QString("%1-%2").arg(n).arg(n+1), true)) {
                _mixerwidget->addinchannel(QString("%1-%2").arg(n).arg(n+1));
            }
        }
    }
}
void MainWindow::addPreMono()
{
    QString tmp = QInputDialog::getText(this, trUtf8("Mono pre channel name"), trUtf8("Channel name"), QLineEdit::Normal, trUtf8("(empty)"));
    if (tmp != trUtf8("(empty)")) {
        addPre(tmp, false);
    }
}
void MainWindow::addPreStereo()
{
    QString tmp = QInputDialog::getText(this, trUtf8("Stereo pre channel name"), trUtf8("Channel name"), QLineEdit::Normal, trUtf8("(empty)"));
    if (tmp != trUtf8("(empty)")) {
        addPre(tmp, true);
    }
}
void MainWindow::addPre(QString name, bool stereo)
{
    if (Backend::instance()->prechannels().contains(name)) {
        QMessageBox::critical(this, trUtf8("Unable to add pre-fader channel"), trUtf8("The name \"%1\" allready exists").arg(name));
    } else if (Backend::instance()->addPre(name, stereo)) {
        _mixerwidget->addprechannel(name);
    }
}
void MainWindow::addPostMonoExternal()
{
    QString tmp = QInputDialog::getText(this, trUtf8("External mono post channel name"), trUtf8("Channel short name"), QLineEdit::Normal, trUtf8("(empty)"));
    if (tmp != trUtf8("(empty)")) {
        addPost(tmp, false, true);
    }
}
void MainWindow::addPostStereoExternal()
{
    QString tmp = QInputDialog::getText(this, trUtf8("External stereo post channel name"), trUtf8("Channel short name"), QLineEdit::Normal, trUtf8("(empty)"));
    if (tmp != trUtf8("(empty)")) {
        addPost(tmp, true, true);
    }
}
void MainWindow::addPostMonoInternal()
{
    QString tmp = QInputDialog::getText(this, trUtf8("Internal mono post channel name"), trUtf8("Channel short name"), QLineEdit::Normal, trUtf8("(empty)"));
    if (tmp != trUtf8("(empty)")) {
        addPost(tmp, false, false);
    }
}
void MainWindow::addPostStereoInternal()
{
    QString tmp = QInputDialog::getText(this, trUtf8("Internal stereo post channel name"), trUtf8("Channel short name"), QLineEdit::Normal, trUtf8("(empty)"));
    if (tmp != trUtf8("(empty)")) {
        addPost(tmp, true, false);
    }
}
void MainWindow::addPost(QString name, bool stereo, bool external)
{
    if (Backend::instance()->postchannels().contains(name)) {
        QMessageBox::critical(this, trUtf8("Unable to add post-fader channel"), trUtf8("The name \"%1\" allready exists").arg(name));
    } else if (Backend::instance()->addPost(name, stereo, external)) {
        _mixerwidget->addpostchannel(name);
    }
}
void MainWindow::addSubMono()
{
    QString tmp = QInputDialog::getText(this, trUtf8("Mono sub channel name"), trUtf8("Channel name"), QLineEdit::Normal, trUtf8("(empty)"));
    if (tmp != trUtf8("(empty)"))
        addSub(tmp, false);
}
void MainWindow::addSubStereo()
{
    QString tmp = QInputDialog::getText(this, trUtf8("Stereo sub channel name"), trUtf8("Channel name"), QLineEdit::Normal, trUtf8("(empty)"));
    if (tmp != trUtf8("(empty)"))
        addSub(tmp, true);
}
void MainWindow::addSub(QString name, bool stereo)
{
    if (Backend::instance()->subchannels().contains(name)) {
        QMessageBox::critical(this, trUtf8("Unable to add sub-group channel"), trUtf8("The name \"%1\" allready exists").arg(name));
    } else if (Backend::instance()->addSub(name, stereo)) {
        _mixerwidget->addsubchannel(name);
    }
}

void MainWindow::removeInput()
{
//    qDebug( "MainWindow::removeInput()" );
    LiveMix::ChannelSelector *tmp = new LiveMix::ChannelSelector(trUtf8("Delete Input channels"), trUtf8("Select the input channels for deletion:"), Backend::instance()->inchannels(), this);
    connect(tmp, SIGNAL(selectedChannel(QString)), this, SLOT(removeInput(QString)));
    tmp->show();
}
void MainWindow::removeInput(QString n)
{
//    qDebug( "MainWindow::removeInput( QString %s )", qPrintable( n ) );
    if (Backend::instance()->inchannels().contains(n)) {
        foreach(effect* fx, *Backend::instance()->getInEffects(n)) {
            if (fx->gui) {
                _mixerwidget->removeFX(fx->gui, fx);
            }
        }
        if (Backend::instance()->removeInput(n)) {
            _mixerwidget->removeinchannel(n);
        }
    }
}

void MainWindow::removePre()
{
//    qDebug( "MainWindow::removePre()" );
    LiveMix::ChannelSelector *tmp = new LiveMix::ChannelSelector(trUtf8("Delete Pre channels"), trUtf8("Select the pre channels for deletion:"), Backend::instance()->prechannels(), this);
    connect(tmp, SIGNAL(selectedChannel(QString)), this, SLOT(removePre(QString)));
    tmp->show();
}
void MainWindow::removePre(QString n)
{
//    qDebug( "MainWindow::removePre( QString %s )", qPrintable( n ) );
    if (Backend::instance()->prechannels().contains(n)) {
        foreach(effect* fx, *Backend::instance()->getPreEffects(n)) {
            if (fx->gui) {
                _mixerwidget->removeFX(fx->gui, fx);
            }
        }
        if (Backend::instance()->removePre(n)) {
            _mixerwidget->removeprechannel(n);
        }
    }
}

void MainWindow::removePost()
{
//    qDebug( "MainWindow::removePost()" );
    LiveMix::ChannelSelector *tmp = new LiveMix::ChannelSelector(trUtf8("Delete Post channels"), trUtf8("Select the post channels for deletion:"), Backend::instance()->postchannels(), this);
    connect(tmp, SIGNAL(selectedChannel(QString)), this, SLOT(removePost(QString)));
    tmp->show();
}
void MainWindow::removePost(QString n)
{
//    qDebug( "MainWindow::removePost( QString %s )", qPrintable( n ) );
    if (Backend::instance()->postchannels().contains(n)) {
        foreach(effect* fx, *Backend::instance()->getPostEffects(n)) {
            if (fx->gui) {
                _mixerwidget->removeFX(fx->gui, fx);
            }
        }
        if (Backend::instance()->removePost(n)) {
            _mixerwidget->removepostchannel(n);
        }
    }
}

void MainWindow::removeSub()
{
//    qDebug( "MainWindow::removeSub()" );
    LiveMix::ChannelSelector *tmp = new LiveMix::ChannelSelector(trUtf8("Delete Sub channels"), trUtf8("Select the sub channels for deletion:"), Backend::instance()->subchannels(), this);
    connect(tmp, SIGNAL(selectedChannel(QString)), this, SLOT(removeSub(QString)));
    tmp->show();
}
void MainWindow::removeSub(QString n)
{
//    qDebug( "MainWindow::removeSub( QString %s )", qPrintable( n ) );
    if (Backend::instance()->subchannels().contains(n)) {
        foreach(effect* fx, *Backend::instance()->getSubEffects(n)) {
            if (fx->gui) {
                _mixerwidget->removeFX(fx->gui, fx);
            }
        }
        if (Backend::instance()->removeSub(n)) {
            _mixerwidget->removesubchannel(n);
        }
    }
}

void MainWindow::initMatrix()
{
    _mixerwidget->init();
    _initScheduled = false;
    _mixerwidget->showMessage(trUtf8("LiveMix started."));
    show();
}
void MainWindow::scheduleInit()
{
    if (!_initScheduled) {
        QTimer::singleShot(1, this, SLOT(initMatrix()));
        _initScheduled = true;
    }
}

}
; //LiveMix
