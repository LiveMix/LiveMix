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
#include "globals.h"
#include "AssigneToPannel.h"

#include <QtCore/QMetaProperty>
#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <QtGui/QRubberBand>
#include <QtGui/QLayout>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QCursor>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QLabel>
#include <QtGui/QSpacerItem>
#include <QtGui/QFont>
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

QString Widget::getDisplayElement(Backend::ElementType p_eElement) {
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
}

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
QString Widget::getDisplayChannelType(Backend::ChannelType p_eType) {
	QString displayName;
    switch (p_eType) {
   		case Backend::IN:
            displayName = trUtf8("Input");
            break;
	    case Backend::OUT:
            displayName = trUtf8("Output");
            break;
    	case Backend::PRE:
            displayName = trUtf8("Pre-fader");
            break;
	    case Backend::POST:
            displayName = trUtf8("Post-fader");
            break;
    	case Backend::SUB:
            displayName = trUtf8("Sub-group");
            break;
    }
    return displayName;
}
QString Widget::getShortDisplayChannelType(Backend::ChannelType p_eType) {
	QString displayName;
    switch (p_eType) {
   		case Backend::IN:
            displayName = trUtf8("In");
            break;
	    case Backend::OUT:
            displayName = trUtf8("Out");
            break;
    	case Backend::PRE:
            displayName = trUtf8("Pre");
            break;
	    case Backend::POST:
            displayName = trUtf8("Post");
            break;
    	case Backend::SUB:
            displayName = trUtf8("Sub");
            break;
    }
    return displayName;
}

