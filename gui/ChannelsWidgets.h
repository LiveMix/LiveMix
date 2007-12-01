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
class VWidget;
class TWidget;
class FWidget;
class IFWidget;

class TbWrapp : public QWidget
{
    Q_OBJECT
public:
    TbWrapp(Widget *p_pMatrix, ToggleButton *p_pButton, ElementType p_eType, QString p_rRefChannel);

    ToggleButton *m_pButton;

public slots:
    void clicked();

private:
    ElementType m_eType;
    QString m_rRefChannel;
    Widget *m_pMatrix;
};
class InfoWidget : public QWidget
{
    Q_OBJECT
public:
    InfoWidget(Widget* p_pMatrix);
    ~InfoWidget();

    void addPre(QString channelPre);
    void addPost(QString channelPre);
    void addSub(QString channelPre);

    void removePre(QString channelPre);
    void removePost(QString channelPost);
    void removeSub(QString channelSub);

    int setVisible(bool p_bVisible, ElementType p_eElement, QString p_rChannelTo ="");

    IFWidget* m_pFader;

private:
    ToggleButton* createToggleButton(Widget* p_pMatrix, ElementType p_eElement, QString p_rChannelTo ="");
    QLabel* createLabel(int p_iHeight, ElementType p_eElement, QString p_rChannelTo ="");

    QMap<ElementType, QMap<QString, TbWrapp*>*> m_rToggleButtons;
    QMap<ElementType, QMap<QString, QLabel*>*> m_rLabels;

    /* ToggleButton *gain_tb;
     ToggleButton *mute_tb;
     ToggleButton *pfl_tb;
     ToggleButton *bal_tb;
     ToggleButton *main_on_tb;
     QLabel *gain;
     QLabel *mute;
     QLabel *pfl;
     QLabel *bal;
     QLabel *main_on;

        QMap<QString, ToggleButton*> pre_tb;
        QMap<QString, ToggleButton*> post_tb;
        QMap<QString, ToggleButton*> sub_tb;
        QMap<QString, QLabel*> pre;
        QMap<QString, QLabel*> post;
        QMap<QString, QLabel*> sub;*/

private:
    Widget* m_pMatrix;

    QWidget* wPre;
    QWidget* wPost;
    QWidget* wSub;

    QVBoxLayout* lPre;
    QVBoxLayout* lPost;
    QVBoxLayout* lSub;
};
class ChannelWidget : public QWidget
{
    Q_OBJECT
public:
    QLabel* getLabel() {
        return m_pLabel;
    };
protected:
    QLabel* m_pLabel;
};
class InWidget : public ChannelWidget
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

    FWidget* fader;

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

    QMap<QString, PixmapWidget*> pre_tb;
    QMap<QString, PixmapWidget*> post_tb;
    QMap<QString, PixmapWidget*> sub_tb;

    QMap<QString, VWidget*> pre;
    QMap<QString, VWidget*> post;
    QMap<QString, TWidget*> sub;

// QMap<QWidget*, Wrapp*> m_mWrapps;
};

class PreWidget : public ChannelWidget
{
    Q_OBJECT
public:
    PreWidget(QString channel, Widget* p_pMatrix);
    ~PreWidget();

    FWidget* fader;

    void mouseReleaseEvent(QMouseEvent* ev);

signals:
    void clicked(ChannelType, QString channel);

private:
    QString m_Channel;
    Widget* m_pMatrix;

// QMap<QWidget*, Wrapp*> m_mWrapps;
};

class PostWidget : public ChannelWidget
{
    Q_OBJECT
public:
    PostWidget(QString channel, Widget* p_pMatrix);
    ~PostWidget();

    void addSub(QString channelPost, QString channelSub);
    void removeSub(QString channelIn, QString channelSub);

    FWidget* fader;

    void mouseReleaseEvent(QMouseEvent* ev);

signals:
    void clicked(ChannelType, QString channel);

private:
    QString m_Channel;
    Widget* m_pMatrix;

    QWidget* wSub;
    QVBoxLayout* lSub;
    QMap<QString, PixmapWidget*> sub_tb;
    QMap<QString, TWidget*> sub;

// QMap<QWidget*, Wrapp*> m_mWrapps;
};

class SubWidget : public ChannelWidget
{
    Q_OBJECT
public:
    SubWidget(QString channel, Widget* p_pMatrix);
    ~SubWidget();

    FWidget* fader;

    void mouseReleaseEvent(QMouseEvent* ev);

signals:
    void clicked(ChannelType, QString channel);

private:
    QString m_Channel;
    Widget* m_pMatrix;

// QMap<QWidget*, Wrapp*> wrapps;
};

class MainWidget : public ChannelWidget
{
    Q_OBJECT
public:
    MainWidget(Widget* p_pMatrix);
    ~MainWidget();

    void update();

    void mouseReleaseEvent(QMouseEvent* ev);

    FWidget* fader;

signals:
    void clicked(ChannelType, QString channel);

private:
    Widget* m_pMatrix;

    VWidget *phone;
    Toggle *mute;
    VWidget *mono;
    VWidget *bal;
    Toggle *afl;
};

class IFWidget : public QWidget
{
public:
    IFWidget(Widget *p_pWidget);
    virtual void mousePressEvent(QMouseEvent *p_pEvent);
    virtual void mouseReleaseEvent(QMouseEvent *p_pEvent);

private:
    int m_fMousePressY;
    Widget *m_pWidget;
};

}; // LiveMix

#endif /*CHANNELSWIDGETS_H_*/
