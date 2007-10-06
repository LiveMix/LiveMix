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

#include "channelselector.h"

#include <QString>
#include <QLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>

namespace LiveMix
{

ChannelSelector::ChannelSelector( QString title, QString label, QStringList channels, QWidget* p, const char* n ) : QDialog( p )
{
    this->setWindowTitle( title );

    QGridLayout *_layout = new QGridLayout( this );
    QLabel *_label = new QLabel( label, this );
    _layout->addWidget( _label, 0,0, 1,4 );

    _list = new QListWidget( this );
    _list->addItems( channels );
    _layout->addWidget( _list, 1,0, 1,4 );

    _layout->addItem( new QSpacerItem( 10,10 ), 2,0, 1,4 );

    _layout->addItem( new QSpacerItem( 40,10 ), 3,0 );
    _commit_n_quit = new QPushButton( trUtf8("Commit && Quit"), this );
    _commit_n_quit->setDefault( true );
    connect( _commit_n_quit, SIGNAL( clicked() ), this, SLOT( commitnquit() ) );
    _layout->addWidget( _commit_n_quit, 3,3 );
    _commit = new QPushButton( trUtf8("Commit"), this );
    connect( _commit, SIGNAL( clicked() ), this, SLOT( commit() ) );
    _layout->addWidget( _commit, 3,2 );
    _cancel = new QPushButton( trUtf8("Cancel"), this );
    connect( _cancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    _layout->addWidget( _cancel, 3,1 );

}
ChannelSelector::~ChannelSelector()
{}

void ChannelSelector::commit()
{
    qDebug( "ChannelSelector::commit()" );
    qDebug( "Returning: %s", _list->currentItem()->text().toStdString().c_str() );
    emit selectedChannel( _list->currentItem()->text() );
}
void ChannelSelector::commitnquit()
{
    qDebug( "ChannelSelector::commitnquit()" );
    commit();
    qDebug( "Now quit..." );
    done( 0 );
}

void ChannelSelector::addChannel( QString )
{}
void ChannelSelector::removeChannel( QString )
{}

}; //LiveMix