void Widget::middleClick(Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, QString p_sReatedChannelName, QString p_sDisplayReatedChannelName, QMouseEvent* ev)
{
    UNUSED(p_eType);
    UNUSED(p_sChannelName);
    UNUSED(p_eElement);
    UNUSED(p_sReatedChannelName);
    UNUSED(ev);

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
	
	QString displayElement = getDisplayElement(p_eElement);
	if (p_sReatedChannelName != "") {
		displayElement += " " + p_sDisplayReatedChannelName;
	}
	
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
		QString p_sReatedChannelName, QString p_sDisplayReatedChannelName)
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
            	QString elem = getDisplayElement(p_eElement);
            	elem = elem.left(1).toUpper() + elem.right(elem.size() - 1);
            	if (p_sReatedChannelName.size() > 0) {
            		elem += " ";
            	}

	            showMessage(trUtf8("%1%2 on channel %3 selected.").arg(elem).arg(p_sDisplayReatedChannelName)
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

KeyDo::KeyDo(Widget* p_pMatrix)
        : m_pMatrix(p_pMatrix)
{}
KeyDo::~KeyDo()
{}

KeyDoSelectChannel::KeyDoSelectChannel(Widget* p_pMatrix, Backend::ChannelType p_eType, QString p_sChannelName)
        : KeyDo(p_pMatrix)
        , m_eType(p_eType)
        , m_sChannelName(p_sChannelName)
{
}
KeyDoSelectChannel::~KeyDoSelectChannel()
{}
void KeyDoSelectChannel::action()
{
    m_pMatrix->doSelect(m_eType, m_sChannelName);
}

KeyDoChannelAction::KeyDoChannelAction(Widget* p_pMatrix, Backend::ElementType p_eElement, 
		QString p_sReatedChannelName, QString p_sDisplayReatedChannelName)
        : KeyDo(p_pMatrix)
        , m_eElement(p_eElement)
        , m_sReatedChannelName(p_sReatedChannelName)
        , m_sDisplayReatedChannelName(p_sDisplayReatedChannelName)
{
}
KeyDoChannelAction::~KeyDoChannelAction()
{}
void KeyDoChannelAction::action()
{
    m_pMatrix->action(m_pMatrix->getSelectedChanelType(), m_pMatrix->getSetectedChannelName(), m_eElement, m_sReatedChannelName, m_sDisplayReatedChannelName);
}

KeyDoDirectAction::KeyDoDirectAction(Widget* p_pMatrix, Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, 
		QString p_sReatedChannelName, QString p_sDisplayReatedChannelName)
        : KeyDo(p_pMatrix)
        , m_eType(p_eType)
        , m_sChannelName(p_sChannelName)
        , m_eElement(p_eElement)
        , m_sReatedChannelName(p_sReatedChannelName)
        , m_sDisplayReatedChannelName(p_sDisplayReatedChannelName)
{
}
KeyDoDirectAction::~KeyDoDirectAction()
{}
void KeyDoDirectAction::action()
{
    m_pMatrix->action(m_eType, m_sChannelName, m_eElement, m_sReatedChannelName, m_sDisplayReatedChannelName);
}

Wrapp::Wrapp(Widget* p_pMatrix, Action* p_pWidget, Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, 
		QString p_sReatedChannelName, QString p_sDisplayReatedChannelName)
        : QObject()
        , m_pMatrix(p_pMatrix)
        , m_eType(p_eType)
        , m_sChannelName(p_sChannelName)
        , m_eElement(p_eElement)
        , m_sReatedChannelName(p_sReatedChannelName)
		, m_sDisplayReatedChannelName(p_sDisplayReatedChannelName == "" ? p_sReatedChannelName : p_sDisplayReatedChannelName)
{
    connect(p_pWidget, SIGNAL( leftClick(QMouseEvent*) ), this, SLOT( leftClick(QMouseEvent*) ) );
    connect(p_pWidget, SIGNAL( middleClick(QMouseEvent*) ), this, SLOT( middleClick(QMouseEvent*) ) );
    connect(p_pWidget, SIGNAL( rightClick(QMouseEvent*) ), this, SLOT( rightClick(QMouseEvent*) ) );
};
bool Wrapp::exec()
{
    return false;
}
void Wrapp::leftClick(QMouseEvent* p_ev)
{
    m_pMatrix->leftClick(m_eType, m_sChannelName, m_eElement, m_sReatedChannelName, m_sDisplayReatedChannelName, p_ev);
};
void Wrapp::middleClick(QMouseEvent* p_ev)
{
    m_pMatrix->middleClick(m_eType, m_sChannelName, m_eElement, m_sReatedChannelName, m_sDisplayReatedChannelName, p_ev);
};
void Wrapp::rightClick(QMouseEvent* p_ev)
{
    m_pMatrix->rightClick(m_eType, m_sChannelName, m_eElement, m_sReatedChannelName, m_sDisplayReatedChannelName, p_ev);
};

WrappVolume::WrappVolume(Widget* p_pMatrix, Volume* p_pWidget, Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, QString p_sReatedChannelName, QString p_sDisplayReatedChannelName)
        : Wrapp(p_pMatrix, p_pWidget, p_eType, p_sChannelName, p_eElement, p_sReatedChannelName, p_sDisplayReatedChannelName)
        , m_pWidget(p_pWidget)
{
    connect(p_pWidget, SIGNAL(displayValueChanged(QString)), this, SLOT(displayValueChanged(QString)));
}
void WrappVolume::displayValueChanged(QString p_sValue) {
	QString elem = m_pMatrix->getDisplayElement(m_eElement);
	if (m_sReatedChannelName.size() > 0) {
		elem += " ";
	}
	
    switch (m_eType) {
    	case Backend::OUT:
            if (m_sChannelName == MAIN && m_sReatedChannelName == PLF) {
	            m_pMatrix->showMessage(trUtf8("Phono value: %1.").arg(p_sValue));
            }
            else if (m_sChannelName == MAIN && m_sReatedChannelName == MONO) {
	            m_pMatrix->showMessage(trUtf8("Mono value: %1.").arg(p_sValue));
            }
            else if (m_sChannelName == MAIN && m_sReatedChannelName == MAIN) {
	            m_pMatrix->showMessage(trUtf8("Main fader value: %1.").arg(p_sValue));
            }
            else {
	            m_pMatrix->showMessage(trUtf8("Output %1 value: %2.").arg(elem).arg(p_sValue));
            }
            break;
	    case Backend::IN:
	    case Backend::PRE:
    	case Backend::POST:
	    case Backend::SUB:
            m_pMatrix->showMessage(trUtf8("%1 \"%2\" %3%4 value: %5.").arg(m_pMatrix->getDisplayChannelType(m_eType))
            		.arg(m_pMatrix->getDisplayNameOfChannel(m_eType, m_sChannelName)).arg(elem)
            		.arg(m_sDisplayReatedChannelName).arg(p_sValue));
            break;
    }
}

Volume* WrappVolume::getVolume()
{
    return m_pWidget;
}

WrappToggle::WrappToggle(Widget* p_pMatrix, ToggleButton* p_pWidget, Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, QString p_sReatedChannelName, QString p_sDisplayReatedChannelName)
        : Wrapp(p_pMatrix, p_pWidget, p_eType, p_sChannelName, p_eElement, p_sReatedChannelName, p_sDisplayReatedChannelName)
        , m_pWidget(p_pWidget)
{}
bool WrappToggle::exec()
{
    m_pWidget->mousePressEvent(NULL);
    return true;
}


InWidget::InWidget(QString p_sChannel, Widget* p_pMatrix)
        : QWidget()
        , m_Channel(p_sChannel)
        , m_pMatrix(p_pMatrix)
{
    QVBoxLayout* layout = new QVBoxLayout();
    setLayout(layout);
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    setFixedWidth(CHANNEL_WIDTH);
    layout->setSpacing(0);
    layout->setMargin(0);

    layout->addWidget(new QLabel(p_sChannel));

    Rotary *gain = new Rotary(0, Rotary::TYPE_NORMAL, trUtf8("Gain"), false, true, p_sChannel);
    m_pMatrix->addVolume(gain, Backend::IN, p_sChannel, Backend::GAIN);
	gain->setVisible(m_pMatrix->isGainVisible());
    layout->addWidget(gain);
    connect(gain, SIGNAL( dbValueChanged(QString, float) ), Backend::instance(), SLOT( setInGain( QString, float ) ) );
    gain->setDbValue(Backend::instance()->getInput(p_sChannel)->gain);

    ToggleButton* mute = ToggleButton::createSolo(0, p_sChannel);
    m_pMatrix->addToggle(mute, Backend::IN, p_sChannel, Backend::MUTE);
    mute->setToolTip(trUtf8("mute"));
    mute->setText(trUtf8("M"));
    layout->addWidget(mute);
    connect(mute, SIGNAL( valueChanged(QString, bool) ), Backend::instance(), SLOT( setInMute( QString, bool ) ) );
    mute->setValue(Backend::instance()->getInput(p_sChannel)->mute);

    ToggleButton* plf = ToggleButton::createSolo(0, p_sChannel);
    m_pMatrix->addToggle(plf, Backend::IN, p_sChannel, Backend::TO_PLF);
    plf->setToolTip(trUtf8("plf"));
    plf->setText(trUtf8("plf"));
    layout->addWidget(plf);
    connect(plf, SIGNAL( valueChanged(QString, bool) ), Backend::instance(), SLOT( setInPlf( QString, bool ) ) );
    plf->setValue(Backend::instance()->getInput(p_sChannel)->plf);

    addLine(layout);

    wPre = new QWidget;
    lPre = new QVBoxLayout;
    lPre->setSpacing(0);
    lPre->setMargin(0);
    wPre->setLayout(lPre);
    layout->addWidget(wPre);
    lPre->addWidget(new QLabel(m_pMatrix->getShortDisplayChannelType(Backend::PRE)));
    addLine(layout);
    lPre->parentWidget()->hide();

    wPost = new QWidget;
    lPost = new QVBoxLayout;
    lPost->setSpacing(0);
    lPost->setMargin(0);
    wPost->setLayout(lPost);
    layout->addWidget(wPost);
    lPost->addWidget(new QLabel(m_pMatrix->getShortDisplayChannelType(Backend::POST)));
    addLine(layout);
    lPost->parentWidget()->hide();

    wSub = new QWidget;
    lSub = new QVBoxLayout;
    lSub->setSpacing(0);
    lSub->setMargin(0);
    wSub->setLayout(lSub);
    layout->addWidget(wSub);
    lSub->addWidget(new QLabel(m_pMatrix->getShortDisplayChannelType(Backend::SUB)));
    addLine(layout);
    lSub->parentWidget()->hide();

    Rotary *bal = new Rotary(0, Rotary::TYPE_CENTER, Backend::instance()->getInput(p_sChannel)->stereo ? trUtf8("Bal") :  trUtf8("Pan"), false, true, p_sChannel);
    m_pMatrix->addVolume(bal, Backend::IN, p_sChannel, Backend::PAN_BAL);
    layout->addWidget(bal);
    connect(bal, SIGNAL( valueChanged(QString, float) ), Backend::instance(), SLOT( setInBal( QString, float ) ) );
    bal->setValue(Backend::instance()->getInput(p_sChannel)->bal);

    ToggleButton* main_on = ToggleButton::createMute(0, p_sChannel);
    m_pMatrix->addToggle(main_on, Backend::IN, p_sChannel, Backend::TO_MAIN);
    main_on->setToolTip(trUtf8("Main"));
    main_on->setText(trUtf8("LR"));
    layout->addWidget(main_on);
    connect(main_on, SIGNAL( valueChanged(QString, bool) ), Backend::instance(), SLOT( setInMain( QString, bool ) ) );
    main_on->setValue(Backend::instance()->getInput(p_sChannel)->main);

    fader = new Fader(NULL, false, false, p_sChannel, Backend::IN);
    m_pMatrix->addVolume(fader, Backend::IN, p_sChannel, Backend::FADER);
    fader->setFixedSize(23, m_pMatrix->getFaderHeight());
    layout->addWidget(fader);
    connect(fader, SIGNAL( dbValueChanged(QString, float) ), Backend::instance(), SLOT( setInVolume( QString, float ) ) );

    fader->setDbValue(Backend::instance()->getInput(p_sChannel)->volume);
}
InWidget::~InWidget()
{
	m_pMatrix->removeShurtCut(Backend::IN, m_Channel);
}
void InWidget::mouseReleaseEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton) {
        emit clicked(Backend::IN, m_Channel);
    }
}
void InWidget::addPre(QString channelIn, QString channelPre)
{
    Rotary *elem = new Rotary(0, Rotary::TYPE_NORMAL, channelPre, false, true, channelIn, channelPre);
    m_pMatrix->addVolume(elem, Backend::IN, channelIn, Backend::TO_PRE, channelPre);
    lPre->addWidget(elem);
    pre[channelPre] = elem;
    connect(elem, SIGNAL( dbValueChanged(QString, QString, float) ), Backend::instance(), SLOT( setInPreVolume( QString, QString, float ) ) );
    elem->setDbValue(Backend::instance()->getInput(channelIn)->pre[channelPre]);
    if (Backend::instance()->prechannels().size() > 0) {
        lPre->parentWidget()->show();
    }
}
void InWidget::addPost(QString channelIn, QString channelPost)
{
    Rotary *elem = new Rotary(0, Rotary::TYPE_NORMAL, channelPost, false, true, channelIn, channelPost);
    m_pMatrix->addVolume(elem, Backend::IN, channelIn, Backend::TO_POST, channelPost);
    lPost->addWidget(elem);
    post[channelPost] = elem;
    connect(elem, SIGNAL( dbValueChanged(QString, QString, float) ), Backend::instance(), SLOT( setInPostVolume( QString, QString, float ) ) );
    elem->setDbValue(Backend::instance()->getInput(channelIn)->post[channelPost]);
    if (Backend::instance()->postchannels().size() > 0) {
        lPost->parentWidget()->show();
    }
}
void InWidget::addSub(QString channelIn, QString channelSub)
{
    ToggleButton* elem = ToggleButton::create(NULL, channelIn, channelSub);
    m_pMatrix->addToggle(elem, Backend::IN, channelIn, Backend::TO_SUB, channelSub);
    elem->setToolTip(channelSub);
// elem->setText(trUtf8("sub"));
    lSub->addWidget(elem);
    sub[channelSub] = elem;
    connect(elem, SIGNAL( valueChanged(QString, QString, bool) ), Backend::instance(), SLOT( setInSub( QString, QString, bool ) ) );
    elem->setValue(Backend::instance()->getInput(channelIn)->sub[channelSub]);
    if (Backend::instance()->subchannels().size() > 0) {
        lSub->parentWidget()->show();
    }
}
void InWidget::removePre(QString channelIn, QString channelPre)
{
    UNUSED(channelIn);
    Rotary *elem = pre[channelPre];
    lPre->removeWidget(elem);
    pre.remove(channelPre);
    if (Backend::instance()->prechannels().size() == 0) {
        lPre->parentWidget()->hide();
    }
    delete elem;
}
void InWidget::removePost(QString channelIn, QString channelPost)
{
    UNUSED(channelIn);
    Rotary *elem = post[channelPost];
    lPost->removeWidget(elem);
    post.remove(channelPost);
    if (Backend::instance()->postchannels().size() == 0) {
        lPost->parentWidget()->hide();
    }
    delete elem;
}
void InWidget::removeSub(QString channelIn, QString channelSub)
{
    UNUSED(channelIn);
    ToggleButton *elem = sub[channelSub];
    lSub->removeWidget(elem);
    sub.remove(channelSub);
    if (Backend::instance()->subchannels().size() == 0) {
        lSub->parentWidget()->hide();
    }
    delete elem;
}

