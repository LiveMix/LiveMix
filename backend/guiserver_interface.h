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

#ifndef GUISERVER_INTERFACE_H
#define GUISERVER_INTERFACE_H

#include <QString>
#include <QStringList>

namespace LiveMix
{

/**
 * @brief Abstract interface for a GUI server
 */
class GuiServer_Interface
{
public:
    GuiServer_Interface()
    {};
    virtual ~GuiServer_Interface()
    {};

    virtual void message( const QString& title, const QString& text ) const =0;
    virtual bool messageYesNo( const QString& title, const QString& text ) const =0;
    virtual bool messageOkCancel( const QString& title, const QString& text ) const =0;

    virtual double getDouble( const QString& title, const QString& label, double initvalue, double minvalue, double maxvalue ) const =0;
    virtual int getInt( const QString& title, const QString& label, int initvalue, int minvalue, int maxvalue ) const =0;
    virtual QString getString( const QString& title, const QString& label, const QString& initvalue ) const =0;
    virtual QString getItem( const QString& title, const QString& label, const QStringList& list ) const =0;
};

}
; // LiveMix

#endif // GUISERVER_INTERFACE_H

