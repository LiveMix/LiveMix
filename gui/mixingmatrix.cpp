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

#include "mixingmatrix.h"

#include "LadspaFXProperties.h"
#include "LadspaFXSelector.h"
#include "AssigneToPannel.h"
#include "Fader.h"
#include "FaderName.h"
#include "Button.h"
#include "Rotary.h"

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
#include <QAction>
#include <QPainter>


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
		, m_iFaderHeight(300)
		, m_iEffectFaderHeight(200)        
{
    QVBoxLayout *main_layout = new QVBoxLayout;
    main_layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	main_layout->setSizeConstraint(QLayout::SetMinimumSize);

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
    
    info_widget = new InfoWidget(this);
    mix_layout->addWidget(info_widget);
    
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
    connect( main_widget, SIGNAL( clicked(ChannelType, QString) ), this, SLOT( select(ChannelType, QString) ) );

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

void Widget::displayFX(effect *fx, ChannelType p_eType, QString p_sChannelName)
{
    if (fx->gui == NULL)
    {
        fx->gui = new LadspaFXProperties(NULL, fx);
        fx->gui->setFaderHeight(m_iEffectFaderHeight);
	    addToggle(fx->gui->getActivateButton(), p_eType, p_sChannelName, MUTE_EFFECT, fx->fx->getPluginLabel());
        effect_layout->addWidget(fx->gui);
        connect(fx->gui, SIGNAL(removeClicked(LadspaFXProperties*, effect*)), this, SLOT(askRemoveFX(LadspaFXProperties*, effect*)));

	    channel* c = Backend::instance()->getChannel(p_eType, p_sChannelName);
	    if (!c->effectsMap.contains(fx->fx->getPluginLabel())) {
	    	c->effectsMap[fx->fx->getPluginLabel()] = fx;
			fx->displayname = fx->fx->getPluginName();
	    }
	    else {
	    	for (int i = 2 ; i < 100 ; i++) {
				QString name = QString(fx->fx->getPluginLabel()) + QString("_%1").arg(i);
			    if (!c->effectsMap.contains(name)) {
			    	fx->displayname = QString(fx->fx->getPluginName()) + QString(" %1").arg(i);
			    	c->effectsMap[name] = fx;
				    break;
			    }
	    	}
		}
    }
    fx->gui->show();
    m_lVisibleEffect << fx;
}

void Widget::askRemoveFX(LadspaFXProperties* widget, effect* fx)
{
	if (QMessageBox::question(this, trUtf8("Remove effect"), trUtf8("Are you shure that you want to remove en effect"), 
			QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
		removeFX(widget, fx);
	}
}
void Widget::removeFX(LadspaFXProperties* widget, effect* fx)
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
    case IN: {
            Backend::instance()->removeInEffect(m_sSelectChannel, fx);
            break;
        }
    case OUT: {
            Backend::instance()->removeOutEffect(m_sSelectChannel, fx);
            break;
        }
    case PRE: {
            Backend::instance()->removePreEffect(m_sSelectChannel, fx);
            break;
        }
    case POST: {
            Backend::instance()->removePostEffect(m_sSelectChannel, fx);
            break;
        }
    case SUB: {
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
        effect* elem = NULL;
        switch (m_eSelectType) {
        case IN: {
                elem = Backend::instance()->addInEffect(m_sSelectChannel, fx);
                break;
            }
        case OUT: {
                elem = Backend::instance()->addOutEffect(m_sSelectChannel, fx);
                break;
            }
        case PRE: {
                elem = Backend::instance()->addPreEffect(m_sSelectChannel, fx);
                break;
            }
        case POST: {
                elem = Backend::instance()->addPostEffect(m_sSelectChannel, fx);
                break;
            }
        case SUB: {
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
    m_pStatusLabel->setText("");
}

void Widget::select(ChannelType type, QString channel)
{
    doSelect(type, channel);
}
void Widget::doSelect(ChannelType type, QString channel)
{
// effect = new effectData;
    if (m_eSelectType == type && m_sSelectChannel == channel) {
        return;
    }
    m_eSelectType = type;
    m_sSelectChannel = channel;

    effectName->setText(channel);
//    m_pEffectStart->show();

    foreach (effect* fx, m_lVisibleEffect) {
        fx->gui->hide();
    }
    m_lVisibleEffect.clear();

    switch (m_eSelectType) {
    case IN: {
            showMessage(trUtf8("Input \"%1\" selected.").arg(Backend::instance()->getInput(m_sSelectChannel)->display_name));
            foreach (effect* elem, *(Backend::instance()->getInEffects(m_sSelectChannel))) {
                displayFX(elem, m_eSelectType, m_sSelectChannel);
            }
            break;
        }
    case OUT: {
            showMessage(trUtf8("Output selected."));
            foreach (effect* elem, *(Backend::instance()->getOutEffects(m_sSelectChannel))) {
                displayFX(elem, m_eSelectType, m_sSelectChannel);
            }
            break;
        }
    case PRE: {
            showMessage(trUtf8("Pre fader aux \"%1\" selected.").arg(Backend::instance()->getPre(m_sSelectChannel)->display_name));
            foreach (effect* elem, *(Backend::instance()->getPreEffects(m_sSelectChannel))) {
                displayFX(elem, m_eSelectType, m_sSelectChannel);
            }
            break;
        }
    case POST: {
            showMessage(trUtf8("Post fader aux \"%1\" selected.").arg(Backend::instance()->getPost(m_sSelectChannel)->display_name));
            foreach (effect* elem, *(Backend::instance()->getPostEffects(m_sSelectChannel))) {
                displayFX(elem, m_eSelectType, m_sSelectChannel);
            }
            break;
        }
    case SUB: {
            showMessage(trUtf8("Sub-groupe \"%1\" selected.").arg(Backend::instance()->getSub(m_sSelectChannel)->display_name));
            foreach (effect* elem, *(Backend::instance()->getSubEffects(m_sSelectChannel))) {
                displayFX(elem, m_eSelectType, m_sSelectChannel);
            }
            break;
        }
    }
    effect_layout->parentWidget()->resize(effect_layout->parentWidget()->minimumSize());
}

ChannelType Widget::getSelectedChanelType()
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
            FWidget* fader = in[in_name]->fader;
            fader->getFader()->setDbPeak_L(Backend::instance()->getInPeak(in_name, true));
            fader->getFader()->setDbPeak_R(Backend::instance()->getInPeak(in_name, false));
        }
    }
    foreach (QString pre_name, Backend::instance()->prechannels()) {
        if (pre[pre_name] != NULL) {
            FWidget* fader = pre[pre_name]->fader;
            fader->getFader()->setDbPeak_L(Backend::instance()->getPrePeak(pre_name, true));
            fader->getFader()->setDbPeak_R(Backend::instance()->getPrePeak(pre_name, false));
        }
    }
    foreach (QString post_name, Backend::instance()->postchannels()) {
        if (post[post_name] != NULL) {
            FWidget* fader = post[post_name]->fader;
            fader->getFader()->setDbPeak_L(Backend::instance()->getPostPeak(post_name, true));
            fader->getFader()->setDbPeak_R(Backend::instance()->getPostPeak(post_name, false));
        }
    }
    foreach (QString sub_name, Backend::instance()->subchannels()) {
        if (sub[sub_name] != NULL) {
            FWidget* fader = sub[sub_name]->fader;
            fader->getFader()->setDbPeak_L(Backend::instance()->getSubPeak(sub_name, true));
            fader->getFader()->setDbPeak_R(Backend::instance()->getSubPeak(sub_name, false));
        }
    }
    main_widget->fader->getFader()->setDbPeak_L(Backend::instance()->getOutPeak(MAIN, true));
    main_widget->fader->getFader()->setDbPeak_R(Backend::instance()->getOutPeak(MAIN, false));
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
    doSelect(OUT, MAIN);
}

void Widget::addinchannel( QString name, bool related )
{
    InWidget* elem = new InWidget(name, this);
    in_layout->addWidget(elem);
    in[name] = elem;
    connect( elem, SIGNAL( clicked(ChannelType, QString) ), this, SLOT( select(ChannelType, QString) ) );

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
    connect( elem, SIGNAL( clicked(ChannelType, QString) ), this, SLOT( select(ChannelType, QString) ) );

	info_widget->addPre(name);
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
    connect( elem, SIGNAL( clicked(ChannelType, QString) ), this, SLOT( select(ChannelType, QString) ) );

	info_widget->addPost(name);
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
    connect( elem, SIGNAL( clicked(ChannelType, QString) ), this, SLOT( select(ChannelType, QString) ) );

	info_widget->addSub(name);
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

	m_bVisible[TO_PRE]->remove(name);

	info_widget->removePre(name);
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

	m_bVisible[TO_POST]->remove(name);

	info_widget->removePost(name);
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

	m_bVisible[TO_SUB]->remove(name);

	info_widget->removeSub(name);
    for (QMap<QString, InWidget*>::iterator i = in.begin() ; i != in.end() ; ++i) {
        i.value()->removeSub(i.key(), name);
    }
    for (QMap<QString, PostWidget*>::iterator i = post.begin() ; i != post.end() ; ++i) {
        i.value()->removeSub(i.key(), name);
    }
}

void Widget::clearAll() {
	typedef QMap<QString, bool>* Map; 
	foreach (Map map, m_bVisible.values()) {
		map->empty();
	}
}

void Widget::leftClick(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_sReatedChannelName, QMouseEvent*)
{
	action(p_eType, p_sChannelName, p_eElement, p_sReatedChannelName);
}

QString Widget::getDisplayNameOfChannel(ChannelType p_eType, QString p_sChannelName) {
	QString displayName;
    switch (p_eType) {
   		case IN:
            displayName = Backend::instance()->getInput(p_sChannelName)->display_name;
            break;
	    case OUT:
            displayName = Backend::instance()->getOutput(p_sChannelName)->display_name;
            break;
    	case PRE:
            displayName = Backend::instance()->getPre(p_sChannelName)->display_name;
            break;
	    case POST:
            displayName = Backend::instance()->getPost(p_sChannelName)->display_name;
            break;
    	case SUB:
            displayName = Backend::instance()->getSub(p_sChannelName)->display_name;
            break;
    }
    return displayName;
}
QString Widget::getDisplayChannelType(ChannelType p_eType, bool p_bUpperFirst) {
	QString displayName;
    switch (p_eType) {
   		case IN:
            displayName = trUtf8("input");
            break;
	    case OUT:
            displayName = trUtf8("output");
            break;
    	case PRE:
            displayName = trUtf8("pre-fader");
            break;
	    case POST:
            displayName = trUtf8("post-fader");
            break;
    	case SUB:
            displayName = trUtf8("sub-group");
            break;
    }
    return p_bUpperFirst ? displayName.left(1).toUpper() + displayName.right(displayName.size() - 1) : displayName;
}
QString Widget::getShortDisplayChannelType(ChannelType p_eType, bool p_bUpperFirst) {
	QString displayName;
    switch (p_eType) {
   		case IN:
            displayName = trUtf8("in");
            break;
	    case OUT:
            displayName = trUtf8("out");
            break;
    	case PRE:
            displayName = trUtf8("pre");
            break;
	    case POST:
            displayName = trUtf8("post");
            break;
    	case SUB:
            displayName = trUtf8("sub");
            break;
    }
    return p_bUpperFirst ? displayName.left(1).toUpper() + displayName.right(displayName.size() - 1) : displayName;
}

QString Widget::getDisplayFunction(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_sReatedChannelName, bool p_bUpperFirst) {
	bool stereo = false;
	if (p_sChannelName != "") {
	    switch (p_eType) {
	   		case IN:
	            stereo = Backend::instance()->getInput(p_sChannelName)->stereo;
	            break;
		    case OUT:
	            stereo = Backend::instance()->getOutput(p_sChannelName)->stereo;
	            break;
	    	case PRE:
	            stereo = Backend::instance()->getPre(p_sChannelName)->stereo;
	            break;
		    case POST:
	            stereo = Backend::instance()->getPost(p_sChannelName)->stereo;
	            break;
	    	case SUB:
	            stereo = Backend::instance()->getSub(p_sChannelName)->stereo;
	            break;
	    }
	}

	QString displayName;
    switch (p_eElement) {
    	case GAIN:
    		displayName = trUtf8("gain");
    		break;
    	case MUTE_EFFECT:
    		displayName = trUtf8("mute effect \"%1\"").arg(Backend::instance()->getChannel(p_eType, p_sChannelName)->
    				effectsMap[p_sReatedChannelName]->displayname);
    		break;
    	case MUTE:
    		displayName = trUtf8("mute");
    		break;
    	case PAN_BAL:
    		displayName = p_sChannelName == "" ? trUtf8("pan/bal") : stereo ? trUtf8("bal") : trUtf8("pan");
    		break;
    	case TO_PRE:
    		displayName = trUtf8("pre-fader \"%1\"").arg(Backend::instance()->getPre(p_sReatedChannelName)->display_name);
    		break;
    	case TO_POST:
    		displayName = trUtf8("post-fader \"%1\"").arg(Backend::instance()->getPost(p_sReatedChannelName)->display_name);
    		break;
    	case TO_SUB:
    		displayName = trUtf8("sub-group \"%1\"").arg(Backend::instance()->getSub(p_sReatedChannelName)->display_name);
    		break;
    	case TO_MAIN:
    		displayName = trUtf8("main");
    		break;
    	case FADER:
    		if (p_sReatedChannelName == MAIN) {
    			displayName = trUtf8("main volume");
    		}
    		else if (p_sReatedChannelName == PFL) {
    			displayName = trUtf8("phone volume");
    		}
    		else if (p_sReatedChannelName == MONO) {
    			displayName = trUtf8("mono volume");
    		}
    		else {
    			displayName = trUtf8("volume");
    		}
    		break;
    	case TO_AFL:
            displayName = trUtf8("after-fader listen");
            break;
        case TO_PFL:
            displayName = trUtf8("pre-fader listen");
    		break;
    	case PRE_VOL:
    		displayName = trUtf8("pre volume");
    		break;
    }
    return p_bUpperFirst ? displayName.left(1).toUpper() + displayName.right(displayName.size() - 1) : displayName;
}
QString Widget::getMediumDisplayFunction(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_sReatedChannelName, bool p_bUpperFirst) {
	bool stereo = false;
	if (p_sChannelName != "") {
	    switch (p_eType) {
	   		case IN:
	            stereo = Backend::instance()->getInput(p_sChannelName)->stereo;
	            break;
		    case OUT:
	            stereo = Backend::instance()->getOutput(p_sChannelName)->stereo;
	            break;
	    	case PRE:
	            stereo = Backend::instance()->getPre(p_sChannelName)->stereo;
	            break;
		    case POST:
	            stereo = Backend::instance()->getPost(p_sChannelName)->stereo;
	            break;
	    	case SUB:
	            stereo = Backend::instance()->getSub(p_sChannelName)->stereo;
	            break;
	    }
	}

	QString displayName;
    switch (p_eElement) {
    	case GAIN:
    		displayName = trUtf8("gain");
    		break;
    	case MUTE_EFFECT:
    		displayName = trUtf8("mute %1").arg(Backend::instance()->getChannel(p_eType, p_sChannelName)->
    				effectsMap[p_sReatedChannelName]->displayname);
    		break;
    	case MUTE:
    		displayName = trUtf8("mute");
    		break;
    	case PAN_BAL:
    		displayName = p_sChannelName == "" ? trUtf8("pan/bal") : stereo ? trUtf8("bal") : trUtf8("pan");
    		break;
    	case TO_PRE:
    		displayName = trUtf8("pre %1").arg(p_sReatedChannelName);
    		break;
    	case TO_POST:
    		displayName = trUtf8("post %1").arg(p_sReatedChannelName);
    		break;
    	case TO_SUB:
    		displayName = trUtf8("%1").arg(p_sReatedChannelName);
    		break;
    	case TO_MAIN:
    		displayName = trUtf8("main");
    		break;
    	case FADER:
    		if (p_sReatedChannelName == MAIN) {
    			displayName = trUtf8("main volume");
    		}
    		else if (p_sReatedChannelName == PFL) {
    			displayName = trUtf8("phone volume");
    		}
    		else if (p_sReatedChannelName == MONO) {
    			displayName = trUtf8("mono volume");
    		}
    		else {
    			displayName = trUtf8("volume");
    		}
    		break;
    	case TO_AFL:
    		displayName = trUtf8("afl");
    		break;
    	case TO_PFL:
    		displayName = trUtf8("pfl");
    		break;
    	case PRE_VOL:
    		displayName = trUtf8("pre volume");
    		break;
    }
    return p_bUpperFirst ? displayName.left(1).toUpper() + displayName.right(displayName.size() - 1) : displayName;
}
QString Widget::getShortDisplayFunction(ElementType p_eElement, QString p_sReatedChannelName, bool p_bStereo) {
	QString displayName;
    switch (p_eElement) {
    	case GAIN:
    		displayName = trUtf8("gain");
    		break;
    	case MUTE_EFFECT:
    	case MUTE:
    		displayName = trUtf8("M");
    		break;
    	case PAN_BAL:
    		displayName = p_bStereo ? trUtf8("bal") : trUtf8("pan");
    		break;
    	case TO_PRE:
    		displayName = trUtf8("pre");
    		break;
    	case TO_POST:
    		displayName = trUtf8("post");
    		break;
    	case TO_SUB:
    		displayName = "";
    		break;
    	case TO_MAIN:
    		displayName = trUtf8("LR");
    		break;
    	case FADER:
    		if (p_sReatedChannelName == MAIN) {
    			displayName = trUtf8("main");
    		}
    		else if (p_sReatedChannelName == PFL) {
    			displayName = trUtf8("phone");
    		}
    		else if (p_sReatedChannelName == MONO) {
    			displayName = trUtf8("mono");
    		}
    		else {
    			displayName = trUtf8("volume");
    		}
    		break;
    	case TO_AFL:
            displayName = trUtf8("afl");
            break;
        case TO_PFL:
            displayName = trUtf8("pfl");
    		break;
    	case PRE_VOL:
    		displayName = trUtf8("pre vol");
    		break;
    }
    return displayName;	
}

void Widget::middleClick(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, 
		QString p_sReatedChannelName, QMouseEvent*)
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
		case FADER:
			if (p_sChannelName == MAIN && p_sReatedChannelName != MAIN) {
				only_dirrect = true;
			}
		case GAIN:
		case PAN_BAL:
		case TO_PRE:
		case TO_POST:
		case PRE_VOL:
			break;
		case MUTE:
		case TO_SUB:
		case TO_MAIN:
		case TO_AFL:
		case TO_PFL:
		case MUTE_EFFECT:
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
		    				, new KeyDoDirectAction(this, p_eType, p_sChannelName, p_eElement, p_sReatedChannelName));
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
		    				p_sReatedChannelName));
	    		}
    		}
    	}
    	else {
   			delete m_mKeyToWrapp[rActionOnSelectedChannelKeySequence];
	    	m_mKeyToWrapp.remove(rActionOnSelectedChannelKeySequence);
    	}
    }
}

