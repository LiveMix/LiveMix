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

namespace LiveMix
{

Widget::Widget(QWidget* p)
        : QWidget( p )
        , m_mShurtCut()
        , m_pSelectedWrapper(NULL)
{
    QVBoxLayout *main_layout = new QVBoxLayout;
    main_layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

#ifdef LADSPA_SUPPORT
    QScrollArea *effectScrollArea = new QScrollArea;
// effectScrollArea->setFrameShape( QFrame::NoFrame );
// effectScrollArea->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    effectScrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
    QWidget* effect = new QWidget;
// effect->setMinimumWidth(50);
// effect->setMaximumWidth(2000);
// effect->setMinimumHeight(200);
// effect->resize(30, 200);
// effect->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    effectScrollArea->setFixedHeight(239+58);
    main_layout->addWidget(effectScrollArea);
    effectScrollArea->setWidget( effect );

    effect_layout = new QHBoxLayout;
    effect->setLayout(effect_layout);
    effect_layout->setSizeConstraint(QLayout::SetMinimumSize);
// effect_layout->setSpacing(0);
// effect_layout->setMargin(0);
    QWidget* effect_start = new QWidget();
    m_pEffectStart = effect_start;
    effect_layout->addWidget(effect_start);
    QVBoxLayout *effect_start_layout = new QVBoxLayout;
    effect_start_layout->setSpacing(0);
    effect_start_layout->setMargin(0);
    effect_start->setMinimumHeight(259);
    effect_start->resize(259, effect_start->height());
    effect_start->setLayout(effect_start_layout);
    effect_start->hide();
    Button* select = Button::create();
    connect(select, SIGNAL( clicked() ), this, SLOT( addFX() ) );
    select->setText(trUtf8("A"));
    select->setToolTip(trUtf8("Add effect"));
    effect_start_layout->addWidget(select);
    addSpacer(effect_start_layout);
    effectName = new FaderName();
    {
        QFont font = effectName->font();
//  font.setPixelSize((int)(font.pixelSize() * 1.5));
        font.setPixelSize(16);
        font.setItalic(false);
        effectName->setFont(font);
    }
    effectName->setText("test");
    effect_start_layout->addWidget(effectName);
// effect_start_layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
// TODO effect
    effect_layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
// effect->updateGeometry();
// effect_layout->update();
#endif

// main_layout->addWidget(effect); // rm
// QScrollArea *mix_mainScrollArea = new QScrollArea( NULL );
// mix_mainScrollArea->setFrameShape( QFrame::NoFrame );
// mix_mainScrollArea->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
// mix_mainScrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
    QWidget* mix_main = new QWidget;
// mix_mainScrollArea->setWidget( mix_main );
// main_layout->addWidget(mix_mainScrollArea);
    main_layout->addWidget(mix_main); // rm
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
    connect(main_widget->fader, SIGNAL( displayValueChanged(Backend::ChannelType, QString, QString) ), this, SLOT( faderValueChange( Backend::ChannelType, QString, QString ) ) );
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
	    addToggle(fx->gui->getActivateButton(), p_eType, p_sChannelName, Backend::MUTE_EFFECT, fx->fx->getPluginLabel());
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

void Widget::faderValueChange(Backend::ChannelType type, QString channel, QString p_fValue)
{
    switch (type) {
    case Backend::IN: {
            showMessage(trUtf8("Input '%1' fader value : %2.").arg(Backend::instance()->getInput(channel)->display_name).arg(p_fValue));
            break;
        }
    case Backend::OUT: {
            showMessage(trUtf8("Output fader value : %1.").arg(p_fValue));
            break;
        }
    case Backend::PRE: {
            showMessage(trUtf8("Pre fader aux '%1' value : %2.").arg(Backend::instance()->getPre(channel)->display_name).arg(p_fValue));
            break;
        }
    case Backend::POST: {
            showMessage(trUtf8("Post fader aux '%1' value : %2.").arg(Backend::instance()->getPost(channel)->display_name).arg(p_fValue));
            break;
        }
    case Backend::SUB: {
            showMessage(trUtf8("Sub-groupe '%1' fader value : %2.").arg(Backend::instance()->getSub(channel)->display_name).arg(p_fValue));
            break;
        }
    }
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
    m_pEffectStart->show();

    foreach (struct effect* fx, m_lVisibleEffect) {
        fx->gui->hide();
    }
    m_lVisibleEffect.clear();

    switch (m_eSelectType) {
    case Backend::IN: {
            showMessage(trUtf8("Input '%1' selected.").arg(Backend::instance()->getInput(m_sSelectChannel)->display_name));
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
            showMessage(trUtf8("Pre fader aux '%1' selected.").arg(Backend::instance()->getPre(m_sSelectChannel)->display_name));
            foreach (effect* elem, *(Backend::instance()->getPreEffects(m_sSelectChannel))) {
                displayFX(elem, m_eSelectType, m_sSelectChannel);
            }
            break;
        }
    case Backend::POST: {
            showMessage(trUtf8("Post fader aux '%1' selected.").arg(Backend::instance()->getPost(m_sSelectChannel)->display_name));
            foreach (effect* elem, *(Backend::instance()->getPostEffects(m_sSelectChannel))) {
                displayFX(elem, m_eSelectType, m_sSelectChannel);
            }
            break;
        }
    case Backend::SUB: {
            showMessage(trUtf8("Sub-groupe '%1' selected.").arg(Backend::instance()->getSub(m_sSelectChannel)->display_name));
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
    for (QListIterator<QString> in_iter(Backend::instance()->inchannels()) ;
            in_iter.hasNext() ;
        ) {
        QString in_name = in_iter.next();
        addinchannel( in_name, false );
    }
    for (QListIterator<QString> iter(Backend::instance()->prechannels()) ;
            iter.hasNext() ;
        ) {
        QString name = iter.next();
        addprechannel( name );
    }
    for (QListIterator<QString> iter(Backend::instance()->postchannels()) ;
            iter.hasNext() ;
        ) {
        QString name = iter.next();
        addpostchannel( name, false );
    }
    for (QListIterator<QString> iter(Backend::instance()->subchannels()) ;
            iter.hasNext() ;
        ) {
        QString name = iter.next();
        addsubchannel( name );
    }
    doSelect(Backend::OUT, MAIN);
}

void Widget::addinchannel( QString name, bool related )
{
    InWidget* elem = new InWidget(name, this);
    connect(elem->fader, SIGNAL( displayValueChanged(Backend::ChannelType, QString, QString) ), this, SLOT( faderValueChange( Backend::ChannelType, QString, QString ) ) );
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
    connect(elem->fader, SIGNAL( displayValueChanged(Backend::ChannelType, QString, QString) ), this, SLOT( faderValueChange( Backend::ChannelType, QString, QString ) ) );
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
    connect(elem->fader, SIGNAL( displayValueChanged(Backend::ChannelType, QString, QString) ), this, SLOT( faderValueChange( Backend::ChannelType, QString, QString ) ) );
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
    connect(elem->fader, SIGNAL( displayValueChanged(Backend::ChannelType, QString, QString) ), this, SLOT( faderValueChange( Backend::ChannelType, QString, QString ) ) );
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

QString Widget::displayElement(Backend::ElementType p_eElement) {
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

void Widget::middleClick(Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, QString p_sReatedChannelName, QMouseEvent* ev)
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
	AssigneToPannel* panel = new AssigneToPannel(p_sChannelName, displayElement(p_eElement), true, rActionOnChannelKeySequence, rSelectChannelKeySequence, rActionOnSelectedChannelKeySequence);;
	bool volume = true;
	switch (p_eElement) {
		case Backend::GAIN:
		case Backend::PAN_BAL:
		case Backend::TO_PRE:
		case Backend::TO_POST:
		case Backend::FADER:
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

    if (panel->exec() == QDialog::Accepted) {
//qDebug()<<111<<panel->getActionOnChannelKeySequence().toString()<<rSelectChannelKeySequence.toString();
//qDebug()<<111<<panel->getSelectChannelKeySequence().toString()<<rActionOnChannelKeySequence.toString();
//qDebug()<<111<<panel->getActionOnSelectedChannelKeySequence().toString()<<rActionOnSelectedChannelKeySequence.toString();
    	if (panel->getActionOnChannelKeySequence() != QKeySequence()) {
    		if (panel->getActionOnChannelKeySequence() != rActionOnChannelKeySequence) {
	    		if ((!m_mKeyToWrapp.contains(panel->getActionOnChannelKeySequence())) || QMessageBox::question (this, trUtf8("Reassigne key")
	    				, trUtf8("Does I reassigne the %1 key ?").arg(panel->getActionOnChannelKeySequence().toString())
	    				, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)) {
qDebug()<<(m_mKeyToWrapp[panel->getActionOnChannelKeySequence()]->name());
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
			    	m_mKeyToWrapp.remove(rActionOnSelectedChannelKeySequence);
		    		m_mKeyToWrapp.insert(panel->getActionOnSelectedChannelKeySequence(), new KeyDoChannelAction(this, p_eElement, p_sReatedChannelName));
	    		}
    		}
    	}
    	else {
   			delete m_mKeyToWrapp[rActionOnSelectedChannelKeySequence];
	    	m_mKeyToWrapp.remove(rActionOnSelectedChannelKeySequence);
    	}
    }
}

void Widget::rightClick(Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, QString p_sReatedChannelName, QMouseEvent* ev)
{
    middleClick(p_eType, p_sChannelName, p_eElement, p_sReatedChannelName, ev);
}

void Widget::action(Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, QString p_sReatedChannelName)
{
showMessage("333");
qDebug()<<(p_eType==Backend::IN);
qDebug()<<p_sChannelName;
qDebug()<<p_sReatedChannelName;
qDebug()<<displayElement(p_eElement);
qDebug()<<(*(*m_mShurtCut[p_eType])[p_sChannelName])[p_eElement]->size();
    if (m_mShurtCut.contains(p_eType) && m_mShurtCut[p_eType]->contains(p_sChannelName)
            && (*m_mShurtCut[p_eType])[p_sChannelName]->contains(p_eElement)
            && (*(*m_mShurtCut[p_eType])[p_sChannelName])[p_eElement]->contains(p_sReatedChannelName)) {
        Wrapp* pWrapp = (*(*(*m_mShurtCut[p_eType])[p_sChannelName])[p_eElement])[p_sReatedChannelName];
        if (!pWrapp->exec()) {
            m_pSelectedWrapper = (WrappVolume*)pWrapp;
            showMessage(QString("Select %1 %2 on channel %3.").arg(displayElement(p_eElement)).arg(p_sReatedChannelName).arg(p_sChannelName));
        } else {
            m_pSelectedWrapper = NULL;
        }
    }
}
void Widget::keyPressEvent ( QKeyEvent * p_pEvent )
{
    if (p_pEvent->key() == Qt::Key_Home || p_pEvent->key() == Qt::Key_Up || p_pEvent->key() == Qt::Key_PageUp) {
        if (m_pSelectedWrapper != NULL) {
            m_pSelectedWrapper->getVolume()->incValue(true);
        }
    } else if (p_pEvent->key() == Qt::Key_End || p_pEvent->key() == Qt::Key_Down || p_pEvent->key() == Qt::Key_PageDown) {
        if (m_pSelectedWrapper != NULL) {
            m_pSelectedWrapper->getVolume()->incValue(false);
        }
    } else if (!p_pEvent->isAutoRepeat()) {
        QKeySequence keys = QKeySequence(p_pEvent->key()+p_pEvent->modifiers());
        if (m_mKeyToWrapp.contains(keys)) {
            m_mKeyToWrapp[keys]->action();
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
void Widget::addVolume(Volume* p_pVolume, Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, QString p_sReatedChannelName)
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
	            new WrappVolume(this, p_pVolume, p_eType, p_sChannelName, p_eElement, p_sReatedChannelName));
    }
}
void Widget::addToggle(ToggleButton* p_pVolume, Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, QString p_sReatedChannelName)
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
    	        new WrappToggle(this, p_pVolume, p_eType, p_sChannelName, p_eElement, p_sReatedChannelName));
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

KeyDoChannelAction::KeyDoChannelAction(Widget* p_pMatrix, Backend::ElementType p_eElement, QString p_sReatedChannelName)
        : KeyDo(p_pMatrix)
        , m_eElement(p_eElement)
        , m_sReatedChannelName(p_sReatedChannelName)
{
}
KeyDoChannelAction::~KeyDoChannelAction()
{}
void KeyDoChannelAction::action()
{
    m_pMatrix->action(m_pMatrix->getSelectedChanelType(), m_pMatrix->getSetectedChannelName(), m_eElement, m_sReatedChannelName);
}

KeyDoDirectAction::KeyDoDirectAction(Widget* p_pMatrix, Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, QString p_sReatedChannelName)
        : KeyDo(p_pMatrix)
        , m_eType(p_eType)
        , m_sChannelName(p_sChannelName)
        , m_eElement(p_eElement)
        , m_sReatedChannelName(p_sReatedChannelName)
{
}
KeyDoDirectAction::~KeyDoDirectAction()
{}
void KeyDoDirectAction::action()
{
    m_pMatrix->action(m_eType, m_sChannelName, m_eElement, m_sReatedChannelName);
}

Wrapp::Wrapp(Widget* p_pMatrix, Action* p_pWidget, Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, QString p_sReatedChannelName)
        : QObject()
        , m_pMatrix(p_pMatrix)
        , m_eType(p_eType)
        , m_sChannelName(p_sChannelName)
        , m_eElement(p_eElement)
        , m_sReatedChannelName(p_sReatedChannelName)
{
    connect(p_pWidget, SIGNAL( middleClick(QMouseEvent*) ), this, SLOT( middleClick(QMouseEvent*) ) );
    connect(p_pWidget, SIGNAL( rightClick(QMouseEvent*) ), this, SLOT( rightClick(QMouseEvent*) ) );
};
bool Wrapp::exec()
{
    return false;
}
void Wrapp::middleClick(QMouseEvent* p_ev)
{
    m_pMatrix->middleClick(m_eType, m_sChannelName, m_eElement, m_sReatedChannelName, p_ev);
};
void Wrapp::rightClick(QMouseEvent* p_ev)
{
    m_pMatrix->rightClick(m_eType, m_sChannelName, m_eElement, m_sReatedChannelName, p_ev);
};

WrappVolume::WrappVolume(Widget* p_pMatrix, Volume* p_pWidget, Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, QString p_sReatedChannelName)
        : Wrapp(p_pMatrix, p_pWidget, p_eType, p_sChannelName, p_eElement, p_sReatedChannelName)
        , m_pWidget(p_pWidget)
{}
Volume* WrappVolume::getVolume()
{
    return m_pWidget;
}

WrappToggle::WrappToggle(Widget* p_pMatrix, ToggleButton* p_pWidget, Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, QString p_sReatedChannelName)
        : Wrapp(p_pMatrix, p_pWidget, p_eType, p_sChannelName, p_eElement, p_sReatedChannelName)
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
    lPre->addWidget(new QLabel(trUtf8("Pre")));
    addLine(layout);
    lPre->parentWidget()->hide();

    wPost = new QWidget;
    lPost = new QVBoxLayout;
    lPost->setSpacing(0);
    lPost->setMargin(0);
    wPost->setLayout(lPost);
    layout->addWidget(wPost);
    lPost->addWidget(new QLabel(trUtf8("Post")));
    addLine(layout);
    lPost->parentWidget()->hide();

    wSub = new QWidget;
    lSub = new QVBoxLayout;
    lSub->setSpacing(0);
    lSub->setMargin(0);
    wSub->setLayout(lSub);
    layout->addWidget(wSub);
    lSub->addWidget(new QLabel(trUtf8("Sub")));
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
    fader->setFixedSize( 23, 232 );
    layout->addWidget(fader);
    connect(fader, SIGNAL( dbValueChanged(QString, float) ), Backend::instance(), SLOT( setInVolume( QString, float ) ) );

    fader->setDbValue(Backend::instance()->getInput(p_sChannel)->volume);
}
InWidget::~InWidget()
{}
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
    fader->setFixedSize( 23, 232 );
    fader->setMaxValue(0);
    fader->setMaxPeak(0);
    fader->setMinValue(-80);
    fader->setMinPeak(-80);
    layout->addWidget(fader);
    connect(fader, SIGNAL( dbValueChanged(QString, float) ), Backend::instance(), SLOT( setPreVolume( QString, float ) ) );
    fader->setDbValue(Backend::instance()->getPre(channel)->volume);
}
PreWidget::~PreWidget()
{}
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
    lSub->addWidget(new QLabel(trUtf8("Sub")));
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
    fader->setFixedSize( 23, 232 );
// fader->setMaxValue(0);
// fader->setMaxPeak(0);
    layout->addWidget(fader);
    connect(fader, SIGNAL( dbValueChanged(QString, float) ), Backend::instance(), SLOT( setPostPostVolume( QString, float ) ) );
    fader->setDbValue(Backend::instance()->getPost(channel)->postvolume);
}
PostWidget::~PostWidget()
{}
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
    fader->setFixedSize( 23, 232 );
    fader->setMaxValue(0);
    fader->setMaxPeak(0);
    fader->setMinValue(-80);
    fader->setMinPeak(-80);
    layout->addWidget(fader);
    connect(fader, SIGNAL( dbValueChanged(QString, float) ), Backend::instance(), SLOT( setSubVolume( QString, float ) ) );
    fader->setDbValue(Backend::instance()->getSub(channel)->volume);
}
SubWidget::~SubWidget()
{}
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
    fader->setFixedSize( 23, 232 );
    fader->setMaxValue(0);
    fader->setMaxPeak(0);
    fader->setMinValue(-80);
    fader->setMinPeak(-80);
    layout->addWidget(fader);
    connect(fader, SIGNAL( dbValueChanged(QString, float) ), Backend::instance(), SLOT( setOutVolume( QString, float ) ) );
    fader->setDbValue(Backend::instance()->getOutput(MAIN)->volume);
}
MainWidget::~MainWidget()
{}
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
