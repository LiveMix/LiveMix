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

#ifndef CHANNELSWIDGETS_H_
#define CHANNELSWIDGETS_H_

#include "mixingmatrix.h"

#include "Rotary.h"
#include "Fader.h"
#include "FaderName.h"
#include "Button.h"
#include "ClickableLabel.h"

#include <QWidget>
#include <QString>
#include <QMouseEvent>
#include <QVBoxLayout>


namespace LiveMix
{

class Widget;

class InWidget : public QWidget
{
    Q_OBJECT
public:
    InWidget(QString p_sChannel, Widget* p_pMatrix);
    ~InWidget();

    void addPre(QString channelIn, QString channelPre);
    void addPost(QString channelIn, QString channelPre);
    void addSub(QString channelIn, QString channelPre);

    void removePre(QString channelIn, QString channelPre);
    void removePost(QString channelIn, QString channelPost);
    void removeSub(QString channelIn, QString channelSub);

    Fader* fader;

    void mouseReleaseEvent(QMouseEvent* ev);

signals:
    void clicked(ChannelType, QString channel);

private:
    QString m_Channel;
    Widget* m_pMatrix;

    QWidget* wPre;
    QWidget* wPost;
    QWidget* wSub;

    QVBoxLayout* lPre;
    QVBoxLayout* lPost;
    QVBoxLayout* lSub;

    QMap<QString, Rotary*> pre;
    QMap<QString, Rotary*> post;
    QMap<QString, ToggleButton*> sub;

// QMap<QWidget*, Wrapp*> m_mWrapps;
};

class PreWidget : public QWidget
{
    Q_OBJECT
public:
    PreWidget(QString channel, Widget* p_pMatrix);
    ~PreWidget();

    Fader* fader;

    void mouseReleaseEvent(QMouseEvent* ev);

signals:
    void clicked(ChannelType, QString channel);

private:
    QString m_Channel;
    Widget* m_pMatrix;

// QMap<QWidget*, Wrapp*> m_mWrapps;
};

class PostWidget : public QWidget
{
    Q_OBJECT
public:
    PostWidget(QString channel, Widget* p_pMatrix);
    ~PostWidget();

    void addSub(QString channelPost, QString channelSub);
    void removeSub(QString channelIn, QString channelSub);

    Fader* fader;

    void mouseReleaseEvent(QMouseEvent* ev);

signals:
    void clicked(ChannelType, QString channel);

private:
    QString m_Channel;
    Widget* m_pMatrix;

    QWidget* wSub;
    QVBoxLayout* lSub;
    QMap<QString, ToggleButton*> sub;

// QMap<QWidget*, Wrapp*> m_mWrapps;
};

class SubWidget : public QWidget
{
    Q_OBJECT
public:
    SubWidget(QString channel, Widget* p_pMatrix);
    ~SubWidget();

    Fader* fader;

    void mouseReleaseEvent(QMouseEvent* ev);

signals:
    void clicked(ChannelType, QString channel);

private:
    QString m_Channel;
    Widget* m_pMatrix;

// QMap<QWidget*, Wrapp*> wrapps;
};

class MainWidget : public QWidget
{
    Q_OBJECT
public:
    MainWidget(Widget* p_pMatrix);
    ~MainWidget();

    void update();

    void mouseReleaseEvent(QMouseEvent* ev);

    Fader* fader;

signals:
    void clicked(ChannelType, QString channel);

private:
    Widget* m_pMatrix;

    Rotary *phone;
    ToggleButton* mute;
    Rotary *mono;
    Rotary *bal;
    ToggleButton* alf;
// QMap<QWidget*, Wrapp*> m_mWrapps;
};

}
; // LiveMix

#endif /*CHANNELSWIDGETS_H_*/
