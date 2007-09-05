/*
   Copyright 2004 - 2007 Arnold Krille <arnold@arnoldarts.de>
   Copyright 2007 Stéphane Brunner <stephane.brunner@gmail.com>
 
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation;
   version 2 of the License.
 
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
 
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "mainwindow.h"
//#include "mainwindow.moc"

#include "mixingmatrix.h"
#include "channelselector.h"
#include "graphicalguiserver.h"

#include <QtCore/QDebug>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QLayout>
#include <QtGui/QInputDialog>
#include <QtCore/QSettings>
#include <QtGui/QFileDialog>
#include <QtCore/QFile>
#include <QtGui/QMessageBox>
#include <QtGui/QAction>
#include <QtCore/QTimer>
#include <QtGui/QCloseEvent>
#include <QtGui/QStatusBar>

#include <QtXml/QDomDocument>

using namespace JackMix;

MainWindow::MainWindow( QWidget* p ) : QMainWindow( p ), _initScheduled( true )
{
    qDebug() << "MainWindow::MainWindow()";
    Backend::init(new GraphicalGuiServer(this));
    init();
    QStringList ins = QStringList() << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8";
// QStringList pres = QStringList() << "pre1" << "pre2";
// QStringList posts = QStringList() << "post1" << "post2";
// QStringList subs = QStringList() << "sub1" << "sub2";

    foreach( QString in, ins ) {
        Backend::instance()->addInput( in, false );
    }
    Backend::instance()->addInput( "9-10", true );
    Backend::instance()->addInput( "11-12", true );
    Backend::instance()->addPre( "pre1", true );
    Backend::instance()->addPre( "pre2", false );
    Backend::instance()->addPost( "post1", true, true );
    Backend::instance()->addPost( "post2", false, true );
    Backend::instance()->addSub( "sub1", true );
    Backend::instance()->addSub( "sub2", false );
    /* foreach( QString pre, pres ) {
      Backend::instance()->addPre( pre, false );
     }
     foreach( QString post, posts ) {
      Backend::instance()->addPost( post, false, true );
     }
     foreach( QString sub, subs ) {
      Backend::instance()->addSub( sub, false );
     }*/

    _initScheduled = false;
    scheduleInit();

    qDebug() << "MainWindow::MainWindow() finished...";
}
MainWindow::MainWindow( QString filename, QWidget* p ) : QMainWindow( p ), _initScheduled( true )
{
    qDebug() << "MainWindow::MainWindow(" << filename << "," << p << ")";
    Backend::init(new GraphicalGuiServer(this));
    init();

    openFile( filename );

    QStringList ins = Backend::instance()->inchannels();
    QStringList outs = Backend::instance()->outchannels();
    if ( ins.empty() || outs.empty() )
        statusBar()->showMessage(trUtf8("No Channels available :-("));

    _initScheduled = false;
    scheduleInit();

    qDebug() << "MainWindow::MainWindow() finished...";
}

