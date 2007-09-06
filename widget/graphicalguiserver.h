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

#ifndef GRAPHICALGUISERVER_H
#define GRAPHICALGUISERVER_H

#include <guiserver_interface.h>

class QWidget;

namespace JackMix
{

class GraphicalGuiServer : public GuiServer_Interface
{
public:
    GraphicalGuiServer( QWidget* );

    void message( const QString&, const QString& ) const;
    bool messageYesNo( const QString&, const QString& ) const;
    bool messageOkCancel( const QString&, const QString& ) const;

    double getDouble( const QString&, const QString&, double, double, double ) const;
    int getInt( const QString&, const QString&, int, int, int ) const;
    QString getString( const QString&, const QString&, const QString& ) const;
    QString getItem( const QString&, const QString&, const QStringList& ) const;
private:
    QWidget* _parent;
};

}
; //JackMix

#endif // GRAPHICALGUISERVER_H