PreWidget::PreWidget(QString channel, Widget* p_pMatrix)
        : m_Channel(channel)
        , m_pMatrix(p_pMatrix)
{
    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);
    setFixedWidth(CHANNEL_WIDTH);
    layout->setSpacing(0);
    layout->setMargin(0);

    layout->addWidget(new QLabel(channel));

    ToggleButton* mute = ToggleButton::createSolo(0, channel);
    m_pMatrix->addToggle(mute, Backend::PRE, channel, Backend::MUTE);
    mute->setToolTip(trUtf8("mute"));
    mute->setText(trUtf8("M"));
    layout->addWidget(mute);
    connect(mute, SIGNAL( valueChanged(QString, bool) ), Backend::instance(), SLOT( setPreMute( QString, bool ) ) );
    mute->setValue(Backend::instance()->getPre(channel)->mute);

    ToggleButton* alf = ToggleButton::createSolo(0, channel);
    m_pMatrix->addToggle(alf, Backend::PRE, channel, Backend::TO_ALF);
    alf->setToolTip("alf");
    alf->setText(trUtf8("alf"));
    layout->addWidget(alf);
    connect(alf, SIGNAL( valueChanged(QString, bool) ), Backend::instance(), SLOT( setPreAlf( QString, bool ) ) );
    alf->setValue(Backend::instance()->getPre(channel)->alf);

    addSpacer(layout);

    if (Backend::instance()->getPre(channel)->stereo) {
        Rotary *bal = new Rotary(0, Rotary::TYPE_CENTER, trUtf8("Bal"), false, true, channel);
	    m_pMatrix->addVolume(bal, Backend::PRE, channel, Backend::PAN_BAL);
        layout->addWidget(bal);
        connect(bal, SIGNAL( valueChanged(QString, float) ), Backend::instance(), SLOT( setPreBal( QString, float ) ) );
        bal->setValue(Backend::instance()->getPre(channel)->bal);
    }

    fader = new Fader(0, false, false, channel, Backend::PRE);
    m_pMatrix->addVolume(fader, Backend::PRE, channel, Backend::FADER);
    fader->setFixedSize(23, m_pMatrix->getFaderHeight());
    fader->setMaxValue(0);
    fader->setMaxPeak(0);
    fader->setMinValue(-80);
    fader->setMinPeak(-80);
    layout->addWidget(fader);
    connect(fader, SIGNAL( dbValueChanged(QString, float) ), Backend::instance(), SLOT( setPreVolume( QString, float ) ) );
    fader->setDbValue(Backend::instance()->getPre(channel)->volume);
}
PreWidget::~PreWidget()
{
	m_pMatrix->removeShurtCut(Backend::PRE, m_Channel);
}
void PreWidget::mouseReleaseEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton) {
        emit clicked(Backend::PRE, m_Channel);
    }
}
PostWidget::PostWidget(QString channel, Widget* p_pMatrix)
        : m_Channel(channel)
        , m_pMatrix(p_pMatrix)
{
    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);
    setFixedWidth(CHANNEL_WIDTH);
    layout->setSpacing(0);
    layout->setMargin(0);

    layout->addWidget(new QLabel(channel));

    Rotary *prevol = new Rotary(0, Rotary::TYPE_NORMAL, trUtf8("Volume"), false, true, channel);
    m_pMatrix->addVolume(prevol, Backend::POST, channel, Backend::PRE_VOL);
    layout->addWidget(prevol);
    connect(prevol, SIGNAL( dbValueChanged(QString, float) ), Backend::instance(), SLOT( setPostPreVolume( QString, float ) ) );
    prevol->setDbValue(Backend::instance()->getPost(channel)->prevolume);

    ToggleButton* mute = ToggleButton::createSolo(0, channel);
    m_pMatrix->addToggle(mute, Backend::POST, channel, Backend::MUTE);
    mute->setToolTip(trUtf8("mute"));
    mute->setText(trUtf8("M"));
    layout->addWidget(mute);
    connect(mute, SIGNAL( valueChanged(QString, bool) ), Backend::instance(), SLOT( setPostMute( QString, bool ) ) );
    mute->setValue(Backend::instance()->getPost(channel)->mute);

    ToggleButton* alf = ToggleButton::createSolo(0, channel);
    m_pMatrix->addToggle(alf, Backend::POST, channel, Backend::TO_ALF);
    alf->setToolTip(trUtf8("alf"));
    alf->setText(trUtf8("alf"));
    layout->addWidget(alf);
    connect(alf, SIGNAL( valueChanged(QString, bool) ), Backend::instance(), SLOT( setPostAlf( QString, bool ) ) );
    alf->setValue(Backend::instance()->getPost(channel)->alf);
    addLine(layout, true);

    addSpacer(layout);

    layout->addWidget(new QLabel(trUtf8("Return")));
    ToggleButton* plf = ToggleButton::createSolo(0, channel);
    m_pMatrix->addToggle(plf, Backend::POST, channel, Backend::TO_PLF);
    plf->setToolTip(trUtf8("plf"));
    plf->setText(trUtf8("plf"));
    layout->addWidget(plf);
    connect(plf, SIGNAL( valueChanged(QString, bool) ), Backend::instance(), SLOT( setPostPlf( QString, bool ) ) );
    plf->setValue(Backend::instance()->getPost(channel)->plf);

    wSub = new QWidget;
    lSub = new QVBoxLayout;
    lSub->setSpacing(0);
    lSub->setMargin(0);
    wSub->setLayout(lSub);
    layout->addWidget(wSub);
    lSub->addWidget(new QLabel(m_pMatrix->getShortDisplayChannelType(Backend::SUB)));
    addLine(layout);
    lSub->parentWidget()->hide();

    Rotary *bal = new Rotary(0, Rotary::TYPE_CENTER, Backend::instance()->getPost(channel)->stereo ? trUtf8("Bal") : trUtf8("Pan"), false, true, channel);
    m_pMatrix->addVolume(bal, Backend::POST, channel, Backend::PAN_BAL);
    layout->addWidget(bal);
    connect(bal, SIGNAL( valueChanged(QString, float) ), Backend::instance(), SLOT( setPostBal( QString, float ) ) );
    bal->setValue(Backend::instance()->getPost(channel)->bal);

    ToggleButton* main_on = ToggleButton::createMute(0, channel);
    m_pMatrix->addToggle(main_on, Backend::POST, channel, Backend::TO_MAIN);
    main_on->setText(trUtf8("LR"));
    main_on->setToolTip(trUtf8("Main"));
    layout->addWidget(main_on);
    connect(main_on, SIGNAL( valueChanged(QString, bool) ), Backend::instance(), SLOT( setPostMain( QString, bool ) ) );
    main_on->setValue(Backend::instance()->getPost(channel)->main);

    fader = new Fader(0, false, false, channel, Backend::POST);
    m_pMatrix->addVolume(fader, Backend::POST, channel, Backend::FADER);
    fader->setFixedSize(23, m_pMatrix->getFaderHeight());