void Widget::rightClick(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, 
		QString p_sReatedChannelName, QMouseEvent* p_pEvent)
{
    doSelect(p_eType, p_sChannelName);
    m_eSelectedElement = p_eElement;
    m_sSelectedReatedChannelName = p_sReatedChannelName;

    QMenu menu(this);
    
    QAction* assigne = new QAction(trUtf8("Assigne key"), this);
    connect(assigne, SIGNAL(triggered()), this, SLOT(assigneKey()));
    menu.addAction(assigne);
    
    switch (p_eElement) {
        case MUTE:
        case TO_SUB:
        case TO_MAIN:
        case TO_PFL:
        case TO_AFL:
        {
            QAction* enable = new QAction(trUtf8("Enable all %1").arg(
                    getMediumDisplayFunction(p_eType, p_sChannelName, p_eElement, p_sReatedChannelName, false)), this);
            connect(enable, SIGNAL(triggered()), this, SLOT(enableAllTheLine()));
            menu.addAction(enable);

            QAction* desable = new QAction(trUtf8("Desable all %1").arg(
                    getMediumDisplayFunction(p_eType, p_sChannelName, p_eElement, p_sReatedChannelName, false)), this);
            connect(desable, SIGNAL(triggered()), this, SLOT(desableAllTheLine()));
            menu.addAction(desable);

            break;
        }
        default:
            if (p_eElement == PAN_BAL) {
                QAction* restet = new QAction(trUtf8("Reset %1").arg(
                        getMediumDisplayFunction(p_eType, p_sChannelName, p_eElement, p_sReatedChannelName, false)), this);
                connect(restet, SIGNAL(triggered()), this, SLOT(resetAllTheLine()));
                menu.addAction(restet);
            }
            QAction* restet = new QAction(trUtf8("Reset all %1").arg(
                    getMediumDisplayFunction(p_eType, p_sChannelName, p_eElement, p_sReatedChannelName, false)), this);
            connect(restet, SIGNAL(triggered()), this, SLOT(resetAllTheLine()));
            menu.addAction(restet);
    }
    menu.exec(p_pEvent->globalPos());
}
void Widget::assigneKey() {
    middleClick(m_eSelectType, m_sSelectChannel, m_eSelectedElement, m_sSelectedReatedChannelName, NULL);
}
void Widget::resetAllTheLine() {
    foreach (ChannelType i, m_mShurtCut.keys()) {
        foreach (QString j, m_mShurtCut[i]->keys()) {
            if ((*m_mShurtCut[i])[j]->contains(m_eSelectedElement) && (*(*m_mShurtCut[i])[j])[m_eSelectedElement]->contains(m_sSelectedReatedChannelName)) {
                Volume* vol = ((WrappVolume*)(*(*(*m_mShurtCut[i])[j])[m_eSelectedElement])[m_sSelectedReatedChannelName])->getVolume();
                switch (m_eSelectedElement) {
                    case PAN_BAL:
                        vol->setValue(0, true);
                        break;
                    default:
                        vol->setValue(vol->getMinValue(), true);
                }
            }
        }
    }
}
void Widget::enableAllTheLine() {
    foreach (ChannelType i, m_mShurtCut.keys()) {
        foreach (QString j, m_mShurtCut[i]->keys()) {
            if ((*m_mShurtCut[i])[j]->contains(m_eSelectedElement) && (*(*m_mShurtCut[i])[j])[m_eSelectedElement]->contains(m_sSelectedReatedChannelName)) {
                Toggle* vol = ((WrappToggle*)(*(*(*m_mShurtCut[i])[j])[m_eSelectedElement])[m_sSelectedReatedChannelName])->getToggle();
                vol->setValue(true, true);
            }
        }
    }
}
void Widget::desableAllTheLine() {
    foreach (ChannelType i, m_mShurtCut.keys()) {
        foreach (QString j, m_mShurtCut[i]->keys()) {
            if ((*m_mShurtCut[i])[j]->contains(m_eSelectedElement) && (*(*m_mShurtCut[i])[j])[m_eSelectedElement]->contains(m_sSelectedReatedChannelName)) {
                Toggle* vol = ((WrappToggle*)(*(*(*m_mShurtCut[i])[j])[m_eSelectedElement])[m_sSelectedReatedChannelName])->getToggle();
                vol->setValue(false, true);
            }
        }
    }
}

