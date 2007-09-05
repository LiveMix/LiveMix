/*
    Copyright 2007 Arnold Krille <arnold@arnoldarts.de>
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

#include "graphicalguiserver.h"

#include <QtGui/QWidget>

#include <QtGui/QMessageBox>
#include <QtGui/QInputDialog>

namespace JackMix
{

GraphicalGuiServer::GraphicalGuiServer( QWidget* p ) : _parent( p )
{}
GraphicalGuiServer::~GraphicalGuiServer()
{}

void GraphicalGuiServer::message( const QString& title, const QString& text ) const
{
    QMessageBox::information( _parent, title, text );
}

bool GraphicalGuiServer::messageYesNo( const QString& title, const QString& text ) const
{
    QMessageBox::StandardButton button = QMessageBox::information( _parent, title, text, QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes );
    if ( button == QMessageBox::Yes )
        return true;
    return false;
}

bool GraphicalGuiServer::messageOkCancel( const QString& title, const QString& text ) const
{
    QMessageBox::StandardButton button = QMessageBox::information( _parent, title, text, QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Ok );
    if ( button == QMessageBox::Ok )
        return true;
    return false;
}

double GraphicalGuiServer::getDouble( const QString& title, const QString& label, double initvalue, double minvalue, double maxvalue ) const
{
    return QInputDialog::getDouble( _parent, title, label, initvalue, minvalue, maxvalue );
}

int GraphicalGuiServer::getInt( const QString& title, const QString& label, int initvalue, int minvalue, int maxvalue ) const
{
    return QInputDialog::getInteger( _parent, title, label, initvalue, minvalue, maxvalue );
}

QString GraphicalGuiServer::getString( const QString& title, const QString& label, const QString& initvalue ) const
{
    return QInputDialog::getText( _parent, title, label, QLineEdit::Normal, initvalue );
}

QString GraphicalGuiServer::getItem( const QString& title, const QString& label, const QStringList& list ) const
{
    bool ok = false;
    QString ret = QInputDialog::getItem( _parent, title, label, list, 0, false, &ok );
    if ( ok )
        return ret;
    return "";
}

}
; //JackMix