void MainWindow::init()
{

    _filemenu = menuBar()->addMenu(trUtf8("&File"));
    _filemenu->addAction(trUtf8("Open File..."), this, SLOT( openFile() ), Qt::CTRL+Qt::Key_O );
    _filemenu->addAction(trUtf8("Save File..."), this, SLOT( saveFile() ), Qt::CTRL+Qt::Key_S );
    _filemenu->addSeparator();
    _filemenu->addAction(trUtf8("&Quit"), this, SLOT( close() ), Qt::CTRL+Qt::Key_Q );

    _editmenu = menuBar()->addMenu(trUtf8("&Edit"));
// _select_action = new QAction( "Select Mode"), this );
// _select_action->setCheckable( true );
// connect( _select_action, SIGNAL( triggered() ), this, SLOT( toggleselectmode() ) );
    //_editmenu->addAction( _select_action );
    //_select_action->addTo( new QToolBar( this ) );
// _editmenu->addAction(trUtf8("&Fill empty spaces"), this, SLOT( scheduleInit() ) );
// _editmenu->addSeparator();
    QMenu* editInput = _editmenu->addMenu(trUtf8("Add &Input"));
    _add_inchannel_action = new QAction(trUtf8("&Mono..."), this );
    connect( _add_inchannel_action, SIGNAL( triggered() ), this, SLOT( addInputMono() ) );
    editInput->addAction( _add_inchannel_action );
    _add_stinchannel_action = new QAction(trUtf8("&Stereo ..."), this );
    connect( _add_stinchannel_action, SIGNAL( triggered() ), this, SLOT( addInputStereo() ) );
    editInput->addAction( _add_stinchannel_action );
    QMenu* editPre = _editmenu->addMenu(trUtf8("Add &Pre"));
    _add_prechannel_action = new QAction(trUtf8("&Mono..."), this );
    connect( _add_prechannel_action, SIGNAL( triggered() ), this, SLOT( addPreMono() ) );
    editPre->addAction( _add_prechannel_action );
    _add_stprechannel_action = new QAction(trUtf8("&Stereo..."), this );
    connect( _add_stprechannel_action, SIGNAL( triggered() ), this, SLOT( addPreStereo() ) );
    editPre->addAction( _add_stprechannel_action );
    QMenu* editPost = _editmenu->addMenu(trUtf8("Add P&ost"));
    _add_postchannel_action = new QAction(trUtf8("&External mono..."), this );
    connect( _add_postchannel_action, SIGNAL( triggered() ), this, SLOT( addPostMonoExternal() ) );
    editPost->addAction( _add_postchannel_action );
    _add_stpostchannel_action = new QAction(trUtf8("External stereo..."), this );
    connect( _add_stpostchannel_action, SIGNAL( triggered() ), this, SLOT( addPostStereoExternal() ) );
    editPost->addAction( _add_stpostchannel_action );
    _add_intpostchannel_action = new QAction(trUtf8("Internal &mono..."), this );
    connect( _add_intpostchannel_action, SIGNAL( triggered() ), this, SLOT( addPostMonoInternal() ) );
    editPost->addAction( _add_intpostchannel_action );
    _add_stintpostchannel_action = new QAction(trUtf8("Internal &stereo..."), this );
    connect( _add_stintpostchannel_action, SIGNAL( triggered() ), this, SLOT( addPostStereoInternal() ) );
    editPost->addAction( _add_stintpostchannel_action );
    QMenu* editSub = _editmenu->addMenu(trUtf8("Add &Sub"));
    _add_subchannel_action = new QAction(trUtf8("&Mono..."), this );
    connect( _add_subchannel_action, SIGNAL( triggered() ), this, SLOT( addSubMono() ) );
    editSub->addAction( _add_subchannel_action );
    _add_stsubchannel_action = new QAction(trUtf8("&Stereo..."), this );
    connect( _add_stsubchannel_action, SIGNAL( triggered() ), this, SLOT( addSubStereo() ) );
    editSub->addAction( _add_stsubchannel_action );
    QMenu* editRemove = _editmenu->addMenu(trUtf8("&Remove"));
    _remove_inchannel_action = new QAction(trUtf8("&Input..."), this );
    connect( _remove_inchannel_action, SIGNAL( triggered() ), this, SLOT( removeInput() ) );
    editRemove->addAction( _remove_inchannel_action );
    _remove_prechannel_action = new QAction(trUtf8("&Pre..."), this );
    connect( _remove_prechannel_action, SIGNAL( triggered() ), this, SLOT( removePre() ) );
    editRemove->addAction( _remove_prechannel_action );
    _remove_postchannel_action = new QAction(trUtf8("P&ost..."), this );
    connect( _remove_postchannel_action, SIGNAL( triggered() ), this, SLOT( removePost() ) );
    editRemove->addAction( _remove_postchannel_action );
    _remove_subchannel_action = new QAction(trUtf8("&Sub..."), this );
    connect( _remove_subchannel_action, SIGNAL( triggered() ), this, SLOT( removeSub() ) );
    editRemove->addAction( _remove_subchannel_action );

    _helpmenu = menuBar()->addMenu(trUtf8("&Help"));
    _helpmenu->addAction(trUtf8("About &JackMix"), this, SLOT( about() ) );
    _helpmenu->addAction(trUtf8("About &Qt"), this, SLOT( aboutQt() ) );

    _mixerwidget = new Widget(this);
    setCentralWidget( _mixerwidget );
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

void MainWindow::closeEvent( QCloseEvent* e )
{
    qDebug() << "MainWindow::closeEvent( QCloseEvent " << e << " )";
    e->accept();
}

void MainWindow::openFile()
{
    QString path = QFileDialog::getOpenFileName( this, 0, 0, trUtf8("LiveMix (*.lm)"));
    openFile( path );
}

bool MainWindow::toBool( QString value )
{
    bool result = false;
    result |= value.compare( "yes", Qt::CaseInsensitive ) == 0;
    result |= value.compare( "on", Qt::CaseInsensitive ) == 0;
    result |= value.compare( "true", Qt::CaseInsensitive ) == 0;
    return result;
}
QString MainWindow::fromBool( bool value )
{
    return value ? "yes" : "no";
}
void MainWindow::openFile( QString path )
{
    //qDebug() << "MainWindow::openFile(" << path << ")";
    Backend::instance()->run(false);
    if ( path.isEmpty() ) {
        return;
    }

    // delay autofill at least until all saved elements are created:
    bool save_initScheduled = _initScheduled;
    _initScheduled = true;

    QFile file( path );
    if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
        while ( Backend::instance()->inchannels().size() > 0 ) {
            removeInput( Backend::instance()->inchannels()[ 0 ] );
        }
        while ( Backend::instance()->prechannels().size() > 0 ) {
            removePre( Backend::instance()->prechannels()[ 0 ] );
        }
        while ( Backend::instance()->postchannels().size() > 0 ) {
            removePost( Backend::instance()->postchannels()[ 0 ] );
        }
        while ( Backend::instance()->subchannels().size() > 0 ) {
            removeSub( Backend::instance()->subchannels()[ 0 ] );
        }
        Backend::instance()->getOutEffects(MAIN)->clear();

        QDomDocument doc( "bla" );
        doc.setContent( &file );

        QDomElement jackmix = doc.documentElement();
        QString version = jackmix.attribute( "version", "0.4" );

        if ( version == "0.4" ) {

            for ( QDomElement in = jackmix.firstChildElement( "in" ); !in.isNull(); in = in.nextSiblingElement( "in" ) ) {
                QString name = in.attribute( "name" );
                Backend::instance()->addInput( name, toBool(in.attribute( "stereo" ) ) );
                Backend::instance()->setInGain( name, in.attribute( "gain" ).toFloat() );
                Backend::instance()->setInVolume( name, in.attribute( "volume" ).toFloat() );
                Backend::instance()->setInBal( name, in.attribute( "bal" ).toFloat() );
                Backend::instance()->setInMute( name, toBool( in.attribute( "mute" ) ) );
                Backend::instance()->setInPlf( name, toBool( in.attribute( "plf" ) ) );
                Backend::instance()->setInMain( name, toBool( in.attribute( "main" ) ) );
                for ( QDomElement pre = in.firstChildElement( "pre" ); !pre.isNull(); pre = pre.nextSiblingElement( "pre" ) ) {
                    Backend::instance()->setInPreVolume( name, pre.attribute( "name" ), pre.attribute( "volume" ).toFloat() );
                }
                for ( QDomElement post = in.firstChildElement( "post" ); !post.isNull(); post = post.nextSiblingElement( "post" ) ) {
                    Backend::instance()->setInPostVolume( name, post.attribute( "name" ), post.attribute( "volume" ).toFloat() );
                }
                for ( QDomElement sub = in.firstChildElement( "sub" ); !sub.isNull(); sub = sub.nextSiblingElement( "sub" ) ) {
                    Backend::instance()->setInSub( name, sub.attribute( "name" ), toBool( sub.attribute( "mute" ) ) );
                }

                for ( QDomElement effect = in.firstChildElement( "effect" ); !effect.isNull(); effect = effect.nextSiblingElement( "effect" ) ) {
                    Backend::instance()->addInEffect( name, openEffect(effect) );
                }
            }

            QDomElement out = jackmix.firstChildElement( "out" );
            qDebug()<<out.isNull();
            if (!out.isNull()) {
                Backend::instance()->setOutVolume( MAIN, out.attribute( "volume" ).toDouble() );
                Backend::instance()->setOutMute( MAIN, toBool( out.attribute( "mute" ) ) );
                Backend::instance()->setOutAlf( MAIN, toBool( out.attribute( "alf" ) ) );
                Backend::instance()->setOutBal( MAIN, out.attribute( "bal" ).toDouble() );
                Backend::instance()->setOutVolume( MONO, out.attribute( "monovolume" ).toDouble() );
                Backend::instance()->setOutVolume( PLF, out.attribute( "phonevolume" ).toDouble() );

                for ( QDomElement effect = out.firstChildElement( "effect" ); !effect.isNull(); effect = effect.nextSiblingElement( "effect" ) ) {
                    Backend::instance()->addOutEffect( MAIN, openEffect(effect) );
                }
            }

            for ( QDomElement pre = jackmix.firstChildElement( "pre" ); !pre.isNull(); pre = pre.nextSiblingElement( "pre" ) ) {
                QString name = pre.attribute( "name" );
                Backend::instance()->addPre( name, toBool( pre.attribute( "stereo" ) ) );
                Backend::instance()->setPreVolume( name, pre.attribute( "volume" ).toDouble() );
                Backend::instance()->setPreMute( name, toBool( pre.attribute( "mute" ) ) );
                Backend::instance()->setPreAlf( name, toBool( pre.attribute( "alf" ) ) );

                for ( QDomElement effect = pre.firstChildElement( "effect" ); !effect.isNull(); effect = effect.nextSiblingElement( "effect" ) ) {
                    Backend::instance()->addPreEffect( name, openEffect(effect) );
                }
            }

            for ( QDomElement post = jackmix.firstChildElement( "post" ); !post.isNull(); post = post.nextSiblingElement( "post" ) ) {
                QString name = post.attribute( "name" );
                Backend::instance()->addPost( name, toBool( post.attribute( "stereo" ) ), toBool( post.attribute( "external" ) ) );
                Backend::instance()->setPostPreVolume( name, post.attribute( "pre-volume" ).toDouble() );
                Backend::instance()->setPostPostVolume( name, post.attribute( "post-volume" ).toDouble() );
                Backend::instance()->setPostMute( name, toBool( post.attribute( "mute" ) ) );
                Backend::instance()->setPostBal( name, post.attribute( "bal" ).toFloat() );
                Backend::instance()->setPostAlf( name, toBool( post.attribute( "alf" ) ) );
                Backend::instance()->setPostPlf( name, toBool( post.attribute( "plf" ) ) );
                Backend::instance()->setPostMain( name, toBool( post.attribute( "main" ) ) );

                for ( QDomElement sub = post.firstChildElement( "sub" ); !sub.isNull(); sub = sub.nextSiblingElement( "sub" ) ) {
                    Backend::instance()->setPostSub( name, sub.attribute( "name" ), toBool( sub.attribute( "mute" ) ) );
                }

                for ( QDomElement effect = post.firstChildElement( "effect" ); !effect.isNull(); effect = effect.nextSiblingElement( "effect" ) ) {
                    Backend::instance()->addPostEffect( name, openEffect(effect) );
                }
            }

            for ( QDomElement sub = jackmix.firstChildElement( "sub" ); !sub.isNull(); sub = sub.nextSiblingElement( "sub" ) ) {
                QString name = sub.attribute( "name" );
                Backend::instance()->addSub( name, toBool( sub.attribute( "stereo" ) ) );
                Backend::instance()->setSubVolume( name, sub.attribute( "volume" ).toDouble() );
                Backend::instance()->setSubMute( name, toBool( sub.attribute( "mute" ) ) );
                Backend::instance()->setSubAlf( name, toBool( sub.attribute( "alf" ) ) );
                Backend::instance()->setSubMain( name, toBool( sub.attribute( "main" ) ) );
                Backend::instance()->setSubBal( name, sub.attribute( "bal" ).toFloat() );

                for ( QDomElement effect = sub.firstChildElement( "effect" ); !effect.isNull(); effect = effect.nextSiblingElement( "effect" ) ) {
                    Backend::instance()->addSubEffect( name, openEffect(effect) );
                }
            }
        }

        file.close();
    }

    _initScheduled = save_initScheduled;
    scheduleInit();
    Backend::instance()->run(true);
    //qDebug() << "MainWindow::openFile() finished";
}
void MainWindow::saveFile()
{
    QString path = QFileDialog::getSaveFileName( this, 0, 0, trUtf8("LiveMix (*.lm)"));
    if ( path.isEmpty() )
        return;

    if ( ! path.endsWith( ".lm" ) )
        path += ".lm";

    QString xml = "<livemix versimute=\"0.5\">";
    foreach( QString name, Backend::instance()->inchannels() ) {
        const struct in* elem = Backend::instance()->getInput( name );
        xml += QString( "  <in name=\"%1\" gain=\"%2\" volume=\"%3\" mute=\"%4\" plf=\"%5\" bal=\"%6\" stereo=\"%7\" main=\"%8\">" )
               .arg( name ).arg( elem->gain ).arg( elem->volume ).arg( fromBool( elem->mute ) ).arg( fromBool( elem->plf ) )
               .arg( elem->bal ).arg( fromBool( elem->stereo ) ).arg( fromBool( elem->main ) );

        foreach( QString pre, Backend::instance()->prechannels() ) {
            xml += QString( "    <pre name=\"%1\" volume=\"%2\" />" ).arg( pre ).arg( elem->pre[ pre ] );
        }
        foreach( QString post, Backend::instance()->postchannels() ) {
            xml += QString( "    <post name=\"%1\" volume=\"%2\" />" ).arg( post ).arg( elem->post[ post ] );
        }
        foreach( QString sub, Backend::instance()->subchannels() ) {
            xml += QString( "    <sub name=\"%1\" mute=\"%2\" />" ).arg( sub ).arg( fromBool( elem->sub[ sub ] ) );
        }

        for (QList<struct effect *>::const_iterator effect = elem->effects.begin();
                effect != elem->effects.end();
                ++effect) {
            saveEffect(xml, *effect);
        }
        xml += "  </in>";
    }

    {
        const struct out* elem = Backend::instance()->getOutput( MAIN );
        xml += QString( "  <out volume=\"%1\" mute=\"%2\" alf=\"%3\" bal=\"%4\" monovolume=\"%5\" phonevolume=\"%6\">" )
               .arg( elem->volume ).arg( fromBool( elem->mute ) ).arg( fromBool( elem->alf ) ).arg( elem->bal )
               .arg( Backend::instance()->getOutput( MONO )->volume ).arg( Backend::instance()->getOutput( PLF )->volume );

        for (QList<struct effect *>::const_iterator effect = elem->effects.begin();
                effect != elem->effects.end();
                ++effect) {
            saveEffect(xml, *effect);
        }
        xml += "  </out>";
    }

    foreach( QString name, Backend::instance()->prechannels() ) {
        const struct pre* elem = Backend::instance()->getPre( name );
        xml += QString( "  <pre name=\"%1\" volume=\"%2\" mute=\"%3\" alf=\"%4\" stereo=\"%5\">" )
               .arg( name ).arg( elem->volume ).arg( fromBool( elem->mute ) ).arg( fromBool( elem->alf ) ).arg( fromBool( elem->stereo ) );

        for (QList<struct effect *>::const_iterator effect = elem->effects.begin();
                effect != elem->effects.end();
                ++effect) {
            saveEffect(xml, *effect);
        }
        xml += "  </pre>";
    }

    foreach( QString name, Backend::instance()->postchannels() ) {
        const struct post* elem = Backend::instance()->getPost( name );
        xml += QString( "  <post name=\"%1\" pre-volume=\"%2\" post-volume=\"%3\" mute=\"%4\" alf=\"%5\" stereo=\"%6\" main=\"%7\" bal=\"%8\" external=\"%9\" plf=\"%10\">" )
               .arg( name ).arg( elem->prevolume ).arg( elem->postvolume ).arg( fromBool( elem->mute ) ).arg( fromBool( elem->alf ) )
               .arg( fromBool( elem->stereo ) ).arg( fromBool( elem->main ) ).arg( elem->bal ).arg( fromBool( elem->external ) ).arg( fromBool( elem->plf ) );

        foreach( QString sub, Backend::instance()->subchannels() ) {
            xml += QString( "    <sub name=\"%1\" mute=\"%2\" />" ).arg( sub ).arg( fromBool( elem->sub[ sub ] ) );
        }

        for (QList<struct effect *>::const_iterator effect = elem->effects.begin();
                effect != elem->effects.end();
                ++effect) {
            saveEffect(xml, *effect);
        }
        xml += "  </post>";
    }

    foreach( QString name, Backend::instance()->subchannels() ) {
        const struct sub* elem = Backend::instance()->getSub( name );
        xml += QString( "  <sub name=\"%1\" volume=\"%2\" mute=\"%3\" alf=\"%4\" stereo=\"%5\" main=\"%6\" bal=\"%7\">" )
               .arg( name ).arg( elem->volume ).arg( fromBool( elem->mute ) ).arg( fromBool( elem->alf ) ).arg( fromBool( elem->stereo ) )
               .arg( fromBool( elem->main ) ).arg( elem->bal );

        for (QList<struct effect *>::const_iterator effect = elem->effects.begin();
                effect != elem->effects.end();
                ++effect) {
            saveEffect(xml, *effect);
        }
        xml += "  </sub>";
    }

    xml += "</livemix>";

    QFile file( path );
    if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
        QTextStream stream( &file );
        stream << xml.replace( ">", ">\n" );
        file.close();
    }

}