// fader->setMaxValue(0);
// fader->setMaxPeak(0);
    layout->addWidget(fader);
    connect(fader, SIGNAL( dbValueChanged(QString, float) ), Backend::instance(), SLOT( setPostPostVolume( QString, float ) ) );
    fader->setDbValue(Backend::instance()->getPost(channel)->postvolume);
}
PostWidget::~PostWidget()
{
	m_pMatrix->removeShurtCut(Backend::POST, m_Channel);
}
void PostWidget::mouseReleaseEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton) {
        emit clicked(Backend::POST, m_Channel);
    }
}
void PostWidget::addSub(QString channelPost, QString channelSub)
{
    ToggleButton* elem = ToggleButton::create(0, channelPost, channelSub);
    m_pMatrix->addToggle(elem, Backend::POST, channelPost, Backend::TO_SUB, channelSub);
    elem->setToolTip(channelSub);
// elem->setText(channelSub);
    lSub->addWidget(elem);
    sub[channelSub] = elem;
    connect(elem, SIGNAL( valueChanged(QString, QString, bool) ), Backend::instance(), SLOT( setPostSub( QString, QString, bool ) ) );
    elem->setValue(Backend::instance()->getPost(channelPost)->sub[channelSub]);
    if (Backend::instance()->subchannels().size() > 0) {
        lSub->parentWidget()->show();
    }
}
void PostWidget::removeSub(QString channelPost, QString channelSub)
{
    UNUSED(channelPost);
    ToggleButton *elem = sub[channelSub];
    lSub->removeWidget(elem);
    sub.remove(channelSub);
    if (Backend::instance()->subchannels().size() == 0) {
        lSub->parentWidget()->hide();
    }
    delete elem;
}

