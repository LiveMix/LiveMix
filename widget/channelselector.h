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

#ifndef CHANNELSELECTOR_H
#define CHANNELSELECTOR_H

#include <QtGui/QDialog>

class QListWidget;
class QPushButton;

namespace LiveMix
{

class ChannelSelector : public QDialog
{
    Q_OBJECT
public:
    ChannelSelector(QString title, QString label, QStringList channels, QWidget*, const char* =0);
    ~ChannelSelector();
signals:
    void selectedChannel(QString);
public slots:
    void addChannel(QString);
    void removeChannel(QString);
private slots:
    void commit();
    void commitnquit();
private:
    QListWidget *_list;
    QPushButton *_commit, *_commit_n_quit, *_cancel;
};

}
; //LiveMix

#endif // CHANNELSELECTOR_H
