/*
    Copyright 2004 - 2007 Arnold Krille <arnold@arnoldarts.de>
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
    the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
    MA 02110-1301, USA.
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


namespace LiveMix
{

class Wrapp;
class WrappVolume;
class InWidget;
class OutWidget;
class PreWidget;
class PostWidget;
class SubWidget;
class MainWidget;
class KeyDo;


#define CHANNEL_WIDTH 50

class Widget : public QWidget
{
    Q_OBJECT

public:
    // \param inchannels, outchannels, backend, parent, name=0
    Widget( QWidget* parent);
    ~Widget();

    /// New input/output channels
    void addinchannel( QString, bool related =true );
    void addprechannel( QString );
    void addpostchannel( QString, bool related =true );
    void addsubchannel( QString );
    /// Remove input/output channels
    void removeinchannel( QString );
    void removeprechannel( QString );
    void removepostchannel( QString );
    void removesubchannel( QString );

    /// Create Controls
    // Create controls. return true on success
    bool createControl();

    void doSelect(ChannelType, QString channel);
    ChannelType getSelectedChanelType();
    QString getSetectedChannelName();

    void showMessage( const QString& msg, int msec=5000 );

	QString getDisplayNameOfChannel(ChannelType p_eType, QString p_sChannelName);
	QString getDisplayChannelType(ChannelType p_eType, bool p_bUpperFirst =true);
	QString getShortDisplayChannelType(ChannelType p_eType, bool p_bUpperFirst =true);
	QString getDisplayFunction(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_sReatedChannelName, bool p_bUpperFirst =true);
	QString getShortDisplayFunction(ElementType p_eElement, QString p_sReatedChannelName, bool p_bStereo = false);
//	QString getShortAbreviationDisplayFunction(ElementType p_eElement, QString p_sReatedChannelName, bool p_bStereo);

    void leftClick(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_sReatedChannelName, QMouseEvent* ev);
    void middleClick(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_sReatedChannelName, QMouseEvent* ev);
    void rightClick(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_sReatedChannelName, QMouseEvent* ev);

    void action(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_sReatedChannelName);
    void addVolume(Volume* p_pVolume, ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, 
    		QString p_sReatedChannelName ="");
    void addToggle(ToggleButton* p_pVolume, ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, 
    		QString p_sReatedChannelName ="");
 	void removeShurtCut(ChannelType p_eType, QString p_sChannelName);

    Fader* createFader(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_rChannelTo ="");
    Rotary* createRotary(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_rChannelTo ="");
    ToggleButton* createToggle(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_rChannelTo ="");

    const QMap<QKeySequence, KeyDo*>* getKeyToWrapp() { return &m_mKeyToWrapp; };
    void clearKeyToWrapp();
    void insertKeyToWrapp(QKeySequence p_rKey, KeyDo* p_pDo) { m_mKeyToWrapp.insert(p_rKey,p_pDo); };
    
//    QString getDisplayElement(ElementType p_eElement);

	bool isGainVisible() {return m_bShowGain; };
	int getFaderHeight() {return m_iFaderHeight; };
	int getEffectFaderHeight() {return m_iEffectFaderHeight; };
	void setGainVisible(bool p_bVisible) {m_bShowGain = p_bVisible; };
	void setFaderHeight(int p_iHeight);
	void setEffectFaderHeight(int p_iHeight);

	static void addLine(QVBoxLayout*, bool bold =false);
	static void addLine(QHBoxLayout*, bool bold =false);
	static void addSpacer(QVBoxLayout*);

public slots:
    // Fills the empty nodes with 1to1-controls
    void init();
    void removeFX(LadspaFXProperties*, struct effect*);

	void showGain();
	void faderHeight();
	void effectFaderHeight();

private slots:
    void update();
    void select(ChannelType, QString channel);
    void addFX();
    void displayFX(struct effect *fx, ChannelType p_eType, QString p_sChannelName);
    void onStatusTimerEvent();

private:
    QHBoxLayout *in_layout;
    QHBoxLayout *pre_layout;
    QHBoxLayout *post_layout;
    QHBoxLayout *sub_layout;
    QHBoxLayout *effect_layout;

    QMap<QString, InWidget*> in;
    QMap<QString, PreWidget*> pre;
    QMap<QString, PostWidget*> post;
    QMap<QString, SubWidget*> sub;
    MainWidget* main_widget;
	QScrollArea *m_pEffectScrollArea;

    LCDDisplay* m_pStatusLabel;
    CpuLoadWidget *cpuLoad;
    QTimer *m_pStatusTimer;

// EffectData effect;
    FaderName *effectName;
    QWidget* m_pEffectStart;

    enum ChannelType m_eSelectType;
    QString m_sSelectChannel;
    QList<effect*> m_lVisibleEffect;

    QMap<ChannelType, QMap<QString, QMap<ElementType, QMap<QString, Wrapp*>*>*>*> m_mShurtCut;
    QMap<QKeySequence, KeyDo*> m_mKeyToWrapp;
    WrappVolume* m_pSelectedWrapper;

    void keyPressEvent (QKeyEvent * p_pEvent);
    void wheelEvent (QWheelEvent *p_pEvent);

	bool m_bShowGain;
	int m_iFaderHeight;
	int m_iEffectFaderHeight;
};

}
; // LiveMix

#endif // MIXINGMATRIX_H