SubWidget::SubWidget(QString channel, Widget* p_pMatrix)
        : m_Channel(channel)
        , m_pMatrix(p_pMatrix)
{
    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);
    setFixedWidth(CHANNEL_WIDTH);
    layout->setSpacing(0);
    layout->setMargin(0);

    layout->addWidget(new QLabel(channel));

    ToggleButton* mute = ToggleButton::createSolo(0, channel);
    m_pMatrix->addToggle(mute, Backend::SUB, channel, Backend::MUTE);
    mute->setToolTip(trUtf8("mute"));
    mute->setText(trUtf8("M"));
    layout->addWidget(mute);
    connect(mute, SIGNAL( valueChanged(QString, bool) ), Backend::instance(), SLOT( setSubMute( QString, bool ) ) );
    mute->setValue(Backend::instance()->getSub(channel)->mute);

    ToggleButton* alf = ToggleButton::createSolo(0, channel);
    m_pMatrix->addToggle(alf, Backend::SUB, channel, Backend::TO_ALF);
    alf->setToolTip(trUtf8("alf"));
    alf->setText(trUtf8("alf"));
    layout->addWidget(alf);
    connect(alf, SIGNAL( valueChanged(QString, bool) ), Backend::instance(), SLOT( setSubAlf( QString, bool ) ) );
    alf->setValue(Backend::instance()->getSub(channel)->alf);

    addSpacer(layout);

    Rotary *bal = new Rotary(0, Rotary::TYPE_CENTER, Backend::instance()->getSub(channel)->stereo ? trUtf8("Bal") : trUtf8("Pan"), false, true, channel);
    m_pMatrix->addVolume(bal, Backend::SUB, channel, Backend::PAN_BAL);
    layout->addWidget(bal);
    connect(bal, SIGNAL( valueChanged(QString, float) ), Backend::instance(), SLOT( setSubBal( QString, float ) ) );
    bal->setValue(Backend::instance()->getSub(channel)->bal);

    ToggleButton* main_on = ToggleButton::createMute(0, channel);
    m_pMatrix->addToggle(main_on, Backend::SUB, channel, Backend::TO_MAIN);
    main_on->setToolTip(trUtf8("Main"));
    main_on->setText(trUtf8("LR"));
    layout->addWidget(main_on);
    connect(main_on, SIGNAL( valueChanged(QString, bool) ), Backend::instance(), SLOT( setSubMain( QString, bool ) ) );
    main_on->setValue(Backend::instance()->getSub(channel)->main);

    fader = new Fader(0, false, false, channel, Backend::SUB);
    m_pMatrix->addVolume(fader, Backend::SUB, channel, Backend::FADER);
    fader->setFixedSize(23, m_pMatrix->getFaderHeight());
    fader->setMaxValue(0);
    fader->setMaxPeak(0);
    fader->setMinValue(-80);
    fader->setMinPeak(-80);
    layout->addWidget(fader);
    connect(fader, SIGNAL( dbValueChanged(QString, float) ), Backend::instance(), SLOT( setSubVolume( QString, float ) ) );
    fader->setDbValue(Backend::instance()->getSub(channel)->volume);
}
SubWidget::~SubWidget()
{
	m_pMatrix->removeShurtCut(Backend::SUB, m_Channel);
}
void SubWidget::mouseReleaseEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton) {
        emit clicked(Backend::SUB, m_Channel);
    }
}