/*
    LadspaFX* pFX = LadspaFX::load( sFilename, sName, 44100 );
    Effects::getInstance()->setLadspaFX( pFX, nFX );
    if (pFX) {
     pFX->setEnabled( bEnabled );
     pFX->setVolume( fVolume );
     TiXmlNode* inputControlNode;
 
  string sName = LocalFileMng::readXmlString( inputControlNode, "name", "" );
  float fValue = LocalFileMng::readXmlFloat( inputControlNode, "value", 0.0 );
 
  for (unsigned nPort = 0; nPort < pFX->inputControlPorts.size(); nPort++) {
   LadspaControlPort *port = pFX->inputControlPorts[ nPort ];
   if ( string(port->sName) == sName) {
    port->fControlValue = fValue;
   }
  }
*/
LadspaFX* MainWindow::openEffect(QDomElement effect)
{
// int pos = 0;
// for ( QDomElement effect = parent.firstChildElement( "effect" ); !effect.isNull(); effect = effect.nextSiblingElement( "effect" ) ) {
    LadspaFX* pFX = LadspaFX::load( effect.attribute( "filename" ), effect.attribute( "name" ), 44100 );
    pFX->setEnabled( toBool(effect.attribute( "enabled" )) );
//  Backend::instance()->setSubEffectMute( name, pos, toBool( effect.attribute( "mute" ) ) );
    for ( QDomElement attr = effect.firstChildElement( "attribute" ); !attr.isNull(); attr = attr.nextSiblingElement( "attribute" ) ) {
        QString sName = attr.attribute( "name" );
        float fValue = attr.attribute( "value" ).toFloat();

        for (int nPort = 0; nPort < pFX->inputControlPorts.size(); nPort++) {
            LadspaControlPort *port = pFX->inputControlPorts[ nPort ];
            if ( port->m_sName == sName) {
                port->m_fControlValue = fValue;
            }
        }
    }
//  pos++;
// }
    return pFX;
}

