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
    the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
    MA 02110-1301, USA.
*/

#include <QDebug>
#include <QFile>
#include <QDir>
#include <QTranslator>
#include <QLocale>
#include <QApplication>
#include <QIcon>
#include <QPushButton>

#include "mainwindow.h"

//
// Set the palette used in the application
//
void setPalette( QApplication *pQApp )
{
    // create the default palette
    QPalette defaultPalette;

    // A general background color.
    defaultPalette.setColor( QPalette::Background, QColor( 58, 62, 72 ) );

    // A general foreground color.
    defaultPalette.setColor( QPalette::Foreground, QColor( 255, 255, 255 ) );

    // Used as the background color for text entry widgets; usually white or another light color.
    defaultPalette.setColor( QPalette::Base, QColor( 88, 94, 112 ) );

    // Used as the alternate background color in views with alternating row colors
    defaultPalette.setColor( QPalette::AlternateBase, QColor( 138, 144, 162 ) );

    // The foreground color used with Base. This is usually the same as the Foreground, in which case it must provide good contrast with Background and Base.
    defaultPalette.setColor( QPalette::Text, QColor( 255, 255, 255 ) );

    // The general button background color. This background can be different from Background as some styles require a different background color for buttons.
    defaultPalette.setColor( QPalette::Button, QColor( 88, 94, 112 ) );

    // A foreground color used with the Button color.
    defaultPalette.setColor( QPalette::ButtonText, QColor( 255, 255, 255 ) );


    // Lighter than Button color.
    defaultPalette.setColor( QPalette::Light, QColor( 138, 144, 162 ) );

    // Between Button and Light.
    defaultPalette.setColor( QPalette::Midlight, QColor( 128, 134, 152 ) );

    // Darker than Button.
    defaultPalette.setColor( QPalette::Dark, QColor( 58, 62, 72 ) );

    // Between Button and Dark.
    defaultPalette.setColor( QPalette::Mid, QColor( 81, 86, 99 ) );

    // A very dark color. By default, the shadow color is Qt::black.
    defaultPalette.setColor( QPalette::Shadow, QColor( 255, 255, 255 ) );


    // A color to indicate a selected item or the current item.
    defaultPalette.setColor( QPalette::Highlight, QColor( 116, 124, 149 ) );

    // A text color that contrasts with Highlight.
    defaultPalette.setColor( QPalette::HighlightedText, QColor( 255, 255, 255 ) );

    defaultPalette.setColor( QPalette::Link, QColor( 78, 84, 102 ) );
    defaultPalette.setColor( QPalette::LinkVisited, QColor( 88, 94, 112 ) );

    pQApp->setPalette( defaultPalette );
}

int main( int argc, char** argv )
{
    qDebug() << "JackMix starting";
    //Q_INIT_RESOURCE(i18n);

    QApplication *qapp = new QApplication( argc, argv );
    QStringList args = qapp->arguments();
    qapp->setWindowIcon(QIcon(":/data/livemix.svg"));

    QTranslator tor;
    QString sTranslationFile = QString("livemix_") + QLocale::system().name();
    QString sLocale = QLocale::system().name();
    if ( sLocale != "C") {
        QString sTranslationPath = ":i18n";

        bool bTransOk = tor.load(sTranslationFile, sTranslationPath);
//        bool bTransOk = tor.load( sTranslationFile, sTranslationPath );
        if ( bTransOk ) {
            qDebug() << "Using locale: " + sTranslationPath + "/" + sTranslationFile;
        } else {
            qDebug() << "Warning: no locale found: " + sTranslationPath + "/" + sTranslationFile;
        }
        if (tor.isEmpty()) {
            qDebug() << "Warning: error loading locale";
        }
    }
    qapp->installTranslator( &tor );

//    QTranslator translator;
//    translator.load("livemix.fr.qm", ":i18n");
//    translator.load("livemix.fr.qm", "/home/sbrunner/workspace/livemix/i18n");
//    qapp->installTranslator(&translator);

//    qDebug() << QPushButton::trUtf8("Locale Test C.");
//    qDebug() << QPushButton::tr("Locale Test C.");

    setPalette( qapp );

//     QPushButton hello(QPushButton::tr("Hello world!"));
//     hello.resize(100, 30);
//     hello.show();


    QString file;
    for( int i=1; i<args.size(); ++i ) {
        qDebug() << QString( " arg %1: %2" ).arg( i ).arg( args[ i ] );
        if ( QFile::exists( args[ i ] ) )
            file = args[ i ];
    }

    LiveMix::MainWindow *mw;
    if (!file.isEmpty()) {
        mw = new LiveMix::MainWindow( file );
    }
    else {
		if (QFile(QDir::homePath().append("/.livemix/table.lm")).exists()) {
	        mw = new LiveMix::MainWindow(QDir::homePath().append("/.livemix/table.lm"));
		}
		else if (QFile(QCoreApplication::applicationDirPath() + "/default.lm").exists()) {
	        mw = new LiveMix::MainWindow(QCoreApplication::applicationDirPath() + "/default.lm");
		}			
		else {
	        mw = new LiveMix::MainWindow(QCoreApplication::applicationDirPath() + "/../share/livemix/sample/default.lm");
		}
    }

    int ret = qapp->exec();

//    delete mw;

    return ret;
}
