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

#include <QtGui/QWidget>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QVBoxLayout>

#include "backend.h"
#include "Rotary.h"
#include "Fader.h"
#include "FaderName.h"
#include "Button.h"
#include "ClickableLabel.h"
#include "CpuLoadWidget.h"

#if ( QT_POINTER_SIZE == 8 )
#define pint qint64
#else
#define pint qint32
#endif

namespace JackMix
{

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

    void showMessage( const QString& msg, int msec=5000 );

	void middleClick(Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, QString p_sReatedChannelName, QMouseEvent* ev);
	void rightClick(Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, QString p_sReatedChannelName, QMouseEvent* ev);

public slots:
    // Fills the empty nodes with 1to1-controls
    void init();
    void removeFX(LadspaFXProperties*, struct effect*);

private slots:
    void update();
    void select(Backend::ChannelType, QString channel);
    void addFX();
    void displayFX(struct effect *fx);
    void faderValueChange(Backend::ChannelType type, QString channel, QString p_fValue);
    void onStatusTimerEvent();

private:
    FaderName *effectName;

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

    LCDDisplay* m_pStatusLabel;
    CpuLoadWidget *cpuLoad;
    QTimer *m_pStatusTimer;

// EffectData effect;
    QWidget* m_pEffectStart;

    enum Backend::ChannelType m_eSelectType;
    QString m_sSelectChannel;
    QList<effect*> m_lVisibleEffect;
};

class Wrapp : public QObject {
    Q_OBJECT
public:
	Wrapp(Widget* p_pMatrix, QObject* p_pObject, Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, QString p_sReatedChannelName) 
	 : QObject()
	 , m_pMatrix(p_pMatrix)
	 , m_eType(p_eType)
	 , m_sChannelName(p_sChannelName)
	 , m_eElement(p_eElement)
	 , m_sReatedChannelName(p_sReatedChannelName)
	{
	    connect(p_pObject, SIGNAL( middleClick(QMouseEvent*) ), this, SLOT( middleClick(QMouseEvent*) ) );
	    connect(p_pObject, SIGNAL( rightClick(QMouseEvent*) ), this, SLOT( rightClick(QMouseEvent*) ) );
	};
	
public slots:
	void middleClick(QMouseEvent* p_ev) {
		m_pMatrix->middleClick(m_eType, m_sChannelName, m_eElement, m_sReatedChannelName, p_ev);
	};
	void rightClick(QMouseEvent* p_ev) {
		m_pMatrix->rightClick(m_eType, m_sChannelName, m_eElement, m_sReatedChannelName, p_ev);
	};

private:
	Widget* m_pMatrix;
	Backend::ChannelType m_eType;
	QString m_sChannelName;
	Backend::ElementType m_eElement;
	QString m_sReatedChannelName;
	
	void middleClick(Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, QString p_sReatedChannelName, QMouseEvent* ev);
	void rightClick(Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, QString p_sReatedChannelName, QMouseEvent* ev);
 
};

class InWidget : public QWidget
{
    Q_OBJECT
public:
    InWidget( QString channel );
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
    void clicked(Backend::ChannelType, QString channel);

private:
    QString m_Channel;

    QWidget* wPre;
    QWidget* wPost;
    QWidget* wSub;

    QVBoxLayout* lPre;
    QVBoxLayout* lPost;
    QVBoxLayout* lSub;

    QMap<QString, Rotary*> pre;
    QMap<QString, Rotary*> post;
    QMap<QString, ToggleButton*> sub;

	QMap<QWidget*, Wrapp*> m_mWrapps;
};

class PreWidget : public QWidget
{
    Q_OBJECT
public:
    PreWidget(QString channel);
    ~PreWidget();

    Fader* fader;

    void mouseReleaseEvent(QMouseEvent* ev);

signals:
    void clicked(Backend::ChannelType, QString channel);

private:
    QString m_Channel;

	QMap<QWidget*, Wrapp*> m_mWrapps;
};

class PostWidget : public QWidget
{
    Q_OBJECT
public:
    PostWidget(QString channel);
    ~PostWidget();

    void addSub(QString channelPost, QString channelSub);
    void removeSub(QString channelIn, QString channelSub);

    Fader* fader;

    void mouseReleaseEvent(QMouseEvent* ev);

signals:
    void clicked(Backend::ChannelType, QString channel);

private:
    QString m_Channel;

    QWidget* wSub;
    QVBoxLayout* lSub;
    QMap<QString, ToggleButton*> sub;

	QMap<QWidget*, Wrapp*> m_mWrapps;
};

class SubWidget : public QWidget
{
    Q_OBJECT
public:
    SubWidget(QString channel );
    ~SubWidget();

    Fader* fader;

    void mouseReleaseEvent(QMouseEvent* ev);

signals:
    void clicked(Backend::ChannelType, QString channel);

private:
    QString m_Channel;

	QMap<QWidget*, Wrapp*> wrapps;
};

class MainWidget : public QWidget
{
    Q_OBJECT
public:
    MainWidget();
    ~MainWidget();

    void update();

    Fader* fader;
    Rotary *phone;
    ToggleButton* mute;
    Rotary *mono;
    Rotary *bal;
    ToggleButton* alf;

    void mouseReleaseEvent(QMouseEvent* ev);

signals:
    void clicked(Backend::ChannelType, QString channel);

private:
	QMap<QWidget*, Wrapp*> m_mWrapps;
};

void addLine(QVBoxLayout*, bool bold =false);
void addLine(QHBoxLayout*, bool bold =false);
void addSpacer(QVBoxLayout*);

}; // JackMix

#endif // MIXINGMATRIX_H