void Widget::action(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, 
		QString p_sReatedChannelName)
{
    if (m_mShurtCut.contains(p_eType) && m_mShurtCut[p_eType]->contains(p_sChannelName)
            && (*m_mShurtCut[p_eType])[p_sChannelName]->contains(p_eElement)
            && (*(*m_mShurtCut[p_eType])[p_sChannelName])[p_eElement]->contains(p_sReatedChannelName)) {
        Wrapp* pWrapp = (*(*(*m_mShurtCut[p_eType])[p_sChannelName])[p_eElement])[p_sReatedChannelName];
        if (!pWrapp->exec()) {
            m_pSelectedWrapper = (WrappVolume*)pWrapp;
            if (p_sChannelName == MAIN && p_sReatedChannelName == PFL) {
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

void Widget::addVolume(Volume* p_pVolume, ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_sReatedChannelName)
{
    if (!m_mShurtCut.contains(p_eType)) {
        m_mShurtCut.insert(p_eType, new QMap<QString, QMap<ElementType, QMap<QString, Wrapp*>*>*>());
    }
    if (!m_mShurtCut[p_eType]->contains(p_sChannelName)) {
        m_mShurtCut[p_eType]->insert(p_sChannelName, new QMap<ElementType, QMap<QString, Wrapp*>*>());
    }
    if (!(*m_mShurtCut[p_eType])[p_sChannelName]->contains(p_eElement)) {
        (*m_mShurtCut[p_eType])[p_sChannelName]->insert(p_eElement, new QMap<QString, Wrapp*>());
    }
    if (!(*(*m_mShurtCut[p_eType])[p_sChannelName])[p_eElement]->contains(p_sReatedChannelName)) {
	    (*(*m_mShurtCut[p_eType])[p_sChannelName])[p_eElement]->insert(p_sReatedChannelName,
	            new WrappVolume(this, p_pVolume, p_eType, p_sChannelName, p_eElement, p_sReatedChannelName));
    }
    else {
    	for (int i = 2 ; i < 100 ; i++) {
    		QString name = QString(p_sReatedChannelName) + QString("_%1").arg(i);
		    if (!(*(*m_mShurtCut[p_eType])[p_sChannelName])[p_eElement]->contains(name)) {
			    (*(*m_mShurtCut[p_eType])[p_sChannelName])[p_eElement]->insert(p_sReatedChannelName,
			            new WrappVolume(this, p_pVolume, p_eType, p_sChannelName, p_eElement, name));
			    break;
		    }
    	}
    }
}
void Widget::addToggle(Toggle* p_pVolume, ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, 
		QString p_sReatedChannelName)
{
    if (!m_mShurtCut.contains(p_eType)) {
        m_mShurtCut.insert(p_eType, new QMap<QString, QMap<ElementType, QMap<QString, Wrapp*>*>*>());
    }
    if (!m_mShurtCut[p_eType]->contains(p_sChannelName)) {
        m_mShurtCut[p_eType]->insert(p_sChannelName, new QMap<ElementType, QMap<QString, Wrapp*>*>());
    }
    if (!(*m_mShurtCut[p_eType])[p_sChannelName]->contains(p_eElement)) {
        (*m_mShurtCut[p_eType])[p_sChannelName]->insert(p_eElement, new QMap<QString, Wrapp*>());
    }
    if (!(*(*m_mShurtCut[p_eType])[p_sChannelName])[p_eElement]->contains(p_sReatedChannelName)) {
	    (*(*m_mShurtCut[p_eType])[p_sChannelName])[p_eElement]->insert(p_sReatedChannelName,
    	        new WrappToggle(this, p_pVolume, p_eType, p_sChannelName, p_eElement, p_sReatedChannelName));
    }
    else {
    	for (int i = 2 ; i < 100 ; i++) {
    		QString name = QString(p_sReatedChannelName) + QString("_%1").arg(i);
		    if (!(*(*m_mShurtCut[p_eType])[p_sChannelName])[p_eElement]->contains(name)) {
			    (*(*m_mShurtCut[p_eType])[p_sChannelName])[p_eElement]->insert(name,
		    	        new WrappToggle(this, p_pVolume, p_eType, p_sChannelName, p_eElement, name));
		    	break;
		    }
    	}
    }
}
void Widget::removeShurtCut(ChannelType p_eType, QString p_sChannelName)
{
    if (m_mShurtCut.contains(p_eType) && m_mShurtCut[p_eType]->contains(p_sChannelName)) {
    	foreach(ElementType type, (*m_mShurtCut[p_eType])[p_sChannelName]->keys()) {
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
bool Widget::isVisible(ElementType p_eElement, QString p_rChannelTo) {
	if (m_bVisible.contains(p_eElement)) {
		if (m_bVisible[p_eElement]->contains(p_rChannelTo)) {
			return (*m_bVisible[p_eElement])[p_rChannelTo];
		}
		else {
			return true;
		}
	}
	else {
		return true;
	}
}
void Widget::setVisible(bool p_bVisible, ElementType p_eElement, QString p_rChannelTo) {
	if (!m_bVisible.contains(p_eElement)) {
		m_bVisible.insert(p_eElement, new QMap<QString, bool>);
	}
	m_bVisible[p_eElement]->insert(p_rChannelTo, p_bVisible);
	
	int height = info_widget->setVisible(p_bVisible, p_eElement, p_rChannelTo);

	foreach (ChannelType i, m_mShurtCut.keys()) {
		foreach (QString j, m_mShurtCut[i]->keys()) {
			if ((*m_mShurtCut[i])[j]->contains(p_eElement) && (*(*m_mShurtCut[i])[j])[p_eElement]->contains(p_rChannelTo)) {
				((WrappVolume*)(*(*(*m_mShurtCut[i])[j])[p_eElement])[p_rChannelTo])->getVolume()->setVisible(p_bVisible);
			}
		}
	}
	if (p_eElement == TO_PFL) {
		foreach (ChannelType i, m_mShurtCut.keys()) {
			foreach (QString j, m_mShurtCut[i]->keys()) {
				if ((*m_mShurtCut[i])[j]->contains(TO_AFL) && (*(*m_mShurtCut[i])[j])[TO_AFL]->contains(p_rChannelTo)) {
					((WrappVolume*)(*(*(*m_mShurtCut[i])[j])[TO_AFL])[p_rChannelTo])->getVolume()->setVisible(p_bVisible);
				}
			}
		}
	}

	if (!p_bVisible) {
	    QRect size = this->parentWidget()->geometry();
	    size.setHeight(size.height() + height);
		this->parentWidget()->layout()->invalidate();
		parentWidget()->setGeometry(size);
	}
/*qDebug()<<this->parentWidget()->minimumHeight();
this->parentWidget()->updateGeometry();
qDebug()<<this->parentWidget()->minimumHeight();
this->parentWidget()->layout()->update();
qDebug()<<this->parentWidget()->minimumHeight();
qDebug()<<this->parentWidget()->layout()->activate();
qDebug()<<this->parentWidget()->minimumHeight();
qDebug()<<this->parentWidget()->layout()->minimumSize();
this->parentWidget()->layout()->invalidate();
qDebug()<<this->parentWidget()->minimumHeight();
qDebug()<<this->parentWidget()->layout()->minimumSize();
qDebug()<<this->parentWidget()->layout()->activate();
qDebug()<<this->parentWidget()->minimumHeight();
qDebug()<<this->parentWidget()->layout()->minimumSize();*/
//	this->parentWidget()->layout()->invalidate();
//qDebug()<<1111;
//qDebug()<<this->parentWidget()->height();
//qDebug()<<this->parentWidget()->minimumHeight();
//	this->parentWidget()->setBaseSize(this->parentWidget()->minimumHeight(), this->parentWidget()->width());
//	this->parentWidget()->resize(this->parentWidget()->minimumHeight(), this->parentWidget()->width());
//	this->parentWidget()->setFixedHeight(this->parentWidget()->minimumHeight());
}
//void Widget::showGain() {
//	setGainVisible(!m_bShowGain);
//}
void Widget::setFaderHeight(int p_iHeight) {
    int diff = m_iFaderHeight - p_iHeight;
    if (p_iHeight < 200) {
        p_iHeight = 200;
    }
	m_iFaderHeight = p_iHeight; 

    info_widget->m_pFader->setFixedHeight(m_iFaderHeight);

	foreach (ChannelType i, m_mShurtCut.keys()) {
		foreach (QString j, m_mShurtCut[i]->keys()) {
			foreach (QString sub, (*(*m_mShurtCut[i])[j])[FADER]->keys()) {
				if (sub != PFL && sub != MONO) {
					Wrapp* w = (*(*(*m_mShurtCut[i])[j])[FADER])[sub];
					((Fader*)((WrappVolume*)w)->getVolume())->setFixedHeight(m_iFaderHeight);
				}
			}
		}
	}

    if (diff > 0) {
        QRect size = this->parentWidget()->geometry();
        size.setHeight(size.height() - diff);
        this->parentWidget()->layout()->invalidate();
        parentWidget()->setGeometry(size);
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

void Widget::addSpacer(QVBoxLayout* layout)
{
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

FWidget::FWidget(int p_fFaderHeignt, ChannelType p_eType, QString p_sChannelName)
 : m_eType(p_eType)
 , m_sChannelName(p_sChannelName) {
    m_pVolume = new Fader(this, false, false);
    m_pLabelFader = new FaderName(this);
    
    setFixedSize(CHANNEL_WIDTH, p_fFaderHeignt);
    setFixedHeight(p_fFaderHeignt);
    m_pVolume->move(CHANNEL_WIDTH/2-10, 0);
    m_pLabelFader->move(CHANNEL_WIDTH/2 -25, 0);
    
    if (p_eType == OUT) {
        m_pLabelFader->setText(trUtf8("Main output"));        
    }
    else {
        m_pLabelFader->setText(Backend::instance()->getChannel(m_eType, m_sChannelName)->display_name);
        connect(m_pLabelFader, SIGNAL(doubleClicked()), this, SLOT(changeName()));
    }
}
Fader* FWidget::getFader() {
    return (Fader*)m_pVolume;
}
FaderName* FWidget::getLabelFader() {
    return m_pLabelFader;
}
void FWidget::setFixedHeight(int h) {
    QWidget::setFixedHeight(h);
    ((Fader*)m_pVolume)->setFixedHeight(h);
    m_pLabelFader->setFixedHeight(h - 10);
}
void FWidget::changeName() {
    bool ok;
    QString text = QInputDialog::getText(this, trUtf8("%1 \"%2\"").arg(Widget::getDisplayChannelType(m_eType)).arg(m_sChannelName),
            trUtf8("Display name"), QLineEdit::Normal, Backend::instance()->getChannel(m_eType, m_sChannelName)->display_name, &ok);
     if (ok && !text.isEmpty()) {
        Backend::instance()->getChannel(m_eType, m_sChannelName)->display_name = text;
        m_pLabelFader->setText(text);
     }
}

FWidget* Widget::createFader(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_rChannelTo) {
    FWidget *fader = new FWidget(getFaderHeight(), p_eType, p_sChannelName);
    addVolume(fader, p_eType, p_sChannelName, p_eElement, p_rChannelTo);
    fader->setToolTip(getDisplayFunction(p_eType, p_sChannelName, p_eElement, p_rChannelTo));
//    connect(fader, SIGNAL( dbValueChanged(QString, float) ), Backend::instance(), SLOT( setInVolume( QString, float ) ) );
	if (p_eType == IN || p_eType == POST) {
	    fader->getFader()->setMaxValue(20);
	    fader->getFader()->setMaxPeak(20);
	    fader->getFader()->setMinValue(-60);
	    fader->getFader()->setMinPeak(-60);
	}
	else {
	    fader->getFader()->setMaxValue(0);
	    fader->getFader()->setMaxPeak(0);
	    fader->getFader()->setMinValue(-80);
	    fader->getFader()->setMinPeak(-80);
	}
    fader->setDbValue(Backend::instance()->getChannel(p_eType, p_sChannelName)->getFloatAttribute(p_eElement, p_rChannelTo));
    return fader;
}


VWidget::VWidget() {
}
Volume* VWidget::getVolume() {
	return m_pVolume;
}
QWidget* VWidget::getWidget() {
    return m_pVolume;
}
void VWidget::setValue(float p_fValue, bool p_bEmit) {
	m_pVolume->setValue(p_fValue, p_bEmit);
}
float VWidget::getValue() {
	return m_pVolume->getValue();
}
void VWidget::setDbValue(float fValue) {
	m_pVolume->setDbValue(fValue);
}
float VWidget::getDbValue() {
	return m_pVolume->getDbValue();
}
float VWidget::getMinValue() {
	return m_pVolume->getMinValue();
}
float VWidget::getMaxValue() {
	return m_pVolume->getMaxValue();
}
void VWidget::incValue(bool p_bDirection, int p_iStep) {
	m_pVolume->incValue(p_bDirection, p_iStep);
}

RWidget::RWidget(ElementType p_eElement, QString p_rToolTip)
{
    m_pVolume = new Rotary(this, p_eElement == PAN_BAL ? Rotary::TYPE_CENTER : Rotary::TYPE_NORMAL, p_rToolTip, false, true);
    setFixedSize(CHANNEL_WIDTH, 26);
    m_pVolume->move((CHANNEL_WIDTH - 28) / 2, 0);
    if (p_eElement == PAN_BAL) {
        m_pBackground = new QPixmap();
        if (!m_pBackground->load( ":/data/bal_background.png")) {
            qDebug() << "Error loading image: 'bal_background.png'";
            m_pBackground = NULL;
        }
    }
    else {
        m_pBackground = NULL;
    }
}
void RWidget::paintEvent(QPaintEvent *p_pEvent)
{
    QPainter painter(this);

    // background
    if (m_pBackground != NULL) {
        painter.drawPixmap(p_pEvent->rect(), *m_pBackground, p_pEvent->rect());
    }
}
Rotary* RWidget::getRotary() {
	return (Rotary*)m_pVolume;
}
TWidget::TWidget() {
    m_pToggle = ToggleButton::create(this);
    setFixedSize(CHANNEL_WIDTH, 16);
    m_pToggle->move((CHANNEL_WIDTH - 21) / 2, 0);
}
ToggleButton* TWidget::getToggle() {
	return m_pToggle;
}
QWidget* TWidget::getWidget() {
    return m_pToggle;
}
bool TWidget::getValue() {
	return m_pToggle->getValue();
}
void TWidget::setValue(bool p_bValue, bool p_bEmit) {
	m_pToggle->setValue(p_bValue, p_bEmit);
}

VWidget* Widget::createRotary(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_rChannelTo) {
	RWidget* rotary = new RWidget(p_eElement, getDisplayFunction(p_eType, p_sChannelName, p_eElement, p_rChannelTo));
    if (p_eElement == PAN_BAL) {
    	rotary->setValue(Backend::instance()->getChannel(p_eType, p_sChannelName)->getFloatAttribute(p_eElement, p_rChannelTo));
    }
    else {
    	rotary->setDbValue(Backend::instance()->getChannel(p_eType, p_sChannelName)->getFloatAttribute(p_eElement, p_rChannelTo));
    }
    if (!isVisible(p_eElement == TO_AFL ? TO_PFL : p_eElement, p_rChannelTo)) {
		rotary->setVisible(false);
    }
    addVolume(rotary, p_eType, p_sChannelName, p_eElement, p_rChannelTo);
    return rotary;
}
TWidget* Widget::createToggle(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_rChannelTo) {
    TWidget* toggle = new TWidget();
    addToggle(toggle, p_eType, p_sChannelName, p_eElement, p_rChannelTo);
    toggle->setToolTip(getDisplayFunction(p_eType, p_sChannelName, p_eElement, p_rChannelTo));
    toggle->getToggle()->setText(getShortDisplayFunction(p_eElement, p_rChannelTo));
//    connect(main_on, SIGNAL( valueChanged(QString, bool) ), Backend::instance(), SLOT( setInMain( QString, bool ) ) );
    toggle->setValue(Backend::instance()->getChannel(p_eType, p_sChannelName)->getBoolAttribute(p_eElement, p_rChannelTo));
    if (!isVisible(p_eElement, p_rChannelTo)) {
		toggle->setVisible(false);
    }
    return toggle;
}

}
; //LiveMix
