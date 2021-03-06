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

#ifndef MIXINGMATRIX_H
#define MIXINGMATRIX_H

#include "ChannelsWidgets.h"
#include "Wrapp.h"
#include "KeyDo.h"
#include "backend.h"
#include "CpuLoadWidget.h"
#include "LCD.h"
#include "FaderName.h"
#include "Action.h"
#include "Button.h"
#include "Fader.h"
#include "Rotary.h"

#include <QWidget>
#include <QList>
#include <QMap>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QContextMenuEvent>


namespace LiveMix
{

class Wrapp;
class WrappVolume;
class InfoWidget;
class InWidget;
class OutWidget;
class PreWidget;
class PostWidget;
class SubWidget;
class MainWidget;
class KeyDo;
class KeyDoDirectAction;
class VWidget;
class RWidget;
class TWidget;
class FWidget;
class ChannelWidget;


#define CHANNEL_WIDTH 50
#define SEPARATOR_HEIGHT 10
#define INFO_WIDTH 30
#define MIDI 2


class LFWidget : public FaderName
{
public:
    LFWidget(Widget *p_pWidget);
    virtual void mousePressEvent(QMouseEvent *p_pEvent);
    virtual void mouseReleaseEvent(QMouseEvent *p_pEvent);

private:
    int m_fMousePressY;
    Widget *m_pWidget;
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    // \param inchannels, outchannels, backend, parent, name=0
    Widget(QWidget* parent);
    ~Widget();

    /// New input/output channels
    void addinchannel(QString, bool related =true);
    void addprechannel(QString);
    void addpostchannel(QString, bool related =true);
    void addsubchannel(QString);
    /// Remove input/output channels
    void removeinchannel(QString);
    void removeprechannel(QString);
    void removepostchannel(QString);
    void removesubchannel(QString);

    /// Create Controls
    // Create controls. return true on success
    bool createControl();

    void doSelect(ChannelType, QString channel, bool p_bForce =false);
    ChannelType getSelectedChanelType();
    QString getSetectedChannelName();

    void showMessage(const QString& msg, int msec=5000);

    static QString getDisplayNameOfChannel(ChannelType p_eType, QString p_sChannelName);
    static QString getDisplayChannelType(ChannelType p_eType, bool p_bUpperFirst =true);
    static QString getShortDisplayChannelType(ChannelType p_eType, bool p_bUpperFirst =true);
    static QString getDisplayFunction(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_sReatedChannelName, bool p_bUpperFirst =true);
    static QString getMediumDisplayFunction(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_sReatedChannelName, bool p_bUpperFirst =true);
    static QString getShortDisplayFunction(ElementType p_eElement, QString p_sReatedChannelName, bool p_bStereo = false);
// QString getShortAbreviationDisplayFunction(ElementType p_eElement, QString p_sReatedChannelName, bool p_bStereo);

    void leftClick(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_sReatedChannelName, QMouseEvent* ev);
    void middleClick(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_sReatedChannelName, QMouseEvent* ev);
    void rightClick(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_sReatedChannelName, QMouseEvent* ev);
    void mouseDoubleClickEvent(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_sReatedChannelName, QMouseEvent* ev);

    void action(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_sReatedChannelName);
    void addVolume(Volume* p_pVolume, ChannelType p_eType, QString p_sChannelName, ElementType p_eElement,
                   QString p_sReatedChannelName ="");
    void addToggle(Toggle* p_pVolume, ChannelType p_eType, QString p_sChannelName, ElementType p_eElement,
                   QString p_sReatedChannelName ="");
    void removeShurtCut(ChannelType p_eType, QString p_sChannelName);

    FWidget* createFader(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_rChannelTo ="");
    VWidget* createRotary(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_rChannelTo ="");
    TWidget* createToggle(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_rChannelTo ="");

    const QMap<QKeySequence, KeyDo*>* getKeyToWrapp() {
        return &m_mKeyToWrapp;
    };
    void clearKeyToWrapp();
    void insertKeyToWrapp(QKeySequence p_rKey, KeyDo* p_pDo) {
        m_mKeyToWrapp.insert(p_rKey,p_pDo);
    };

    const QMap<unsigned char, QMap<unsigned int, KeyDoDirectAction*>*>* getMidiToWrapp() {
        return &m_mMidiToWrapp;
    };
    void clearMidiToWrapp();
    void insertMidiToWrapp(unsigned char p_iChannel, unsigned int p_iControler, KeyDoDirectAction* p_pDo) {
        if (!m_mMidiToWrapp.contains(p_iChannel)) {
            m_mMidiToWrapp.insert(p_iChannel, new QMap<unsigned int, KeyDoDirectAction*>);
        }
        m_mMidiToWrapp[p_iChannel]->insert(p_iControler,p_pDo);
    };

//    QString getDisplayElement(ElementType p_eElement);

