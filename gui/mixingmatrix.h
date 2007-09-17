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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef MIXINGMATRIX_H
#define MIXINGMATRIX_H

#include "backend.h"
#include "Rotary.h"
#include "Fader.h"
#include "FaderName.h"
#include "Button.h"
#include "ClickableLabel.h"
#include "CpuLoadWidget.h"

#include <QtGui/QWidget>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QVBoxLayout>
#include <QScrollArea>


#if ( QT_POINTER_SIZE == 8 )
#define pint qint64
#else
#define pint qint32
#endif

namespace LiveMix
{

class KeyDo;
class Wrapp;
class WrappVolume;
class InWidget;
class PreWidget;
class PostWidget;
class SubWidget;
class MainWidget;

#define CHANNEL_WIDTH 50

//typedef enum TargetType;

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

    /// Layout
// QSize sizeHint() const { return minimumSizeHint(); }

    /// Mode
// enum Mode { Normal, Select };
// Mode mode() const { return _mode; }
// void mode( Mode n ) { _mode=n; }

    void doSelect(Backend::ChannelType, QString channel);
    Backend::ChannelType getSelectedChanelType();
    QString getSetectedChannelName();

    void showMessage( const QString& msg, int msec=5000 );

	QString getDisplayNameOfChannel(Backend::ChannelType p_eType, QString p_sChannelName);
	QString getDisplayChannelType(Backend::ChannelType p_eType);
	QString getShortDisplayChannelType(Backend::ChannelType p_eType);

    void leftClick(Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, QString p_sReatedChannelName, 
    		QString p_sDisplayReatedChannelName, QMouseEvent* ev);
    void middleClick(Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, QString p_sReatedChannelName, 
    		QString p_sDisplayReatedChannelName, QMouseEvent* ev);
    void rightClick(Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, QString p_sReatedChannelName, 
    		QString p_sDisplayReatedChannelName, QMouseEvent* ev);

    void action(Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, QString p_sReatedChannelName, 
    		QString p_sDisplayReatedChannelName);
    void addVolume(Volume* p_pVolume, Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, 
    		QString p_sReatedChannelName ="", QString p_sDisplayReatedChannelName ="");
    void addToggle(ToggleButton* p_pVolume, Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, 
    		QString p_sReatedChannelName ="", QString p_sDisplayReatedChannelName ="");
 	void removeShurtCut(Backend::ChannelType p_eType, QString p_sChannelName);
    
    const QMap<QKeySequence, KeyDo*>* getKeyToWrapp() { return &m_mKeyToWrapp; };
    void clearKeyToWrapp();
    void insertKeyToWrapp(QKeySequence p_rKey, KeyDo* p_pDo) { m_mKeyToWrapp.insert(p_rKey,p_pDo); };
    
    QString getDisplayElement(Backend::ElementType p_eElement);

	bool isGainVisible() {return m_bShowGain; };
	int getFaderHeight() {return m_iFaderHeight; };
	int getEffectFaderHeight() {return m_iEffectFaderHeight; };
	void setGainVisible(bool p_bVisible) {m_bShowGain = p_bVisible; };
	void setFaderHeight(int p_iHeight);
	void setEffectFaderHeight(int p_iHeight);

public slots:
    // Fills the empty nodes with 1to1-controls
    void init();
    void removeFX(LadspaFXProperties*, struct effect*);

	void showGain();
	void faderHeight();
	void effectFaderHeight();

private slots:
    void update();
    void select(Backend::ChannelType, QString channel);
    void addFX();
    void displayFX(struct effect *fx, Backend::ChannelType p_eType, QString p_sChannelName);
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

    enum Backend::ChannelType m_eSelectType;
    QString m_sSelectChannel;
    QList<effect*> m_lVisibleEffect;

    QMap<Backend::ChannelType, QMap<QString, QMap<Backend::ElementType, QMap<QString, Wrapp*>*>*>*> m_mShurtCut;
    QMap<QKeySequence, KeyDo*> m_mKeyToWrapp;
    WrappVolume* m_pSelectedWrapper;

    void keyPressEvent (QKeyEvent * p_pEvent);
    void wheelEvent (QWheelEvent *p_pEvent);

	bool m_bShowGain;
	int m_iFaderHeight;
	int m_iEffectFaderHeight;
};

void addLine(QVBoxLayout*, bool bold =false);
void addLine(QHBoxLayout*, bool bold =false);
void addSpacer(QVBoxLayout*);

}
; // LiveMix

#endif // MIXINGMATRIX_H