/*
        <fx>
            <name>alias</name>
            <filename>/usr/lib/ladspa/alias_1407.so</filename>
            <enabled>true</enabled>
            <inputControlPort>
                <name>Aliasing level</name>
                <value>0.275862</value>
            </inputControlPort>
        </fx>
*/
void MainWindow::saveEffect(QString& xml, struct effect* effect)
{
    xml += QString( "    <effect name=\"%1\" filename=\"%2\" enabled=\"%3\" >" ).arg( effect->fx->getPluginLabel() )
           .arg( effect->fx->getLibraryPath() ).arg( fromBool(effect->fx->isEnabled()));
    foreach (LadspaControlPort* control, effect->fx->inputControlPorts)
    {
        xml += QString( "      <attribute name=\"%1\" value=\"%2\" />" ).arg( control->m_sName ).arg( control->m_fControlValue );
    }
    xml += "    </effect>";
}
void MainWindow::about()
{
    QMessageBox::about( this, "LiveMix: About LiveMix", "<qt> \
                        <p>Maintainer <b>Stéphane Brunner</b> &lt;stephane.brunner@gmail.com&gt;</p> \
                        <p>LiveMix is a mixer application for live performance using Jack (<a href=\"http://www.jackaudio.org/\">www.jackaudio.org</a>) and LADSPA (<a href=\"http://www.ladspa.org/\">www.ladspa.org</a>). \
                        Check out <a href=\"http://livemix.codingteam.net/\">livemix.codingteam.net</a> for more information and new versions of LiveMix.</p> \
                        <p>This application and all its components are licensed under the GPL.</p> \
                        </qt>" );
}
void MainWindow::aboutQt()
{
    QMessageBox::aboutQt( this, trUtf8("LiveMix: About Qt"));
}

