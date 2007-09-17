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

#include "mixingmatrix.h"

#include "LadspaFXProperties.h"
#include "LadspaFXSelector.h"
#include "AssigneToPannel.h"

#include <QMetaProperty>
#include <QDebug>
#include <QTimer>
#include <QRubberBand>
#include <QLayout>
#include <QListWidget>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QCursor>
#include <QContextMenuEvent>
#include <QLabel>
#include <QSpacerItem>
#include <QFont>
#include <QMessageBox>
#include <QInputDialog>


namespace LiveMix
{

class ScrollArea : public QScrollArea {
	void keyPressEvent ( QKeyEvent * e ) {
		switch (e->key()) {
			case Qt::Key_Up:
			case Qt::Key_Down:
			case Qt::Key_PageUp:
			case Qt::Key_PageDown:
				e->setAccepted(false);
				break;
			default:
				QScrollArea::keyPressEvent(e);
		}
	}
};

Widget::Widget(QWidget* p)
        : QWidget( p )
        , m_mShurtCut()
        , m_pSelectedWrapper(NULL)
        , m_bShowGain(true)
		, m_iFaderHeight(300)
		, m_iEffectFaderHeight(200)        
{
    QVBoxLayout *main_layout = new QVBoxLayout;
    main_layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

#ifdef LADSPA_SUPPORT
    m_pEffectScrollArea = new ScrollArea;
	m_pEffectScrollArea->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    m_pEffectScrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
    QWidget* effect = new QWidget;
    m_pEffectScrollArea->setFixedHeight(m_iEffectFaderHeight + 102);
//    m_pEffectScrollArea->setMinimumHeight(m_iEffectFaderHeight + 102);
    main_layout->addWidget(m_pEffectScrollArea);
    m_pEffectScrollArea->setWidget( effect );

    effect_layout = new QHBoxLayout;
    effect->setLayout(effect_layout);
    effect_layout->setSizeConstraint(QLayout::SetMinimumSize);
    QWidget* effect_start = new QWidget();
    m_pEffectStart = effect_start;
    effect_layout->addWidget(effect_start);
    QVBoxLayout *effect_start_layout = new QVBoxLayout;
    effect_start_layout->setSpacing(0);
    effect_start_layout->setMargin(0);
    effect_start->setFixedHeight(m_iEffectFaderHeight + 64);
    effect_start->setLayout(effect_start_layout);
//    effect_start->hide();
    Button* select = Button::create();
    connect(select, SIGNAL( clicked() ), this, SLOT( addFX() ) );
    select->setText(trUtf8("A"));
    select->setToolTip(trUtf8("Add effect"));
    effect_start_layout->addWidget(select);
    addSpacer(effect_start_layout);
    effectName = new FaderName();
    {
        QFont font = effectName->font();
        font.setPixelSize(16);
        font.setItalic(false);
        effectName->setFont(font);
    }
    effect_start_layout->addWidget(effectName);
    effect_layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
#endif

    QWidget* mix_main = new QWidget;
    main_layout->addWidget(mix_main);
    QWidget* status = new QWidget;
    main_layout->addWidget(status);
    this->setLayout(main_layout);

    QHBoxLayout *status_layout = new QHBoxLayout;
    status_layout->setSpacing(0);
    status_layout->setMargin(0);
    status->setLayout(status_layout);
    status_layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    m_pStatusLabel = new LCDDisplay(NULL, LCDDigit::SMALL_BLUE, 60, false);
    status_layout->addWidget(m_pStatusLabel);
    cpuLoad = new CpuLoadWidget(NULL);
    status_layout->addWidget(cpuLoad);
// status.setColor( QPalette::Background, QColor( 128, 134, 152 ) );

    QHBoxLayout *mix_layout = new QHBoxLayout;
    mix_main->setLayout(mix_layout);
    mix_layout->setSpacing(0);
    mix_layout->setMargin(0);
    QWidget *in = new QWidget;
    mix_layout->addWidget(in);
    mix_layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    QWidget *post = new QWidget;
    mix_layout->addWidget(post);
    QWidget *pre = new QWidget;
    mix_layout->addWidget(pre);
    QWidget *sub = new QWidget;
    mix_layout->addWidget(sub);
    main_widget = new MainWidget(this);
    mix_layout->addWidget(main_widget);
    connect( main_widget, SIGNAL( clicked(Backend::ChannelType, QString) ), this, SLOT( select(Backend::ChannelType, QString) ) );

    in_layout = new QHBoxLayout;
    pre_layout = new QHBoxLayout;
    post_layout = new QHBoxLayout;
    sub_layout = new QHBoxLayout;
    in_layout->setSpacing(0);
    pre_layout->setSpacing(0);
    post_layout->setSpacing(0);
    sub_layout->setSpacing(0);
    in_layout->setMargin(0);
    pre_layout->setMargin(0);
    post_layout->setMargin(0);
    sub_layout->setMargin(0);
    in->setLayout(in_layout);
    pre->setLayout(pre_layout);
    post->setLayout(post_layout);
    sub->setLayout(sub_layout);

    connect( Backend::instance(), SIGNAL( processed() ), this, SLOT( update() ) );

    m_pStatusTimer = new QTimer( this );
    connect( m_pStatusTimer, SIGNAL( timeout() ), this, SLOT( onStatusTimerEvent() ) );
}

Widget::~Widget()
{}

void Widget::displayFX(struct effect *fx, Backend::ChannelType p_eType, QString p_sChannelName)
{
    if (fx->gui == NULL)
    {
        fx->gui = new LadspaFXProperties(NULL, fx);
        fx->gui->setFaderHeight(m_iEffectFaderHeight);
	    addToggle(fx->gui->getActivateButton(), p_eType, p_sChannelName, Backend::MUTE_EFFECT, fx->fx->getPluginLabel(), fx->fx->getPluginName());
        effect_layout->addWidget(fx->gui);
        connect(fx->gui, SIGNAL(removeClicked(LadspaFXProperties*, struct effect*)), this, SLOT(removeFX(LadspaFXProperties*, struct effect*)));
    }
    fx->gui->show();
    m_lVisibleEffect << fx;
}

void Widget::removeFX(LadspaFXProperties* widget, struct effect* fx)
{
    m_lVisibleEffect.removeAll(fx);
    disconnect(widget, 0, 0, 0);
// disconnect(0, 0, widget, 0);
    widget->hide();
    effect_layout->removeWidget(widget);

    // workaround
// QSize size(effect_layout->parentWidget()->width() - widget->width() - 10, effect_layout->parentWidget()->height());
    QSize size(50, effect_layout->parentWidget()->height());
//    QSize size(50, m_iEffectHeight);
    effect_layout->parentWidget()->setMinimumSize(size);
    effect_layout->parentWidget()->adjustSize();

    switch (m_eSelectType)
    {
    case Backend::IN: {
            Backend::instance()->removeInEffect(m_sSelectChannel, fx);
            break;
        }
    case Backend::OUT: {
            Backend::instance()->removeOutEffect(m_sSelectChannel, fx);
            break;
        }
    case Backend::PRE: {
            Backend::instance()->removePreEffect(m_sSelectChannel, fx);
            break;
        }
    case Backend::POST: {
            Backend::instance()->removePostEffect(m_sSelectChannel, fx);
            break;
        }
    case Backend::SUB: {
            Backend::instance()->removeSubEffect(m_sSelectChannel, fx);
            break;
        }
    }
// TODO delete ?
// delete widget;
}

void Widget::addFX()
{
#ifdef LADSPA_SUPPORT
    LadspaFX* fx = LadspaFXProperties::getFXSelector(NULL);
    if (fx != NULL) {
        struct effect* elem = NULL;
        switch (m_eSelectType) {
        case Backend::IN: {
                elem = Backend::instance()->addInEffect(m_sSelectChannel, fx);
                break;
            }
        case Backend::OUT: {
                elem = Backend::instance()->addOutEffect(m_sSelectChannel, fx);
                break;
            }
        case Backend::PRE: {
                elem = Backend::instance()->addPreEffect(m_sSelectChannel, fx);
                break;
            }
        case Backend::POST: {
                elem = Backend::instance()->addPostEffect(m_sSelectChannel, fx);
                break;
            }
        case Backend::SUB: {
                elem = Backend::instance()->addSubEffect(m_sSelectChannel, fx);
                break;
            }
        }
        displayFX(elem, m_eSelectType, m_sSelectChannel);
    }
#endif
}

void Widget::showMessage( const QString& msg, int msec )
{
    qDebug() << "Status:" << msg;
    m_pStatusLabel->setText(msg);
    m_pStatusTimer->start(msec);
}
void Widget::onStatusTimerEvent()
{
    m_pStatusTimer->stop();
    m_pStatusLabel->setText( "" );
}

void Widget::select(Backend::ChannelType type, QString channel)
{
    doSelect(type, channel);
}
void Widget::doSelect(Backend::ChannelType type, QString channel)
{
// effect = new struct effectData;
    if (m_eSelectType == type && m_sSelectChannel == channel) {
        return;
    }
    m_eSelectType = type;
    m_sSelectChannel = channel;

    effectName->setText(channel);
//    m_pEffectStart->show();

    foreach (struct effect* fx, m_lVisibleEffect) {
        fx->gui->hide();
    }
    m_lVisibleEffect.clear();

    switch (m_eSelectType) {
    case Backend::IN: {
            showMessage(trUtf8("Input \"%1\" selected.").arg(Backend::instance()->getInput(m_sSelectChannel)->display_name));
            foreach (effect* elem, *(Backend::instance()->getInEffects(m_sSelectChannel))) {
                displayFX(elem, m_eSelectType, m_sSelectChannel);
            }
            break;
        }
    case Backend::OUT: {
            showMessage(trUtf8("Output selected."));
            foreach (effect* elem, *(Backend::instance()->getOutEffects(m_sSelectChannel))) {
                displayFX(elem, m_eSelectType, m_sSelectChannel);
            }
            break;
        }
    case Backend::PRE: {
            showMessage(trUtf8("Pre fader aux \"%1\" selected.").arg(Backend::instance()->getPre(m_sSelectChannel)->display_name));
            foreach (effect* elem, *(Backend::instance()->getPreEffects(m_sSelectChannel))) {
                displayFX(elem, m_eSelectType, m_sSelectChannel);
            }
            break;
        }
    case Backend::POST: {
            showMessage(trUtf8("Post fader aux \"%1\" selected.").arg(Backend::instance()->getPost(m_sSelectChannel)->display_name));
            foreach (effect* elem, *(Backend::instance()->getPostEffects(m_sSelectChannel))) {
                displayFX(elem, m_eSelectType, m_sSelectChannel);
            }
            break;
        }
    case Backend::SUB: {
            showMessage(trUtf8("Sub-groupe \"%1\" selected.").arg(Backend::instance()->getSub(m_sSelectChannel)->display_name));
            foreach (effect* elem, *(Backend::instance()->getSubEffects(m_sSelectChannel))) {
                displayFX(elem, m_eSelectType, m_sSelectChannel);
            }
            break;
        }
    }
    effect_layout->parentWidget()->resize(effect_layout->parentWidget()->minimumSize());
}

Backend::ChannelType Widget::getSelectedChanelType()
{
    return m_eSelectType;
}

QString Widget::getSetectedChannelName()
{
    return m_sSelectChannel;
}

void Widget::update()
{
    cpuLoad->setValue(Backend::instance()->getProcessTime() / Backend::instance()->getMaxProcessTime());
    cpuLoad->setValue2(Backend::instance()->getCPULoad() / 100.0);
    cpuLoad->setToolTip(trUtf8("- %CPU used by LiveMix backend (%1 %).\n- CPU load given by Jack (%2 %).")
    		.arg((int)(Backend::instance()->getProcessTime() / Backend::instance()->getMaxProcessTime() * 100))
    		.arg((int)Backend::instance()->getCPULoad()));
    
    foreach (QString in_name, Backend::instance()->inchannels()) {
        if (in[in_name] != NULL) {
            Fader* fader = in[in_name]->fader;
            fader->setDbPeak_L(Backend::instance()->getInPeak(in_name, true));
            fader->setDbPeak_R(Backend::instance()->getInPeak(in_name, false));
        }
    }
    foreach (QString pre_name, Backend::instance()->prechannels()) {
        if (pre[pre_name] != NULL) {
            Fader* fader = pre[pre_name]->fader;
            fader->setDbPeak_L(Backend::instance()->getPrePeak(pre_name, true));
            fader->setDbPeak_R(Backend::instance()->getPrePeak(pre_name, false));
        }
    }
    foreach (QString post_name, Backend::instance()->postchannels()) {
        if (post[post_name] != NULL) {
            Fader* fader = post[post_name]->fader;
            fader->setDbPeak_L(Backend::instance()->getPostPeak(post_name, true));
            fader->setDbPeak_R(Backend::instance()->getPostPeak(post_name, false));
        }
    }
    foreach (QString sub_name, Backend::instance()->subchannels()) {
        if (sub[sub_name] != NULL) {
            Fader* fader = sub[sub_name]->fader;
            fader->setDbPeak_L(Backend::instance()->getSubPeak(sub_name, true));
            fader->setDbPeak_R(Backend::instance()->getSubPeak(sub_name, false));
        }
    }
    main_widget->fader->setDbPeak_L(Backend::instance()->getOutPeak(MAIN, true));
    main_widget->fader->setDbPeak_R(Backend::instance()->getOutPeak(MAIN, false));
}
void Widget::init()
{
    main_widget->update();
    foreach (QString in_name, Backend::instance()->inchannels()) {
        addinchannel( in_name, false );
    }
    foreach (QString name, Backend::instance()->prechannels()) {
        addprechannel( name );
    }
    foreach (QString name, Backend::instance()->postchannels()) {
        addpostchannel( name, false );
    }
    foreach (QString name, Backend::instance()->subchannels()) {
        addsubchannel( name );
    }
    doSelect(Backend::OUT, MAIN);
}

void Widget::addinchannel( QString name, bool related )
{
    InWidget* elem = new InWidget(name, this);
    in_layout->addWidget(elem);
    in[name] = elem;
    connect( elem, SIGNAL( clicked(Backend::ChannelType, QString) ), this, SLOT( select(Backend::ChannelType, QString) ) );

    if (related) {
        QMapIterator<QString, PreWidget *> iter_pre(pre);
        while (iter_pre.hasNext()) {
            iter_pre.next();
            elem->addPre(name, iter_pre.key());
        }

        QMapIterator<QString, PostWidget *> iter_post(post);
        while (iter_post.hasNext()) {
            iter_post.next();
            elem->addPost(name, iter_post.key());
        }

        QMapIterator<QString, SubWidget *> iter_sub(sub);
        while (iter_sub.hasNext()) {
            iter_sub.next();
            elem->addSub(name, iter_sub.key());
        }
    }
}
void Widget::addprechannel( QString name )
{
    PreWidget* elem = new PreWidget(name, this);
    pre_layout->addWidget(elem);
    pre[name] = elem;
    connect( elem, SIGNAL( clicked(Backend::ChannelType, QString) ), this, SLOT( select(Backend::ChannelType, QString) ) );

    QMapIterator<QString, InWidget *> iter(in);
    while (iter.hasNext()) {
        iter.next();
        iter.value()->addPre(iter.key(), name);
    }
}
void Widget::addpostchannel( QString name, bool related )
{
    PostWidget* elem = new PostWidget(name, this);
    post_layout->addWidget(elem);
    post[name] = elem;
    connect( elem, SIGNAL( clicked(Backend::ChannelType, QString) ), this, SLOT( select(Backend::ChannelType, QString) ) );

    QMapIterator<QString, InWidget *> iter(in);
    while (iter.hasNext()) {
        iter.next();
        iter.value()->addPost(iter.key(), name);
    }

    if (related) {
        QMapIterator<QString, SubWidget *> iter(sub);
        while (iter.hasNext()) {
            iter.next();
            elem->addSub(name, iter.key());
        }
    }
}
void Widget::addsubchannel( QString name )
{
    SubWidget* elem = new SubWidget(name, this);
    sub_layout->addWidget(elem);
    sub[name] = elem;
    connect( elem, SIGNAL( clicked(Backend::ChannelType, QString) ), this, SLOT( select(Backend::ChannelType, QString) ) );

    QMapIterator<QString, InWidget *> iter_in(in);
    while (iter_in.hasNext()) {
        iter_in.next();
        iter_in.value()->addSub(iter_in.key(), name);
    }
    QMapIterator<QString, PostWidget *> iter_post(post);
    while (iter_post.hasNext()) {
        iter_post.next();
        iter_post.value()->addSub(iter_post.key(), name);
    }
}
void Widget::removeinchannel( QString name )
{
    InWidget *elem = in[name];
    in_layout->removeWidget(elem);
    in.remove(name);
    delete elem;
}
void Widget::removeprechannel( QString name )
{
    PreWidget *elem = pre[name];
    pre_layout->removeWidget(elem);
    pre.remove(name);
    delete elem;

    for (QMap<QString, InWidget*>::iterator i = in.begin() ; i != in.end() ; ++i) {
        i.value()->removePre(i.key(), name);
    }
}
void Widget::removepostchannel( QString name )
{
    PostWidget *elem = post[name];
    post_layout->removeWidget(elem);
    post.remove(name);
    delete elem;

    for (QMap<QString, InWidget*>::iterator i = in.begin() ; i != in.end() ; ++i) {
        i.value()->removePost(i.key(), name);
    }
}
void Widget::removesubchannel( QString name )
{
    SubWidget *elem = sub[name];
    sub_layout->removeWidget(elem);
    sub.remove(name);
    delete elem;

    for (QMap<QString, InWidget*>::iterator i = in.begin() ; i != in.end() ; ++i) {
        i.value()->removeSub(i.key(), name);
    }
    for (QMap<QString, PostWidget*>::iterator i = post.begin() ; i != post.end() ; ++i) {
        i.value()->removeSub(i.key(), name);
    }
}

/*QString Widget::getDisplayElement(Backend::ElementType p_eElement, bool p_bUpperFirst) {
	switch (p_eElement) {
		case Backend::GAIN:
			return trUtf8("gain");
		case Backend::PAN_BAL:
			return trUtf8("pan/bal");
		case Backend::TO_PRE:
			return trUtf8("pre");
		case Backend::TO_POST:
			return trUtf8("post");
		case Backend::FADER:
			return trUtf8("fader");
		case Backend::PRE_VOL:
			return trUtf8("pre vol");
		case Backend::MUTE:
			return trUtf8("mute");
		case Backend::TO_SUB:
			return trUtf8("sub");
		case Backend::TO_MAIN:
			return trUtf8("main");
		case Backend::TO_ALF:
			return trUtf8("alf");
		case Backend::TO_PLF:
			return trUtf8("plf");
		case Backend::MUTE_EFFECT:
			return trUtf8("mute effect");
	}
	return "";
}*/

void Widget::leftClick(Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, QString p_sReatedChannelName, QString p_sDisplayReatedChannelName, QMouseEvent*)
{
	action(p_eType, p_sChannelName, p_eElement, p_sReatedChannelName, p_sDisplayReatedChannelName);
}

QString Widget::getDisplayNameOfChannel(Backend::ChannelType p_eType, QString p_sChannelName) {
	QString displayName;
    switch (p_eType) {
   		case Backend::IN:
            displayName = Backend::instance()->getInput(p_sChannelName)->display_name;
            break;
	    case Backend::OUT:
            displayName = Backend::instance()->getOutput(p_sChannelName)->display_name;
            break;
    	case Backend::PRE:
            displayName = Backend::instance()->getPre(p_sChannelName)->display_name;
            break;
	    case Backend::POST:
            displayName = Backend::instance()->getPost(p_sChannelName)->display_name;
            break;
    	case Backend::SUB:
            displayName = Backend::instance()->getSub(p_sChannelName)->display_name;
            break;
    }
    return displayName;
}
QString Widget::getDisplayChannelType(Backend::ChannelType p_eType, bool p_bUpperFirst) {
	QString displayName;
    switch (p_eType) {
   		case Backend::IN:
            displayName = trUtf8("input");
            break;
	    case Backend::OUT:
            displayName = trUtf8("output");
            break;
    	case Backend::PRE:
            displayName = trUtf8("pre-fader");
            break;
	    case Backend::POST:
            displayName = trUtf8("post-fader");
            break;
    	case Backend::SUB:
            displayName = trUtf8("sub-group");
            break;
    }
    return p_bUpperFirst ? displayName.left(1).toUpper() + displayName.right(displayName.size() - 1) : displayName;
}
QString Widget::getShortDisplayChannelType(Backend::ChannelType p_eType, bool p_bUpperFirst) {
	QString displayName;
    switch (p_eType) {
   		case Backend::IN:
            displayName = trUtf8("in");
            break;
	    case Backend::OUT:
            displayName = trUtf8("out");
            break;
    	case Backend::PRE:
            displayName = trUtf8("pre");
            break;
	    case Backend::POST:
            displayName = trUtf8("post");
            break;
    	case Backend::SUB:
            displayName = trUtf8("sub");
            break;
    }
    return p_bUpperFirst ? displayName.left(1).toUpper() + displayName.right(displayName.size() - 1) : displayName;
}

QString Widget::getDisplayFunction(Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, QString p_sReatedChannelName, bool p_bUpperFirst) {
	bool stereo = false;
    switch (p_eType) {
   		case Backend::IN:
            stereo = Backend::instance()->getInput(p_sChannelName)->stereo;
            break;
	    case Backend::OUT:
            stereo = Backend::instance()->getOutput(p_sChannelName)->stereo;
            break;
    	case Backend::PRE:
            stereo = Backend::instance()->getPre(p_sChannelName)->stereo;
            break;
	    case Backend::POST:
            stereo = Backend::instance()->getPost(p_sChannelName)->stereo;
            break;
    	case Backend::SUB:
            stereo = Backend::instance()->getSub(p_sChannelName)->stereo;
            break;
    }
    return getDisplayFunction(p_eElement, p_sReatedChannelName, stereo, p_bUpperFirst);
}
QString Widget::getDisplayFunction(Backend::ElementType p_eElement, QString p_sReatedChannelName, bool p_bStereo, bool p_bUpperFirst) {
	QString displayName;
    switch (p_eElement) {
    	case Backend::GAIN:
    		displayName = trUtf8("gain");
    		break;
    	case Backend::MUTE_EFFECT:
    	case Backend::MUTE:
    		displayName = trUtf8("mute");
    		break;
    	case Backend::PAN_BAL:
    		displayName = p_bStereo ? trUtf8("bal") : trUtf8("pan");
    		break;
    	case Backend::TO_PRE:
    		displayName = trUtf8("pre-fader \"%1\"").arg(Backend::instance()->getPre(p_sReatedChannelName)->display_name);
    		break;
    	case Backend::TO_POST:
    		displayName = trUtf8("post-fader \"%1\"").arg(Backend::instance()->getPost(p_sReatedChannelName)->display_name);
    		break;
    	case Backend::TO_SUB:
    		displayName = trUtf8("sub-group \"%1\"").arg(Backend::instance()->getSub(p_sReatedChannelName)->display_name);
    		break;
    	case Backend::TO_MAIN:
    		displayName = trUtf8("main");
    		break;
    	case Backend::FADER:
    		if (p_sReatedChannelName == MAIN) {
    			displayName = trUtf8("main volume");
    		}
    		else if (p_sReatedChannelName == PLF) {
    			displayName = trUtf8("phone volume");
    		}
    		else if (p_sReatedChannelName == MONO) {
    			displayName = trUtf8("mono volume");
    		}
    		else {
    			displayName = trUtf8("volume");
    		}
    		break;
    	case Backend::TO_ALF:
    		displayName = trUtf8("alf");
    		break;
    	case Backend::TO_PLF:
    		displayName = trUtf8("plf");
    		break;
    	case Backend::PRE_VOL:
    		displayName = trUtf8("pre volume");
    		break;
    }
    return p_bUpperFirst ? displayName.left(1).toUpper() + displayName.right(displayName.size() - 1) : displayName;
}
QString Widget::getShortDisplayFunction(Backend::ElementType p_eElement, QString p_sReatedChannelName, bool p_bStereo) {
	QString displayName;
    switch (p_eElement) {
    	case Backend::GAIN:
    		displayName = trUtf8("gain");
    		break;
    	case Backend::MUTE_EFFECT:
    	case Backend::MUTE:
    		displayName = trUtf8("M");
    		break;
    	case Backend::PAN_BAL:
    		displayName = p_bStereo ? trUtf8("bal") : trUtf8("pan");
    		break;
    	case Backend::TO_PRE:
    		displayName = trUtf8("pre");
    		break;
    	case Backend::TO_POST:
    		displayName = trUtf8("post");
    		break;
    	case Backend::TO_SUB:
    		displayName = "";
    		break;
    	case Backend::TO_MAIN:
    		displayName = trUtf8("LR");
    		break;
    	case Backend::FADER:
    		if (p_sReatedChannelName == MAIN) {
    			displayName = trUtf8("main");
    		}
    		else if (p_sReatedChannelName == PLF) {
    			displayName = trUtf8("phone");
    		}
    		else if (p_sReatedChannelName == MONO) {
    			displayName = trUtf8("mono");
    		}
    		else {
    			displayName = trUtf8("volume");
    		}
    		break;
    	case Backend::TO_ALF:
    		displayName = trUtf8("alf");
    		break;
    	case Backend::TO_PLF:
    		displayName = trUtf8("plf");
    		break;
    	case Backend::PRE_VOL:
    		displayName = trUtf8("pre vol");
    		break;
    }
    return displayName;	
}

void Widget::middleClick(Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, 
		QString p_sReatedChannelName, QString p_sDisplayReatedChannelName, QMouseEvent*)
{
	QKeySequence rActionOnChannelKeySequence;
	QKeySequence rSelectChannelKeySequence;
	QKeySequence rActionOnSelectedChannelKeySequence;
	foreach (QKeySequence rKey, m_mKeyToWrapp.keys()) {
		KeyDo* pKeyDo = m_mKeyToWrapp[rKey];
		if (typeid(*pKeyDo).name() == typeid(KeyDoDirectAction).name()) {
			KeyDoDirectAction* pKD = (KeyDoDirectAction*)pKeyDo;
			if (pKD->m_eType == p_eType && pKD->m_sChannelName == p_sChannelName
					&& pKD->m_eElement == p_eElement && pKD->m_sReatedChannelName == p_sReatedChannelName) {
				rActionOnChannelKeySequence = rKey;
			}
		}
		else if (typeid(*pKeyDo).name() == typeid(KeyDoSelectChannel).name()) {
			KeyDoSelectChannel* pKD = (KeyDoSelectChannel*)pKeyDo;
			if (pKD->m_eType == p_eType && pKD->m_sChannelName == p_sChannelName) {
				rSelectChannelKeySequence = rKey;
			}
		}
		else if (typeid(*pKeyDo).name() == typeid(KeyDoChannelAction).name()) {
			KeyDoChannelAction* pKD = (KeyDoChannelAction*)pKeyDo;
			if (pKD->m_eElement == p_eElement && pKD->m_sReatedChannelName == p_sReatedChannelName) {
				rActionOnSelectedChannelKeySequence = rKey;
			}
		}
	}
    // TODO assigne old key !
	bool volume = true;
	bool only_dirrect = false;
	switch (p_eElement) {
		case Backend::FADER:
			if (p_sChannelName == MAIN && p_sReatedChannelName != MAIN) {
				only_dirrect = true;
			}
		case Backend::GAIN:
		case Backend::PAN_BAL:
		case Backend::TO_PRE:
		case Backend::TO_POST:
		case Backend::PRE_VOL:
			break;
		case Backend::MUTE:
		case Backend::TO_SUB:
		case Backend::TO_MAIN:
		case Backend::TO_ALF:
		case Backend::TO_PLF:
		case Backend::MUTE_EFFECT:
			volume = false;
			break;
	}
	
	// get displayName
	QString displayName = getDisplayNameOfChannel(p_eType, p_sChannelName);
	
	QString displayElement = getDisplayFunction(p_eType, p_sChannelName, p_eElement, p_sReatedChannelName, false);
/*	if (p_sReatedChannelName != "") {
		displayElement += " " + p_sDisplayReatedChannelName;
	}*/
	
	AssigneToPannel* panel = new AssigneToPannel(displayName, displayElement, volume, only_dirrect, rActionOnChannelKeySequence, rSelectChannelKeySequence, rActionOnSelectedChannelKeySequence);;

    if (panel->exec() == QDialog::Accepted) {
    	if (panel->getActionOnChannelKeySequence() != QKeySequence()) {
    		if (panel->getActionOnChannelKeySequence() != rActionOnChannelKeySequence) {
	    		if ((!m_mKeyToWrapp.contains(panel->getActionOnChannelKeySequence())) || QMessageBox::question (this, trUtf8("Reassigne key")
	    				, trUtf8("Does I reassigne the %1 key ?").arg(panel->getActionOnChannelKeySequence().toString())
	    				, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)) {
	    			delete m_mKeyToWrapp[rActionOnChannelKeySequence];
	    			delete m_mKeyToWrapp[panel->getActionOnChannelKeySequence()];
		    		m_mKeyToWrapp.remove(rActionOnChannelKeySequence);
		    		m_mKeyToWrapp.insert(panel->getActionOnChannelKeySequence()
		    				, new KeyDoDirectAction(this, p_eType, p_sChannelName, p_eElement, p_sReatedChannelName, p_sDisplayReatedChannelName));
	    		}
    		}
    	}
    	else {
   			delete m_mKeyToWrapp[rActionOnChannelKeySequence];
    		m_mKeyToWrapp.remove(rActionOnChannelKeySequence);
    	}
    	if (panel->getSelectChannelKeySequence() != QKeySequence()) {
    		if (panel->getSelectChannelKeySequence() != rSelectChannelKeySequence) {
	    		if ((!m_mKeyToWrapp.contains(panel->getSelectChannelKeySequence())) || QMessageBox::question (this, trUtf8("Reassigne key")
	    				, trUtf8("Does I reassigne the %1 key").arg(panel->getSelectChannelKeySequence().toString())
	    				, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)) {
	    			delete m_mKeyToWrapp[rSelectChannelKeySequence];
	    			delete m_mKeyToWrapp[panel->getSelectChannelKeySequence()];
			    	m_mKeyToWrapp.remove(rSelectChannelKeySequence);
		    		m_mKeyToWrapp.insert(panel->getSelectChannelKeySequence(), new KeyDoSelectChannel(this, p_eType, p_sChannelName));
	    		}
    		}
    	}
    	else {
   			delete m_mKeyToWrapp[rSelectChannelKeySequence];
    		m_mKeyToWrapp.remove(rSelectChannelKeySequence);
    	}
    	if (panel->getActionOnSelectedChannelKeySequence() != QKeySequence()) {
    		if (panel->getActionOnSelectedChannelKeySequence() != rActionOnSelectedChannelKeySequence) {
	    		if ((!m_mKeyToWrapp.contains(panel->getActionOnSelectedChannelKeySequence())) || QMessageBox::question (this, trUtf8("Reassigne key")
	    				, trUtf8("Does I reassigne the %1 key").arg(panel->getActionOnSelectedChannelKeySequence().toString())
	    				, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)) {
	    			delete m_mKeyToWrapp[rActionOnSelectedChannelKeySequence];
	    			delete m_mKeyToWrapp[panel->getActionOnSelectedChannelKeySequence()];
			    	m_mKeyToWrapp.remove(rActionOnSelectedChannelKeySequence);
		    		m_mKeyToWrapp.insert(panel->getActionOnSelectedChannelKeySequence(), new KeyDoChannelAction(this, p_eElement, 
		    				p_sReatedChannelName, p_sDisplayReatedChannelName));
	    		}
    		}
    	}
    	else {
   			delete m_mKeyToWrapp[rActionOnSelectedChannelKeySequence];
	    	m_mKeyToWrapp.remove(rActionOnSelectedChannelKeySequence);
    	}
    }
}

void Widget::rightClick(Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, 
		QString p_sReatedChannelName, QString p_sDisplayReatedChannelName, QMouseEvent* ev)
{
    middleClick(p_eType, p_sChannelName, p_eElement, p_sReatedChannelName, p_sDisplayReatedChannelName, ev);
}

void Widget::action(Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, 
		QString p_sReatedChannelName, QString /*p_sDisplayReatedChannelName*/)
{
    if (m_mShurtCut.contains(p_eType) && m_mShurtCut[p_eType]->contains(p_sChannelName)
            && (*m_mShurtCut[p_eType])[p_sChannelName]->contains(p_eElement)
            && (*(*m_mShurtCut[p_eType])[p_sChannelName])[p_eElement]->contains(p_sReatedChannelName)) {
        Wrapp* pWrapp = (*(*(*m_mShurtCut[p_eType])[p_sChannelName])[p_eElement])[p_sReatedChannelName];
        if (!pWrapp->exec()) {
            m_pSelectedWrapper = (WrappVolume*)pWrapp;
            if (p_sChannelName == MAIN && p_sReatedChannelName == PLF) {
	            showMessage(trUtf8("Phono selected."));
            }
            else if (p_sChannelName == MAIN && p_sReatedChannelName == MONO) {
	            showMessage(trUtf8("Mono selected."));
            }
            else if (p_sChannelName == MAIN && p_sReatedChannelName == MAIN) {
	            showMessage(trUtf8("Main fader selected."));
            }
            else {
            	QString elem = getDisplayFunction(p_eType, p_sChannelName, p_eElement, p_sReatedChannelName);
/*            	elem = elem.left(1).toUpper() + elem.right(elem.size() - 1);
            	if (p_sReatedChannelName.size() > 0) {
            		elem += " ";
            	}*/

	            showMessage(trUtf8("%1 on channel %2 selected.").arg(elem)/*.arg(p_sDisplayReatedChannelName)*/
	            		.arg(getDisplayNameOfChannel(p_eType, p_sChannelName)));
            }
        } else {
            m_pSelectedWrapper = NULL;
        }
    }
}
void Widget::keyPressEvent ( QKeyEvent * p_pEvent )
{
	switch (p_pEvent->key()) {
		case Qt::Key_Home:
	        if (m_pSelectedWrapper != NULL) {
            	m_pSelectedWrapper->getVolume()->incValue(true, 10);
	        }
	        break;
		case Qt::Key_PageUp:
	        if (m_pSelectedWrapper != NULL) {
            	m_pSelectedWrapper->getVolume()->incValue(true, 4);
	        }
	        break;
		case Qt::Key_Up:
	        if (m_pSelectedWrapper != NULL) {
            	m_pSelectedWrapper->getVolume()->incValue(true, 1);
	        }
	        break;
		case Qt::Key_End:
	        if (m_pSelectedWrapper != NULL) {
            	m_pSelectedWrapper->getVolume()->incValue(false, 10);
	        }
	        break;
		case Qt::Key_PageDown:
	        if (m_pSelectedWrapper != NULL) {
            	m_pSelectedWrapper->getVolume()->incValue(false, 4);
	        }
	        break;
		case Qt::Key_Down:
	        if (m_pSelectedWrapper != NULL) {
            	m_pSelectedWrapper->getVolume()->incValue(false, 1);
	        }
	        break;
	    default:
		    if (!p_pEvent->isAutoRepeat()) {
		        QKeySequence keys = QKeySequence(p_pEvent->key()+p_pEvent->modifiers());
		        if (m_mKeyToWrapp.contains(keys)) {
		            m_mKeyToWrapp[keys]->action();
		        }
		    }
	}
}
void Widget::wheelEvent(QWheelEvent *p_pEvent)
{
    qDebug()<<(p_pEvent->delta() > 0);
    if (m_pSelectedWrapper != NULL) {
        m_pSelectedWrapper->getVolume()->incValue(p_pEvent->delta() > 0);
    }
}

void Widget::clearKeyToWrapp() {
qDebug()<<1111;
	foreach (KeyDo* keyDo, m_mKeyToWrapp) {
		delete keyDo;
	}
	m_mKeyToWrapp.clear();
	 
}

void Widget::addVolume(Volume* p_pVolume, Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, 
		QString p_sReatedChannelName, QString p_sDisplayReatedChannelName)
{
    p_pVolume->setToolTip(getDisplayFunction(p_eType, p_sChannelName, p_eElement, p_sReatedChannelName));

    if (!m_mShurtCut.contains(p_eType)) {
        m_mShurtCut.insert(p_eType, new QMap<QString, QMap<Backend::ElementType, QMap<QString, Wrapp*>*>*>());
    }
    if (!m_mShurtCut[p_eType]->contains(p_sChannelName)) {
        m_mShurtCut[p_eType]->insert(p_sChannelName, new QMap<Backend::ElementType, QMap<QString, Wrapp*>*>());
    }
    if (!(*m_mShurtCut[p_eType])[p_sChannelName]->contains(p_eElement)) {
        (*m_mShurtCut[p_eType])[p_sChannelName]->insert(p_eElement, new QMap<QString, Wrapp*>());
    }
    if (!(*(*m_mShurtCut[p_eType])[p_sChannelName])[p_eElement]->contains(p_sReatedChannelName)) {
	    (*(*m_mShurtCut[p_eType])[p_sChannelName])[p_eElement]->insert(p_sReatedChannelName,
	            new WrappVolume(this, p_pVolume, p_eType, p_sChannelName, p_eElement, p_sReatedChannelName, p_sDisplayReatedChannelName));
    }
    else {
    	for (int i = 2 ; i < 100 ; i++) {
    		QString name = QString(p_sReatedChannelName) + QString("_%1").arg(i);
		    if (!(*(*m_mShurtCut[p_eType])[p_sChannelName])[p_eElement]->contains(name)) {
	    		QString displayname = QString(p_sDisplayReatedChannelName) + QString(" %1").arg(i);
			    (*(*m_mShurtCut[p_eType])[p_sChannelName])[p_eElement]->insert(p_sReatedChannelName,
			            new WrappVolume(this, p_pVolume, p_eType, p_sChannelName, p_eElement, name, displayname));
			    break;
		    }
    	}
    }
}
void Widget::addToggle(ToggleButton* p_pVolume, Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, 
		QString p_sReatedChannelName, QString p_sDisplayReatedChannelName)
{
    p_pVolume->setToolTip(getDisplayFunction(p_eType, p_sChannelName, p_eElement, p_sReatedChannelName));
    p_pVolume->setText(getShortDisplayFunction(p_eElement, p_sReatedChannelName));
    
    if (!m_mShurtCut.contains(p_eType)) {
        m_mShurtCut.insert(p_eType, new QMap<QString, QMap<Backend::ElementType, QMap<QString, Wrapp*>*>*>());
    }
    if (!m_mShurtCut[p_eType]->contains(p_sChannelName)) {
        m_mShurtCut[p_eType]->insert(p_sChannelName, new QMap<Backend::ElementType, QMap<QString, Wrapp*>*>());
    }
    if (!(*m_mShurtCut[p_eType])[p_sChannelName]->contains(p_eElement)) {
        (*m_mShurtCut[p_eType])[p_sChannelName]->insert(p_eElement, new QMap<QString, Wrapp*>());
    }
    if (!(*(*m_mShurtCut[p_eType])[p_sChannelName])[p_eElement]->contains(p_sReatedChannelName)) {
	    (*(*m_mShurtCut[p_eType])[p_sChannelName])[p_eElement]->insert(p_sReatedChannelName,
    	        new WrappToggle(this, p_pVolume, p_eType, p_sChannelName, p_eElement, p_sReatedChannelName, p_sDisplayReatedChannelName));
    }
    else {
    	for (int i = 2 ; i < 100 ; i++) {
    		QString name = QString(p_sReatedChannelName) + QString("_%1").arg(i);
		    if (!(*(*m_mShurtCut[p_eType])[p_sChannelName])[p_eElement]->contains(name)) {
	    		QString displayname = QString(p_sDisplayReatedChannelName) + QString(" %1").arg(i);
			    (*(*m_mShurtCut[p_eType])[p_sChannelName])[p_eElement]->insert(name,
		    	        new WrappToggle(this, p_pVolume, p_eType, p_sChannelName, p_eElement, name, displayname));
		    	break;
		    }
    	}
    }
}
void Widget::removeShurtCut(Backend::ChannelType p_eType, QString p_sChannelName)
{
    if (m_mShurtCut.contains(p_eType) && m_mShurtCut[p_eType]->contains(p_sChannelName)) {
    	foreach(Backend::ElementType type, (*m_mShurtCut[p_eType])[p_sChannelName]->keys()) {
	    	foreach(QString sub, (*(*m_mShurtCut[p_eType])[p_sChannelName])[type]->keys()) {
	    		delete (*(*(*m_mShurtCut[p_eType])[p_sChannelName])[type])[sub];
	    		(*(*m_mShurtCut[p_eType])[p_sChannelName])[type]->remove(sub);
	    	}
    		delete (*(*m_mShurtCut[p_eType])[p_sChannelName])[type];
    		(*m_mShurtCut[p_eType])[p_sChannelName]->remove(type);
    	}
		delete (*m_mShurtCut[p_eType])[p_sChannelName];
		m_mShurtCut[p_eType]->remove(p_sChannelName);
    }
    foreach(QKeySequence key, m_mKeyToWrapp.keys()) {
		if (typeid(*m_mKeyToWrapp[key]).name() == typeid(KeyDoSelectChannel).name()) {
			KeyDoSelectChannel* keyDo = (KeyDoSelectChannel*)m_mKeyToWrapp[key];
			if (keyDo->m_eType == p_eType && keyDo->m_sChannelName == p_sChannelName) {
				delete keyDo;
				m_mKeyToWrapp.remove(key);
			}
		}
		else if (typeid(*m_mKeyToWrapp[key]).name() == typeid(KeyDoDirectAction).name()) {
			KeyDoDirectAction* keyDo = (KeyDoDirectAction*)m_mKeyToWrapp[key];
			if (keyDo->m_eType == p_eType && keyDo->m_sChannelName == p_sChannelName) {
				delete keyDo;
				m_mKeyToWrapp.remove(key);
			}
		}
    }
    
}
void Widget::showGain() {
	m_bShowGain = !m_bShowGain;
	foreach (Backend::ChannelType i, m_mShurtCut.keys()) {
		foreach (QString j, m_mShurtCut[i]->keys()) {
			if ((*m_mShurtCut[i])[j]->contains(Backend::GAIN) && (*(*m_mShurtCut[i])[j])[Backend::GAIN]->contains("")) {
				((WrappVolume*)(*(*(*m_mShurtCut[i])[j])[Backend::GAIN])[""])->getVolume()->setVisible(m_bShowGain);
			}
		}
	}
}
void Widget::setFaderHeight(int p_iHeight) {
	m_iFaderHeight = p_iHeight; 

	foreach (Backend::ChannelType i, m_mShurtCut.keys()) {
		foreach (QString j, m_mShurtCut[i]->keys()) {
			foreach (QString sub, (*(*m_mShurtCut[i])[j])[Backend::FADER]->keys()) {
				if (sub != PLF && sub != MONO) {
					Wrapp* w = (*(*(*m_mShurtCut[i])[j])[Backend::FADER])[sub];
					((Fader*)((WrappVolume*)w)->getVolume())->setFixedHeight(m_iFaderHeight);
				}
			}
		}
	}
}
void Widget::faderHeight() {
	bool ok;
	int result = QInputDialog::getInteger(this, trUtf8("Change the fader height"), trUtf8("New fader height"), m_iFaderHeight, 150, 500, 1, &ok);
	if (ok) {
		setFaderHeight(result);
	}
}

void Widget::setEffectFaderHeight(int p_iHeight) {
	m_iEffectFaderHeight = p_iHeight;

    m_pEffectScrollArea->setFixedHeight(m_iEffectFaderHeight + 102);
    m_pEffectStart->setFixedHeight(m_iEffectFaderHeight + 64);
	effect_layout->parentWidget()->setFixedHeight(m_iEffectFaderHeight + 82);

	foreach (QString channel, Backend::instance()->inchannels()) {
		foreach (effect* fx, *Backend::instance()->getInEffects(channel)) {
			if (fx->gui != NULL) {
				fx->gui->setFaderHeight(m_iEffectFaderHeight);
			}
		}
	}
	foreach (QString channel, Backend::instance()->prechannels()) {
		foreach (effect* fx, *Backend::instance()->getPreEffects(channel)) {
			if (fx->gui != NULL) {
				fx->gui->setFaderHeight(m_iEffectFaderHeight);
			}
		}
	}
	foreach (QString channel, Backend::instance()->postchannels()) {
		foreach (effect* fx, *Backend::instance()->getPostEffects(channel)) {
			if (fx->gui != NULL) {
				fx->gui->setFaderHeight(m_iEffectFaderHeight);
			}
		}
	}
	foreach (QString channel, Backend::instance()->subchannels()) {
		foreach (effect* fx, *Backend::instance()->getSubEffects(channel)) {
			if (fx->gui != NULL) {
				fx->gui->setFaderHeight(m_iEffectFaderHeight);
			}
		}
	}
	foreach (QString channel, Backend::instance()->outchannels()) {
		foreach (effect* fx, *Backend::instance()->getOutEffects(channel)) {
			if (fx->gui != NULL) {
				fx->gui->setFaderHeight(m_iEffectFaderHeight);
			}
		}
	}
}
void Widget::effectFaderHeight() {
	bool ok;
	int result = QInputDialog::getInteger(this, trUtf8("Change the effect fader height"), trUtf8("New effect fader height"), m_iEffectFaderHeight, 150, 300, 1, &ok);
	if (ok) {
		setEffectFaderHeight(result);
	}
}


void Widget::addLine(QVBoxLayout* layout, bool bold)
{
    /* QFrame* line = new QFrame();
     line->setFrameShape(QFrame::HLine);
     line->setFrameStyle(QFrame::Raised);
     layout->addWidget(line);*/
    QWidget* line = new QWidget;
    QPalette defaultPalette;
    defaultPalette.setColor( QPalette::Background, QColor( 128, 134, 152 ) );
    line->setPalette( defaultPalette );
    line->setFixedSize(CHANNEL_WIDTH, bold ? 3 : 2);
    line->setMinimumSize(CHANNEL_WIDTH, bold ? 3 : 2);
    layout->addWidget(line);
}

void Widget::addLine(QHBoxLayout* layout, bool bold)
{
    QWidget* line = new QWidget;
    QPalette defaultPalette;
    defaultPalette.setColor( QPalette::Background, QColor( 128, 134, 152 ) );
    line->setPalette( defaultPalette );
    line->setFixedSize(bold ? 3 : 2, bold ? 3 : 2);
    line->setMinimumSize(bold ? 3 : 2, bold ? 3 : 2);
    layout->addWidget(line);
}

void Widget::addSpacer(QVBoxLayout* layout)
{
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

/*Fader* Widget::createFader(Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, float p_fValue, QString p_sReatedChannelName) {
}
Rotary* Widget::createRotary(Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, float p_fValue, QString p_sReatedChannelName) {
    rotary = new Rotary(0, p_eElement == PAN_BAL ? Rotary::TYPE_CENTER : Rotary::TYPE_NORMAL, getDisplayFunction(p_eElement, p_sReatedChannelName, false), false, true, p_sChannelName);
    m_pMatrix->addVolume(rotary, p_eType, p_sChannelName, p_eElement, p_sReatedChannelName);
    rotary->setDbValue(value);
    return rotary;
}
ToggleButton* Widget::addToggle(Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, bool p_fValue, bool p_bStereo, QString p_sReatedChannelName) {
}*/

}
; //LiveMix
