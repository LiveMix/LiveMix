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

#ifndef GRAPHICALGUISERVER_H
#define GRAPHICALGUISERVER_H

#include <guiserver_interface.h>

class QWidget;

namespace LiveMix
{

class GraphicalGuiServer : public GuiServer_Interface
{
public:
    GraphicalGuiServer(QWidget*);
    ~GraphicalGuiServer();

    void message(const QString&, const QString&) const;
    bool messageYesNo(const QString&, const QString&) const;
    bool messageOkCancel(const QString&, const QString&) const;

    double getDouble(const QString&, const QString&, double, double, double) const;
    int getInt(const QString&, const QString&, int, int, int) const;
    QString getString(const QString&, const QString&, const QString&) const;
    QString getItem(const QString&, const QString&, const QStringList&) const;
private:
    QWidget* _parent;
};

}
; //LiveMix

#endif // GRAPHICALGUISERVER_H