void MainWindow::addInputMono()
{
    QString tmp = QInputDialog::getText( this, trUtf8("Mono in channel name"), trUtf8("Channel name"), QLineEdit::Normal, trUtf8("(empty)") );
    if ( tmp != trUtf8("(empty)"))
        addInput( tmp, false );
}
void MainWindow::addInputStereo()
{
    QString tmp = QInputDialog::getText( this, trUtf8("Stereo in channel name"), trUtf8("Channel name"), QLineEdit::Normal, trUtf8("(empty)") );
    if ( tmp != trUtf8("(empty)"))
        addInput( tmp, false );
}
void MainWindow::addInput( QString name, bool stereo )
{
    if ( Backend::instance()->addInput( name, stereo ) ) {
        _mixerwidget->addinchannel( name );
    }
}
void MainWindow::addPreMono()
{
    QString tmp = QInputDialog::getText( this, trUtf8("Mono pre channel name"), trUtf8("Channel name"), QLineEdit::Normal, trUtf8("(empty)") );
    if ( tmp != trUtf8("(empty)"))
        addPre( tmp, false );
}
void MainWindow::addPreStereo()
{
    QString tmp = QInputDialog::getText( this, trUtf8("Stereo pre channel name"), trUtf8("Channel name"), QLineEdit::Normal, trUtf8("(empty)") );
    if ( tmp != trUtf8("(empty)"))
        addPre( tmp, false );
}
void MainWindow::addPre( QString name, bool stereo )
{
    if ( Backend::instance()->addPre( name, stereo ) ) {
        _mixerwidget->addprechannel( name );
    }
}
void MainWindow::addPostMonoExternal()
{
    QString tmp = QInputDialog::getText( this, trUtf8("External mono post channel name"), trUtf8("Channel name"), QLineEdit::Normal, trUtf8("(empty)"));
    if ( tmp != trUtf8("(empty)"))
        addPost( tmp, false, true );
}
void MainWindow::addPostStereoExternal()
{
    QString tmp = QInputDialog::getText( this, trUtf8("External stereo post channel name"), trUtf8("Channel name"), QLineEdit::Normal, trUtf8("(empty)"));
    if ( tmp != trUtf8("(empty)"))
        addPost( tmp, false, true );
}
void MainWindow::addPostMonoInternal()
{
    QString tmp = QInputDialog::getText( this, trUtf8("Internal mono post channel name"), trUtf8("Channel name"), QLineEdit::Normal, trUtf8("(empty)"));
    if ( tmp != trUtf8("(empty)"))
        addPost( tmp, false, false );
}
void MainWindow::addPostStereoInternal()
{
    QString tmp = QInputDialog::getText( this, trUtf8("Internal stereo post channel name"), trUtf8("Channel name"), QLineEdit::Normal, trUtf8("(empty)"));
    if ( tmp != trUtf8("(empty)"))
        addPost( tmp, false, false );
}
void MainWindow::addPost( QString name, bool stereo, bool external )
{
    if ( Backend::instance()->addPost( name, stereo, external ) ) {
        _mixerwidget->addpostchannel( name );
    }
}
void MainWindow::addSubMono()
{
    QString tmp = QInputDialog::getText( this, trUtf8("Mono sub channel name"), trUtf8("Channel name"), QLineEdit::Normal, trUtf8("(empty)") );
    if ( tmp != trUtf8("(empty)"))
        addSub( tmp, false );
}
void MainWindow::addSubStereo()
{
    QString tmp = QInputDialog::getText( this, trUtf8("Stereo sub channel name"), trUtf8("Channel name"), QLineEdit::Normal, trUtf8("(empty)") );
    if ( tmp != trUtf8("(empty)"))
        addSub( tmp, false );
}
void MainWindow::addSub( QString name, bool stereo )
{
    if ( Backend::instance()->addSub( name, stereo ) ) {
        _mixerwidget->addsubchannel( name );
    }
}