MainWidget::MainWidget(Widget* p_pMatrix) : QWidget()
        , m_pMatrix(p_pMatrix)
{
    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);
    setFixedWidth(CHANNEL_WIDTH);
    layout->setSpacing(0);
    layout->setMargin(0);

    layout->addWidget(new QLabel(trUtf8("Phone")));

    phone = new Rotary(0, Rotary::TYPE_NORMAL, trUtf8("Phone volume"), false, true, PLF);
    m_pMatrix->addVolume(phone, Backend::OUT, MAIN, Backend::FADER, PLF);
    layout->addWidget(phone);
    connect(phone, SIGNAL( dbValueChanged(QString, float) ), Backend::instance(), SLOT( setOutVolume( QString, float ) ) );
    phone->setDbValue(Backend::instance()->getOutput(PLF)->volume);

    addSpacer(layout);
    layout->addWidget(new QLabel(trUtf8("Main")));

    mute = ToggleButton::createSolo(0, MAIN);
    m_pMatrix->addToggle(mute, Backend::OUT, MAIN, Backend::MUTE);
    mute->setToolTip(trUtf8("mute"));
    mute->setText(trUtf8("M"));
    layout->addWidget(mute);
    connect(mute, SIGNAL( valueChanged(QString, bool) ), Backend::instance(), SLOT( setOutMute( QString, bool ) ) );
    mute->setValue(Backend::instance()->getOutput(MAIN)->mute);
    addLine(layout);

    mono = new Rotary(0, Rotary::TYPE_NORMAL, trUtf8("Mono volume"), false, true, MONO);
    m_pMatrix->addVolume(mono, Backend::OUT, MAIN, Backend::FADER, MONO);
    layout->addWidget(mono);
    connect(mono, SIGNAL( dbValueChanged(QString, float) ), Backend::instance(), SLOT( setOutVolume( QString, float ) ) );
    mono->setDbValue(Backend::instance()->getOutput(MONO)->volume);
    addLine(layout);

    bal = new Rotary(0, Rotary::TYPE_CENTER, trUtf8("Bal"), false, true, MAIN);
    m_pMatrix->addVolume(bal, Backend::OUT, MAIN, Backend::PAN_BAL);
    layout->addWidget(bal);
    connect(bal, SIGNAL( valueChanged(QString, float) ), Backend::instance(), SLOT( setOutBal( QString, float ) ) );
    bal->setValue(Backend::instance()->getOutput(MAIN)->bal);

    alf = ToggleButton::createSolo(0, MAIN);
    m_pMatrix->addToggle(alf, Backend::OUT, MAIN, Backend::TO_ALF);
    alf->setToolTip(trUtf8("alf"));
    alf->setText(trUtf8("alf"));
    layout->addWidget(alf);
    connect(alf, SIGNAL( valueChanged(QString, bool) ), Backend::instance(), SLOT( setOutAlf( QString, bool ) ) );
    alf->setValue(Backend::instance()->getOutput(MAIN)->alf);

    fader = new Fader(NULL, false, false, MAIN, Backend::OUT);
    m_pMatrix->addVolume(fader, Backend::OUT, MAIN, Backend::FADER, MAIN);
    fader->setFixedSize(23, m_pMatrix->getFaderHeight());
    fader->setMaxValue(0);
    fader->setMaxPeak(0);
    fader->setMinValue(-80);
    fader->setMinPeak(-80);
    layout->addWidget(fader);
    connect(fader, SIGNAL( dbValueChanged(QString, float) ), Backend::instance(), SLOT( setOutVolume( QString, float ) ) );
    fader->setDbValue(Backend::instance()->getOutput(MAIN)->volume);
}
MainWidget::~MainWidget()
{
	m_pMatrix->removeShurtCut(Backend::OUT, MAIN);
}
void MainWidget::update()
{
    phone->setDbValue(Backend::instance()->getOutput(PLF)->volume);
    mute->setValue(Backend::instance()->getOutput(MAIN)->mute);
    mono->setDbValue(Backend::instance()->getOutput(MONO)->volume);
    bal->setValue(Backend::instance()->getOutput(MAIN)->bal);
    alf->setValue(Backend::instance()->getOutput(MAIN)->alf);
    fader->setDbValue(Backend::instance()->getOutput(MAIN)->volume);
}
void MainWidget::mouseReleaseEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton) {
        emit clicked(Backend::OUT, MAIN);
    }
}

void addLine(QVBoxLayout* layout, bool bold)
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

void addLine(QHBoxLayout* layout, bool bold)
{
    QWidget* line = new QWidget;
    QPalette defaultPalette;
    defaultPalette.setColor( QPalette::Background, QColor( 128, 134, 152 ) );
    line->setPalette( defaultPalette );
    line->setFixedSize(bold ? 3 : 2, bold ? 3 : 2);
    line->setMinimumSize(bold ? 3 : 2, bold ? 3 : 2);
    layout->addWidget(line);
}

void addSpacer(QVBoxLayout* layout)
{
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

}
; //LiveMix