    bool isVisible(ElementType p_eElement, QString p_rChannelTo ="");
    int getFaderHeight() {
        return m_iFaderHeight;
    };
    int getEffectFaderHeight() {
        return m_iEffectFaderHeight;
    };
    void setVisible(bool p_bVisible, ElementType p_eElement, QString p_rChannelTo ="");
    void setFaderHeight(int p_iHeight);
    void setEffectFaderHeight(int p_iHeight);

// static void addLine(QVBoxLayout*, bool bold =false);
// static void addLine(QHBoxLayout*, bool bold =false);
    static void addSpacer(QVBoxLayout*);

    void clearAll();

    void sendMidiEvent(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_sReatedChannelName, float p_fValue);

public slots:
    // Fills the empty nodes with 1to1-controls
    void init();
    void removeFX(LadspaFXProperties*, effect*);
    void removeFX(LadspaFXProperties*, effect*, ChannelType p_eType, QString p_rChannel);
    void askRemoveFX(LadspaFXProperties*, effect*);
    void askLeftFX(LadspaFXProperties*, effect*);
    void askRightFX(LadspaFXProperties*, effect*);

// void showGain();
    void showAll();
    void hideAll();
    void faderHeight();
    void effectFaderHeight();

private slots:
    void update();
    void select(ChannelType, QString channel);
    void addFX();
    void displayFX(effect *fx, ChannelType p_eType, QString p_sChannelName);
    void onStatusTimerEvent();

    void assigneKey();
    void resetAllTheLine();
    void enableAllTheLine();
    void desableAllTheLine();

    void newValue();
    void newLineValue();

private:
    ChannelWidget* getFaderWidget(ChannelType p_eType, QString p_rChannel);
    void newValue(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_sReatedChannelName, bool p_bToLine);
    float getNewValue(float p_fOld, float p_fNew);

    InfoWidget *info_widget;

    QHBoxLayout *in_layout;
    QHBoxLayout *pre_layout;
    QHBoxLayout *post_layout;
    QHBoxLayout *sub_layout;
    QHBoxLayout *effect_layout;

    QMap<QString, InWidget*> m_in;
    QMap<QString, PreWidget*> m_pre;
    QMap<QString, PostWidget*> m_post;
    QMap<QString, SubWidget*> m_sub;
    MainWidget* main_widget;
    QScrollArea *m_pEffectScrollArea;

    LCDDisplay* m_pStatusLabel;
    CpuLoadWidget *cpuLoad;
    QTimer *m_pStatusTimer;

// EffectData effect;
    LFWidget *effectName;
    QWidget *m_pEffectStart;

    ChannelType m_eSelectType;
    QString m_sSelectChannel;
    ElementType m_eSelectedElement;
    QString m_sSelectedReatedChannelName;
    QList<effect*> m_lVisibleEffect;

    QMap<ChannelType, QMap<QString, QMap<ElementType, QMap<QString, Wrapp*>*>*>*> m_mShurtCut;
    QMap<QKeySequence, KeyDo*> m_mKeyToWrapp;
    WrappVolume* m_pSelectedWrapper;

    void timerEvent(QTimerEvent*);
    QMap<unsigned char, QMap<unsigned int, KeyDoDirectAction*>*> m_mMidiToWrapp;
    bool m_bLearn;

    void keyPressEvent(QKeyEvent * p_pEvent);
    void wheelEvent(QWheelEvent *p_pEvent);

    QMap<ElementType, QMap<QString, bool>*> m_bVisible;
    int m_iFaderHeight;
    int m_iEffectFaderHeight;
};

class TWidget : public Toggle
{
    Q_OBJECT
public:
    TWidget();
    ToggleButton* getToggle();
    virtual QWidget* getWidget();

    virtual bool getValue();
    virtual void setValue(bool value, bool emit = false, int p_iSource =0);

protected:
    ToggleButton* m_pToggle;
};

class VWidget : public Volume
{
    Q_OBJECT
public:
    VWidget();
    Volume* getVolume();
    virtual QWidget* getWidget();

    virtual void setValue(float fValue, bool emit = false, int p_iSource =0);
    virtual float getValue();
    virtual void setDbValue(float fValue);
    virtual float getDbValue();
    virtual float getMinValue();
    virtual float getMaxValue();
    virtual void incValue(bool p_bDirection, int p_iStep =1);

protected:
    Volume* m_pVolume;
};

class RWidget : public VWidget
{
    Q_OBJECT
public:
    RWidget(ElementType p_eElement, QString p_rToolTip);
    Rotary* getRotary();
private:
    void paintEvent(QPaintEvent* p_pEvent);
    QPixmap* m_pBackground;
};

class FWidget : public VWidget
{
    Q_OBJECT
public:
    FWidget(int p_fFaderHeignt, ChannelType p_eType, QString p_sChannelName);
    Fader* getFader();
    Fader* getMeter();
    FaderName* getLabelFader();
    virtual void setFixedHeight(int y);
protected:
    FaderName* m_pLabelFader;
    Fader* m_pMeter;
    ChannelType m_eType;
    QString m_sChannelName;
protected slots:
    void changeName();
};

}
; // LiveMix

#endif // MIXINGMATRIX_H