void MainWindow::removeInput()
{
    qDebug( "MainWindow::removeInput()" );
    JackMix::ChannelSelector *tmp = new JackMix::ChannelSelector( trUtf8("Delete Input channels"), trUtf8("Select the input channels for deletion:"), Backend::instance()->inchannels(), this );
    connect( tmp, SIGNAL( selectedChannel( QString ) ), this, SLOT( removeInput( QString ) ) );
    tmp->show();
}
void MainWindow::removeInput( QString n )
{
    qDebug( "MainWindow::removeInput( QString %s )", qPrintable( n ) );
    foreach(effect* fx, *Backend::instance()->getInEffects(n)) {
        if (fx->gui) {
            _mixerwidget->removeFX(fx->gui, fx);
        }
    }
    if ( Backend::instance()->removeInput( n ) ) {
        _mixerwidget->removeinchannel( n );
    }
}

void MainWindow::removePre()
{
    qDebug( "MainWindow::removePre()" );
    JackMix::ChannelSelector *tmp = new JackMix::ChannelSelector( trUtf8("Delete Pre channels"), trUtf8("Select the pre channels for deletion:"), Backend::instance()->prechannels(), this );
    connect( tmp, SIGNAL( selectedChannel( QString ) ), this, SLOT( removePre( QString ) ) );
    tmp->show();
}
void MainWindow::removePre( QString n )
{
    qDebug( "MainWindow::removePre( QString %s )", qPrintable( n ) );
    foreach(effect* fx, *Backend::instance()->getInEffects(n)) {
        if (fx->gui) {
            _mixerwidget->removeFX(fx->gui, fx);
        }
    }
    if ( Backend::instance()->removePre( n ) ) {
        _mixerwidget->removeprechannel( n );
    }
}

