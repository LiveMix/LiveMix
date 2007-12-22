/*
 * LiveMix
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

#ifndef ASSIGNETOPANNEL_H_
#define ASSIGNETOPANNEL_H_

#import "GetKeyField.h"

#import <QDialog>
#import <QLabel>

namespace LiveMix
{

class AssigneToPannel : public QDialog
{
    Q_OBJECT
public:
    AssigneToPannel(QString p_sChannel, QString p_sFunction, bool p_bVolume, bool p_bOnlyDirrect, QKeySequence p_rActionOnChannelKeySequence
                    , QKeySequence p_rSelectChannelKeySequence, QKeySequence p_rActionOnSelectedChannelKeySequence
                    , unsigned char p_iChannel, unsigned int p_iController);
    virtual ~AssigneToPannel();

    QKeySequence getActionOnChannelKeySequence();
    QKeySequence getSelectChannelKeySequence();
    QKeySequence getActionOnSelectedChannelKeySequence();
    unsigned char getChannel();
    unsigned int getController();

public slots:
    void okClicked(bool p_bChecked);
    void cancelClicked(bool p_bChecked);
    virtual void done(int r);

private:
    GetKeyField *m_pActionOnChannel;
    GetKeyField *m_pSelectChannel;
    GetKeyField *m_pActionOnSelectedChannel;

    void timerEvent(QTimerEvent*);
    QLabel *m_pMidiLabel;
    unsigned char m_iChannel;
    unsigned int m_iController;
    int m_iTimer;
};

}
; // LiveMix

#endif /*ASSIGNETOPANNEL_H_*/