void MainWindow::removePost()
{
    qDebug( "MainWindow::removePost()" );
    JackMix::ChannelSelector *tmp = new JackMix::ChannelSelector( trUtf8("Delete Post channels"), trUtf8("Select the post channels for deletion:"), Backend::instance()->postchannels(), this );
    connect( tmp, SIGNAL( selectedChannel( QString ) ), this, SLOT( removePost( QString ) ) );
    tmp->show();
}
void MainWindow::removePost( QString n )
{
    qDebug( "MainWindow::removePost( QString %s )", qPrintable( n ) );
    foreach(effect* fx, *Backend::instance()->getInEffects(n)) {
        if (fx->gui) {
            _mixerwidget->removeFX(fx->gui, fx);
        }
    }
    if ( Backend::instance()->removePost( n ) ) {
        _mixerwidget->removepostchannel( n );
    }
}

void MainWindow::removeSub()
{
    qDebug( "MainWindow::removeSub()" );
    JackMix::ChannelSelector *tmp = new JackMix::ChannelSelector( trUtf8("Delete Sub channels"), trUtf8("Select the sub channels for deletion:"), Backend::instance()->subchannels(), this );
    connect( tmp, SIGNAL( selectedChannel( QString ) ), this, SLOT( removeSub( QString ) ) );
    tmp->show();
}
void MainWindow::removeSub( QString n )
{
    qDebug( "MainWindow::removeSub( QString %s )", qPrintable( n ) );
    foreach(effect* fx, *Backend::instance()->getInEffects(n)) {
        if (fx->gui) {
            _mixerwidget->removeFX(fx->gui, fx);
        }
    }
    if ( Backend::instance()->removeSub( n ) ) {
        _mixerwidget->removesubchannel( n );
    }
}

void MainWindow::initMatrix()
{
    _mixerwidget->init();
    _initScheduled = false;
    _mixerwidget->showMessage(trUtf8("LiveMix started."));
}
void MainWindow::scheduleInit()
{
    if ( !_initScheduled ) {
        QTimer::singleShot( 1, this, SLOT( initMatrix() ) );
        _initScheduled = true;
    }
}
