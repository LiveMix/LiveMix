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
#include <QLabel>
#include <QSpacerItem>
#include <QFont>
#include <QMessageBox>
#include <QInputDialog>
#include <QAction>
#include <QPainter>

#include <typeinfo> 


namespace LiveMix
{

class ScrollArea : public QScrollArea
{
    void keyPressEvent(QKeyEvent * e) {
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
        : QWidget(p)
	, m_in()
    	, m_pre()
    	, m_post()
    	, m_sub()
        , m_mShurtCut()
        , m_pSelectedWrapper(NULL)
        , m_bLearn(false)
        , m_iFaderHeight(300)
        , m_iEffectFaderHeight(200)
{
    QVBoxLayout *main_layout = new QVBoxLayout;
    main_layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
    main_layout->setSizeConstraint(QLayout::SetMinimumSize);

#ifdef LADSPA_SUPPORT
    QWidget* main_effect = new QWidget;
    QHBoxLayout* main_effect_layout = new QHBoxLayout;
    main_effect->setLayout(main_effect_layout);
    main_layout->addWidget(main_effect);

    QWidget* effect_start = new QWidget();
    m_pEffectStart = effect_start;
    main_effect_layout->addWidget(effect_start);

    m_pEffectScrollArea = new ScrollArea;
    m_pEffectScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pEffectScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    QWidget* effect = new QWidget;
    main_effect_layout->addWidget(m_pEffectScrollArea);
    m_pEffectScrollArea->setWidget(effect);

    effect_layout = new QHBoxLayout;
    effect->setLayout(effect_layout);
    effect_layout->setSizeConstraint(QLayout::SetMinimumSize);
    
    QVBoxLayout *effect_start_layout = new QVBoxLayout;
    effect_start_layout->setSpacing(0);
    effect_start_layout->setMargin(0);
    effect_start->setLayout(effect_start_layout);
    Button* select = Button::create();
    connect(select, SIGNAL(clicked()), this, SLOT(addFX()));
    select->setText(trUtf8("A"));
    select->setToolTip(trUtf8("Add effect"));
    effect_start_layout->addWidget(select);
    addSpacer(effect_start_layout);
    effectName = new LFWidget(this);
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
    connect(main_widget, SIGNAL(clicked(ChannelType, QString)), this, SLOT(select(ChannelType, QString)));

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

    connect(Backend::instance(), SIGNAL(processed()), this, SLOT(update()));

    m_pStatusTimer = new QTimer(this);
    connect(m_pStatusTimer, SIGNAL(timeout()), this, SLOT(onStatusTimerEvent()));

    startTimer(50);
}

Widget::~Widget()
{}

void Widget::timerEvent(QTimerEvent*)
{
    if (!m_bLearn) {
        while (Backend::instance()->hasMidiEvent()) {
            snd_seq_event_t *ev = Backend::instance()->readMidiEvent();
            if (ev->type == SND_SEQ_EVENT_CONTROLLER) {
                if (m_mMidiToWrapp.contains(ev->data.control.channel)) {
                    QMap<unsigned int, KeyDoDirectAction*>* map = m_mMidiToWrapp[ev->data.control.channel];
                    if (map->contains(ev->data.control.param)) {
                        KeyDoDirectAction *act = (*map)[ev->data.control.param];
                        Wrapp *w = (*(*(*m_mShurtCut[act->m_eType])[act->m_sChannelName])[act->m_eElement])[act->m_sReatedChannelName];
                        if (w != NULL) {
                            switch (act->m_eElement) {
                            case MUTE:
                            case MUTE_EFFECT:
                            case TO_SUB:
                            case TO_MAIN:
                            case TO_PFL:
                            case TO_AFL:
                                ((WrappToggle*)w)->getToggle()->setValue(ev->data.control.value > 63, true, MIDI);
                                break;
                            default:
                                Volume *vol = ((WrappVolume*)w)->getVolume();
                                vol->setValue(ev->data.control.value * (vol->getMaxValue() - vol->getMinValue()) / 127
                                              + vol->getMinValue(), true, MIDI);
                            }
                        }
                    }
                }
            }
        }
    }
}

void Widget::displayFX(effect *fx, ChannelType p_eType, QString p_sChannelName)
{
    if (fx->gui == NULL) {
        fx->gui = new LadspaFXProperties(NULL, fx, m_iEffectFaderHeight);
        addToggle(fx->gui->getActivateButton(), p_eType, p_sChannelName, MUTE_EFFECT, fx->fx->getPluginLabel());
        effect_layout->addWidget(fx->gui);
        connect(fx->gui, SIGNAL(removeClicked(LadspaFXProperties*, effect*)), this, SLOT(askRemoveFX(LadspaFXProperties*, effect*)));
        connect(fx->gui, SIGNAL(leftClicked(LadspaFXProperties*, effect*)), this, SLOT(askLeftFX(LadspaFXProperties*, effect*)));
        connect(fx->gui, SIGNAL(rightClicked(LadspaFXProperties*, effect*)), this, SLOT(askRightFX(LadspaFXProperties*, effect*)));

        channel* c = Backend::instance()->getChannel(p_eType, p_sChannelName);
        if (!c->effectsMap.contains(fx->fx->getPluginLabel())) {
            c->effectsMap[fx->fx->getPluginLabel()] = fx;
            fx->displayname = fx->fx->getPluginName();
        } else {
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

void Widget::askLeftFX(LadspaFXProperties*, effect* p_pFX)
{
    if (Backend::instance()->moveEffect(m_eSelectType, m_sSelectChannel, p_pFX, true)) {
        doSelect(m_eSelectType, m_sSelectChannel, true);
    }
}

void Widget::askRightFX(LadspaFXProperties*, effect* p_pFX)
{
    if (Backend::instance()->moveEffect(m_eSelectType, m_sSelectChannel, p_pFX, false)) {
        doSelect(m_eSelectType, m_sSelectChannel, true);
    }
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
    removeFX(widget, fx, m_eSelectType, m_sSelectChannel);
}
void Widget::removeFX(LadspaFXProperties* widget, effect* fx, ChannelType p_eType, QString p_rChannel)
{
    m_lVisibleEffect.removeAll(fx);
    disconnect(widget, 0, 0, 0);
    widget->hide();
    effect_layout->removeWidget(widget);

    // workaround
    QSize size(50, effect_layout->parentWidget()->height());
    effect_layout->parentWidget()->setMinimumSize(size);
    effect_layout->parentWidget()->adjustSize();

    switch (p_eType) {
    case IN: {
        Backend::instance()->removeInEffect(p_rChannel, fx);
        break;
    }
    case OUT: {
        Backend::instance()->removeOutEffect(p_rChannel, fx);
        break;
    }
    case PRE: {
        Backend::instance()->removePreEffect(p_rChannel, fx);
        break;
    }
    case POST: {
        Backend::instance()->removePostEffect(p_rChannel, fx);
        break;
    }
    case SUB: {
        Backend::instance()->removeSubEffect(p_rChannel, fx);
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

void Widget::showMessage(const QString& msg, int msec)
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
ChannelWidget* Widget::getFaderWidget(ChannelType p_eType, QString p_rChannel)
{
    switch (p_eType) {
    case IN:
    	if (m_in.contains(p_rChannel)) {
	        return m_in[p_rChannel];
    	}
    	else {
    		return NULL;
    	}
    case PRE:
    	if (m_pre.contains(p_rChannel)) {
	        return m_pre[p_rChannel];
    	}
    	else {
    		return NULL;
    	}
    case POST:
    	if (m_post.contains(p_rChannel)) {
    	    return m_post[p_rChannel];
    	}
    	else {
    		return NULL;
    	}
    case SUB:
    	if (m_sub.contains(p_rChannel)) {
	        return m_sub[p_rChannel];
    	}
    	else {
    		return NULL;
    	}
    case OUT:
    default:
        return main_widget;
    }
}
void Widget::doSelect(ChannelType type, QString channel, bool p_bForce)
{
    ChannelWidget* channelW = getFaderWidget(m_eSelectType, m_sSelectChannel);

    if (channelW != NULL) {
        QFont font = channelW->getLabel()->font();
        font.setBold(false);
        channelW->getLabel()->setFont(font);
    }

// effect = new effectData;
    if (!p_bForce && m_eSelectType == type && m_sSelectChannel == channel) {
        return;
    }
    m_eSelectType = type;
    m_sSelectChannel = channel;

    channelW = getFaderWidget(m_eSelectType, m_sSelectChannel);

    QFont font = channelW->getLabel()->font();
    font.setBold(true);
    channelW->getLabel()->setFont(font);

    if (type == OUT) {
        effectName->setText(trUtf8("Main output"));
    } else {
        effectName->setText(Backend::instance()->getChannel(type, channel)->display_name);
    }
//    m_pEffectStart->show();

    foreach(effect* fx, m_lVisibleEffect) {
        fx->gui->hide();
        if (p_bForce) {
            effect_layout->removeWidget(fx->gui);
            disconnect(fx->gui, SIGNAL(removeClicked(LadspaFXProperties*, effect*)), this, SLOT(askRemoveFX(LadspaFXProperties*, effect*)));
            disconnect(fx->gui, SIGNAL(leftClicked(LadspaFXProperties*, effect*)), this, SLOT(askLeftFX(LadspaFXProperties*, effect*)));
            disconnect(fx->gui, SIGNAL(rightClicked(LadspaFXProperties*, effect*)), this, SLOT(askRightFX(LadspaFXProperties*, effect*)));
            //delete fx->gui; // core dump !!
//            fx->force = true;
            fx->gui = NULL;
        }
    }
    m_lVisibleEffect.clear();

    switch (m_eSelectType) {
    case IN: {
        if (!p_bForce) showMessage(trUtf8("Input \"%1\" selected.").arg(Backend::instance()->getInput(m_sSelectChannel)->display_name));
        list<effect*>* effects = Backend::instance()->getInEffects(m_sSelectChannel);
        for (list<effect*>::iterator i = effects->begin() ; i != effects->end() ; i++) {
            effect* effect = *i;
            displayFX(effect, m_eSelectType, m_sSelectChannel);
        }
        break;
    }
    case OUT: {
        if (!p_bForce) showMessage(trUtf8("Output selected."));
        list<effect*>* effects = Backend::instance()->getOutEffects(m_sSelectChannel);
        for (list<effect*>::iterator i = effects->begin() ; i != effects->end() ; i++) {
            effect* effect = *i;
            displayFX(effect, m_eSelectType, m_sSelectChannel);
        }
        break;
    }
    case PRE: {
        if (!p_bForce) showMessage(trUtf8("Pre fader aux \"%1\" selected.").arg(Backend::instance()->getPre(m_sSelectChannel)->display_name));
        list<effect*>* effects = Backend::instance()->getPreEffects(m_sSelectChannel);
        for (list<effect*>::iterator i = effects->begin() ; i != effects->end() ; i++) {
            effect* effect = *i;
            displayFX(effect, m_eSelectType, m_sSelectChannel);
        }
        break;
    }
    case POST: {
        if (!p_bForce) showMessage(trUtf8("Post fader aux \"%1\" selected.").arg(Backend::instance()->getPost(m_sSelectChannel)->display_name));
        list<effect*>* effects = Backend::instance()->getPostEffects(m_sSelectChannel);
        for (list<effect*>::iterator i = effects->begin() ; i != effects->end() ; i++) {
            effect* effect = *i;
            displayFX(effect, m_eSelectType, m_sSelectChannel);
        }
        break;
    }
    case SUB: {
        if (!p_bForce) showMessage(trUtf8("Sub-groupe \"%1\" selected.").arg(Backend::instance()->getSub(m_sSelectChannel)->display_name));
        list<effect*>* effects = Backend::instance()->getSubEffects(m_sSelectChannel);
        for (list<effect*>::iterator i = effects->begin() ; i != effects->end() ; i++) {
            effect* effect = *i;
            displayFX(effect, m_eSelectType, m_sSelectChannel);
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

float Widget::getNewValue(float p_fOld, float p_fNew)
{
#define FACTOR 0
    return (p_fOld * FACTOR + p_fNew) / (1 + FACTOR);
}
void Widget::update()
{
    {
        float value1 = getNewValue(cpuLoad->getValue(), Backend::instance()->getProcessTime() / Backend::instance()->getMaxProcessTime());
        float value2 = getNewValue(cpuLoad->getValue2(), Backend::instance()->getCPULoad() / 100.0);
        cpuLoad->setValue(value1);
        cpuLoad->setValue2(value2);
        cpuLoad->setToolTip(trUtf8("- %CPU used by LiveMix backend (%1 %).\n- CPU load given by Jack (%2 %).")
                            .arg((int)(value1*100)).arg((int)(value2*100)));
    }

    foreach(QString in_name, Backend::instance()->inchannels()) {
        if (m_in[in_name] != NULL) {
            FWidget* fader = m_in[in_name]->fader;
            fader->getMeter()->setDbPeak_L(getNewValue(fader->getMeter()->getDbPeak_L(), Backend::instance()->getInPeak(in_name, true)));
            fader->getMeter()->setDbPeak_R(getNewValue(fader->getMeter()->getDbPeak_R(), Backend::instance()->getInPeak(in_name, false)));
        }
    }
    foreach(QString pre_name, Backend::instance()->prechannels()) {
        if (m_pre[pre_name] != NULL) {
            FWidget* fader = m_pre[pre_name]->fader;
            fader->getMeter()->setDbPeak_L(getNewValue(fader->getMeter()->getDbPeak_L(), Backend::instance()->getPrePeak(pre_name, true)));
            fader->getMeter()->setDbPeak_R(getNewValue(fader->getMeter()->getDbPeak_R(), Backend::instance()->getPrePeak(pre_name, false)));
        }
    }
    foreach(QString post_name, Backend::instance()->postchannels()) {
        if (m_post[post_name] != NULL) {
            FWidget* fader = m_post[post_name]->fader;
            fader->getMeter()->setDbPeak_L(getNewValue(fader->getMeter()->getDbPeak_L(), Backend::instance()->getPostPeak(post_name, true)));
            fader->getMeter()->setDbPeak_R(getNewValue(fader->getMeter()->getDbPeak_R(), Backend::instance()->getPostPeak(post_name, false)));
        }
    }
    foreach(QString sub_name, Backend::instance()->subchannels()) {
        if (m_sub[sub_name] != NULL) {
            FWidget* fader = m_sub[sub_name]->fader;
            fader->getMeter()->setDbPeak_L(getNewValue(fader->getMeter()->getDbPeak_L(), Backend::instance()->getSubPeak(sub_name, true)));
            fader->getMeter()->setDbPeak_R(getNewValue(fader->getMeter()->getDbPeak_R(), Backend::instance()->getSubPeak(sub_name, false)));
        }
    }
    main_widget->fader->getMeter()->setDbPeak_L(getNewValue(main_widget->fader->getMeter()->getDbPeak_L(), Backend::instance()->getOutPeak(MAIN, true)));
    main_widget->fader->getMeter()->setDbPeak_R(getNewValue(main_widget->fader->getMeter()->getDbPeak_L(), Backend::instance()->getOutPeak(MAIN, false)));
}
void Widget::init()
{
    main_widget->update();
    foreach(QString in_name, Backend::instance()->inchannels()) {
        addinchannel(in_name, false);
    }
    foreach(QString name, Backend::instance()->prechannels()) {
        addprechannel(name);
    }
    foreach(QString name, Backend::instance()->postchannels()) {
        addpostchannel(name, false);
    }
    foreach(QString name, Backend::instance()->subchannels()) {
        addsubchannel(name);
    }
    doSelect(OUT, MAIN);
}

void Widget::addinchannel(QString name, bool related)
{
    InWidget* elem = new InWidget(name, this);
    in_layout->addWidget(elem);
    m_in[name] = elem;
    connect(elem, SIGNAL(clicked(ChannelType, QString)), this, SLOT(select(ChannelType, QString)));

    if (related) {
        QMapIterator<QString, PreWidget *> iter_pre(m_pre);
        while (iter_pre.hasNext()) {
            iter_pre.next();
            elem->addPre(name, iter_pre.key());
        }

        QMapIterator<QString, PostWidget *> iter_post(m_post);
        while (iter_post.hasNext()) {
            iter_post.next();
            elem->addPost(name, iter_post.key());
        }

        QMapIterator<QString, SubWidget *> iter_sub(m_sub);
        while (iter_sub.hasNext()) {
            iter_sub.next();
            elem->addSub(name, iter_sub.key());
        }
    }
}
void Widget::addprechannel(QString name)
{
    PreWidget* elem = new PreWidget(name, this);
    pre_layout->addWidget(elem);
    m_pre[name] = elem;
    connect(elem, SIGNAL(clicked(ChannelType, QString)), this, SLOT(select(ChannelType, QString)));

    info_widget->addPre(name);
    QMapIterator<QString, InWidget *> iter(m_in);
    while (iter.hasNext()) {
        iter.next();
	if (iter.value() == NULL) {
	    m_in.remove(iter.key());
	}
        iter.value()->addPre(iter.key(), name);
    }
}
void Widget::addpostchannel(QString name, bool related)
{
    PostWidget* elem = new PostWidget(name, this);
    post_layout->addWidget(elem);
    m_post[name] = elem;
    connect(elem, SIGNAL(clicked(ChannelType, QString)), this, SLOT(select(ChannelType, QString)));

    info_widget->addPost(name);
    QMapIterator<QString, InWidget *> iter(m_in);
    while (iter.hasNext()) {
        iter.next();
	if (iter.value() == NULL) {
	    m_in.remove(iter.key());
	}
        iter.value()->addPost(iter.key(), name);
    }

    if (related) {
        QMapIterator<QString, SubWidget *> iter(m_sub);
        while (iter.hasNext()) {
            iter.next();
            elem->addSub(name, iter.key());
        }
    }
}
void Widget::addsubchannel(QString name)
{
    SubWidget* elem = new SubWidget(name, this);
    sub_layout->addWidget(elem);
    m_sub[name] = elem;
    connect(elem, SIGNAL(clicked(ChannelType, QString)), this, SLOT(select(ChannelType, QString)));

    info_widget->addSub(name);
    QMapIterator<QString, InWidget *> iter_in(m_in);
    while (iter_in.hasNext()) {
        iter_in.next();
        iter_in.value()->addSub(iter_in.key(), name);
    }
    QMapIterator<QString, PostWidget *> iter_post(m_post);
    while (iter_post.hasNext()) {
        iter_post.next();
        iter_post.value()->addSub(iter_post.key(), name);
    }
}
void Widget::removeinchannel(QString name)
{
    InWidget *elem = m_in[name];
    m_in.remove(name);
    in_layout->removeWidget(elem);
    delete elem;
}
void Widget::removeprechannel(QString name)
{
    PreWidget *elem = m_pre[name];
    m_pre.remove(name);
    pre_layout->removeWidget(elem);
    delete elem;

    m_bVisible[TO_PRE]->remove(name);

    info_widget->removePre(name);
    for (QMap<QString, InWidget*>::iterator i = m_in.begin() ; i != m_in.end() ; ++i) {
        i.value()->removePre(i.key(), name);
    }
}
void Widget::removepostchannel(QString name)
{
    PostWidget *elem = m_post[name];
    m_post.remove(name);
    post_layout->removeWidget(elem);
    delete elem;

    m_bVisible[TO_POST]->remove(name);

    info_widget->removePost(name);
    for (QMap<QString, InWidget*>::iterator i = m_in.begin() ; i != m_in.end() ; ++i) {
        i.value()->removePost(i.key(), name);
    }
}
void Widget::removesubchannel(QString name)
{
    SubWidget *elem = m_sub[name];
    m_sub.remove(name);
    sub_layout->removeWidget(elem);
    delete elem;

    m_bVisible[TO_SUB]->remove(name);

    info_widget->removeSub(name);
    for (QMap<QString, InWidget*>::iterator i = m_in.begin() ; i != m_in.end() ; ++i) {
        i.value()->removeSub(i.key(), name);
    }
    for (QMap<QString, PostWidget*>::iterator i = m_post.begin() ; i != m_post.end() ; ++i) {
        i.value()->removeSub(i.key(), name);
    }
}

void Widget::clearAll()
{
    typedef QMap<QString, bool>* Map;
    foreach(Map map, m_bVisible.values()) {
        map->empty();
    }
    clearKeyToWrapp();
    clearMidiToWrapp();
}

void Widget::leftClick(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_sReatedChannelName, QMouseEvent*)
{
    action(p_eType, p_sChannelName, p_eElement, p_sReatedChannelName);
}

QString Widget::getDisplayNameOfChannel(ChannelType p_eType, QString p_sChannelName)
{
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
QString Widget::getDisplayChannelType(ChannelType p_eType, bool p_bUpperFirst)
{
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
QString Widget::getShortDisplayChannelType(ChannelType p_eType, bool p_bUpperFirst)
{
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

QString Widget::getDisplayFunction(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_sReatedChannelName, bool p_bUpperFirst)
{
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
        displayName = trUtf8("main output");
        break;
    case FADER:
        if (p_sReatedChannelName == MAIN) {
            displayName = trUtf8("main volume");
        } else if (p_sReatedChannelName == PFL) {
            displayName = trUtf8("phone volume");
        } else if (p_sReatedChannelName == MONO) {
            displayName = trUtf8("mono volume");
        } else {
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
QString Widget::getMediumDisplayFunction(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_sReatedChannelName, bool p_bUpperFirst)
{
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
        } else if (p_sReatedChannelName == PFL) {
            displayName = trUtf8("phone volume");
        } else if (p_sReatedChannelName == MONO) {
            displayName = trUtf8("mono volume");
        } else {
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
QString Widget::getShortDisplayFunction(ElementType p_eElement, QString p_sReatedChannelName, bool p_bStereo)
{
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
        } else if (p_sReatedChannelName == PFL) {
            displayName = trUtf8("phone");
        } else if (p_sReatedChannelName == MONO) {
            displayName = trUtf8("mono");
        } else {
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
    unsigned char iChannel = -1;
    unsigned int iController = -1;
    foreach(QKeySequence rKey, m_mKeyToWrapp.keys()) {
        KeyDo* pKeyDo = m_mKeyToWrapp[rKey];
        if (typeid(*pKeyDo).name() == typeid(KeyDoDirectAction).name()) {
            KeyDoDirectAction* pKD = (KeyDoDirectAction*)pKeyDo;
            if (pKD->m_eType == p_eType && pKD->m_sChannelName == p_sChannelName
                    && pKD->m_eElement == p_eElement && pKD->m_sReatedChannelName == p_sReatedChannelName) {
                rActionOnChannelKeySequence = rKey;
            }
        } else if (typeid(*pKeyDo).name() == typeid(KeyDoSelectChannel).name()) {
            KeyDoSelectChannel* pKD = (KeyDoSelectChannel*)pKeyDo;
            if (pKD->m_eType == p_eType && pKD->m_sChannelName == p_sChannelName) {
                rSelectChannelKeySequence = rKey;
            }
        } else if (typeid(*pKeyDo).name() == typeid(KeyDoChannelAction).name()) {
            KeyDoChannelAction* pKD = (KeyDoChannelAction*)pKeyDo;
            if (pKD->m_eElement == p_eElement && pKD->m_sReatedChannelName == p_sReatedChannelName) {
                rActionOnSelectedChannelKeySequence = rKey;
            }
        }
    }
    foreach(unsigned char ch, m_mMidiToWrapp.keys()) {
        foreach(unsigned int co, m_mMidiToWrapp[ch]->keys()) {
            KeyDoDirectAction *pKD = (*m_mMidiToWrapp[ch])[co];
            if (pKD->m_eType == p_eType && pKD->m_sChannelName == p_sChannelName
                    && pKD->m_eElement == p_eElement && pKD->m_sReatedChannelName == p_sReatedChannelName) {
                iChannel = ch;
                iController = co;
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
    /* if (p_sReatedChannelName != "") {
      displayElement += " " + p_sDisplayReatedChannelName;
     }*/

    AssigneToPannel* panel = new AssigneToPannel(displayName, displayElement, volume, only_dirrect,
            rActionOnChannelKeySequence, rSelectChannelKeySequence, rActionOnSelectedChannelKeySequence,
            iChannel, iController);

    m_bLearn = true;
    if (panel->exec() == QDialog::Accepted) {
        if (panel->getActionOnChannelKeySequence() != QKeySequence()) {
            if (panel->getActionOnChannelKeySequence() != rActionOnChannelKeySequence) {
                if ((!m_mKeyToWrapp.contains(panel->getActionOnChannelKeySequence())) || QMessageBox::question(this, trUtf8("Reassigne key")
                        , trUtf8("Does I reassigne the %1 key ?").arg(panel->getActionOnChannelKeySequence().toString())
                        , QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)) {
                    delete m_mKeyToWrapp[rActionOnChannelKeySequence];
                    delete m_mKeyToWrapp[panel->getActionOnChannelKeySequence()];
                    m_mKeyToWrapp.remove(rActionOnChannelKeySequence);
                    m_mKeyToWrapp.insert(panel->getActionOnChannelKeySequence()
                                         , new KeyDoDirectAction(this, p_eType, p_sChannelName, p_eElement, p_sReatedChannelName));
                }
            }
        } else {
            delete m_mKeyToWrapp[rActionOnChannelKeySequence];
            m_mKeyToWrapp.remove(rActionOnChannelKeySequence);
        }
        if (panel->getSelectChannelKeySequence() != QKeySequence()) {
            if (panel->getSelectChannelKeySequence() != rSelectChannelKeySequence) {
                if ((!m_mKeyToWrapp.contains(panel->getSelectChannelKeySequence())) || QMessageBox::question(this, trUtf8("Reassigne key")
                        , trUtf8("Does I reassigne the %1 key").arg(panel->getSelectChannelKeySequence().toString())
                        , QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)) {
                    delete m_mKeyToWrapp[rSelectChannelKeySequence];
                    delete m_mKeyToWrapp[panel->getSelectChannelKeySequence()];
                    m_mKeyToWrapp.remove(rSelectChannelKeySequence);
                    m_mKeyToWrapp.insert(panel->getSelectChannelKeySequence(), new KeyDoSelectChannel(this, p_eType, p_sChannelName));
                }
            }
        } else {
            delete m_mKeyToWrapp[rSelectChannelKeySequence];
            m_mKeyToWrapp.remove(rSelectChannelKeySequence);
        }
        if (panel->getActionOnSelectedChannelKeySequence() != QKeySequence()) {
            if (panel->getActionOnSelectedChannelKeySequence() != rActionOnSelectedChannelKeySequence) {
                if ((!m_mKeyToWrapp.contains(panel->getActionOnSelectedChannelKeySequence())) || QMessageBox::question(this, trUtf8("Reassigne key")
                        , trUtf8("Does I reassigne the %1 key").arg(panel->getActionOnSelectedChannelKeySequence().toString())
                        , QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)) {
                    delete m_mKeyToWrapp[rActionOnSelectedChannelKeySequence];
                    delete m_mKeyToWrapp[panel->getActionOnSelectedChannelKeySequence()];
                    m_mKeyToWrapp.remove(rActionOnSelectedChannelKeySequence);
                    m_mKeyToWrapp.insert(panel->getActionOnSelectedChannelKeySequence(), new KeyDoChannelAction(this, p_eElement,
                                         p_sReatedChannelName));
                }
            }
        } else {
            delete m_mKeyToWrapp[rActionOnSelectedChannelKeySequence];
            m_mKeyToWrapp.remove(rActionOnSelectedChannelKeySequence);
        }

        if (panel->getChannel() != ((unsigned char)-1) && panel->getChannel() != iChannel && panel->getController() != iController)
            if (!m_mMidiToWrapp.contains(panel->getChannel()) || !m_mMidiToWrapp[panel->getChannel()]->contains(panel->getController()) ||
                    QMessageBox::question(this, trUtf8("Reassigne MIDI")
                                          , trUtf8("Does I reassigne the MIDI controler (%1 / %2)").arg(panel->getChannel()).arg(panel->getController())
                                          , QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)) {
                if (m_mMidiToWrapp.contains(panel->getChannel())) {
                    QMap<unsigned int, KeyDoDirectAction*>* map = m_mMidiToWrapp[panel->getChannel()];
                    if (map->contains(panel->getController())) {
                        delete(*map)[panel->getController()];
                        map->remove(panel->getController());
                    }
                }

                if (m_mMidiToWrapp.contains(iChannel)) {
                    QMap<unsigned int, KeyDoDirectAction*>* map = m_mMidiToWrapp[iChannel];
                    if (map->contains(iController)) {
                        delete(*map)[iController];
                        map->remove(iController);
                    }
                }

                if (!m_mMidiToWrapp.contains(panel->getChannel())) {
                    m_mMidiToWrapp.insert(panel->getChannel(), new QMap<unsigned int, KeyDoDirectAction*>);
                }

                m_mMidiToWrapp[panel->getChannel()]->insert(panel->getController(),
                        new KeyDoDirectAction(this, p_eType, p_sChannelName, p_eElement, p_sReatedChannelName));

                Wrapp *w = (*(*(*m_mShurtCut[p_eType])[p_sChannelName])[p_eElement])[p_sReatedChannelName];
                int value = 0;
                Toggle* t;
                Volume* vol;
                switch (p_eElement) {
                case MUTE:
                case MUTE_EFFECT:
                case TO_SUB:
                case TO_MAIN:
                case TO_PFL:
                case TO_AFL:
                    t = ((WrappToggle*)w)->getToggle();
                    value = t->getValue() ? 127 : 0;
                    break;
                default:
                    vol = ((WrappVolume*)w)->getVolume();
                    value = (int)((vol->getValue() - vol->getMinValue()) * 127 / (vol->getMaxValue() - vol->getMinValue()));
                }
                Backend::instance()->sendMidiEvent(panel->getChannel(), panel->getController(), value);
            }
    }
    m_bLearn = false;
}

void Widget::sendMidiEvent(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement,
                           QString p_sReatedChannelName, float p_fValue)
{
    foreach(unsigned char ch, m_mMidiToWrapp.keys()) {
        foreach(unsigned int co, m_mMidiToWrapp[ch]->keys()) {
            KeyDoDirectAction *act = (*m_mMidiToWrapp[ch])[co];
            if (act->m_eType == p_eType && act->m_sChannelName == p_sChannelName && act->m_eElement == p_eElement
                    && act->m_sReatedChannelName == p_sReatedChannelName) {
                int value = 0;
                switch (p_eElement) {
                case MUTE:
                case MUTE_EFFECT:
                case TO_SUB:
                case TO_MAIN:
                case TO_PFL:
                case TO_AFL:
                    value = p_fValue == 0 ? 0 : 127;
                    break;
                default:
                    Wrapp *w = (*(*(*m_mShurtCut[p_eType])[p_sChannelName])[p_eElement])[p_sReatedChannelName];
                    Volume *vol = ((WrappVolume*)w)->getVolume();
                    value = (int)((p_fValue - vol->getMinValue()) * 127 / (vol->getMaxValue() - vol->getMinValue()));
                }
                Backend::instance()->sendMidiEvent(ch, co, value);
            }
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

    QAction* rename = new QAction(trUtf8("Rename"), this);
    ChannelWidget* channelWidget = getFaderWidget(m_eSelectType, m_sSelectChannel);
    connect(rename, SIGNAL(triggered()), channelWidget->fader, SLOT(changeName()));
    menu.addAction(rename);

    QAction* addfx = new QAction(trUtf8("Add an effect"), this);
    connect(addfx, SIGNAL(triggered()), this, SLOT(addFX()));
    menu.addAction(addfx);

    QAction* assigne = new QAction(trUtf8("Assigne key"), this);
    connect(assigne, SIGNAL(triggered()), this, SLOT(assigneKey()));
    menu.addAction(assigne);

    switch (p_eElement) {
    case MUTE:
    case TO_SUB:
    case TO_MAIN:
    case TO_PFL:
    case TO_AFL: {
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
        QAction* set = new QAction(trUtf8("Set value to %1").arg(
                                       getMediumDisplayFunction(p_eType, p_sChannelName, p_eElement, p_sReatedChannelName, false)), this);
        connect(set, SIGNAL(triggered()), this, SLOT(newValue()));
        menu.addAction(set);

        if (p_eType == IN) {
            QAction* setall = new QAction(trUtf8("Set value to all %1").arg(
                                              getMediumDisplayFunction(p_eType, p_sChannelName, p_eElement, p_sReatedChannelName, false)), this);
            connect(setall, SIGNAL(triggered()), this, SLOT(newLineValue()));
            menu.addAction(setall);
        }

        QAction* restet = new QAction(trUtf8("Reset all %1").arg(
                                          getMediumDisplayFunction(p_eType, p_sChannelName, p_eElement, p_sReatedChannelName, false)), this);
        connect(restet, SIGNAL(triggered()), this, SLOT(resetAllTheLine()));
        menu.addAction(restet);
    }
    menu.exec(p_pEvent->globalPos());
}

void Widget::newValue()
{
    newValue(m_eSelectType, m_sSelectChannel, m_eSelectedElement, m_sSelectedReatedChannelName, false);
}

void Widget::newLineValue()
{
    newValue(m_eSelectType, m_sSelectChannel, m_eSelectedElement, m_sSelectedReatedChannelName, true);
}

void Widget::mouseDoubleClickEvent(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_sReatedChannelName, QMouseEvent*)
{
    newValue(p_eType, p_sChannelName, p_eElement, p_sReatedChannelName, false);
}

void Widget::newValue(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_sReatedChannelName, bool p_bToLine)
{
    switch (p_eElement) {
    case MUTE:
    case MUTE_EFFECT:
    case TO_SUB:
    case TO_MAIN:
    case TO_PFL:
    case TO_AFL:
        break;
    default:
        Volume* vol = ((WrappVolume*)(*(*(*m_mShurtCut[p_eType])[p_sChannelName])[p_eElement])[p_sReatedChannelName])->getVolume();
        bool ok;
        double value = QInputDialog::getDouble(vol, p_bToLine ? trUtf8("New value to assigne to line") : trUtf8("New value to assigne"),
                                               getDisplayFunction(p_eType, p_sChannelName, p_eElement, p_sReatedChannelName),
                                               vol->getValue(), vol->getMinValue(), vol->getMaxValue(), 1, &ok);
        if (ok) {
            if (p_bToLine) {
                foreach(QString j, m_mShurtCut[p_eType]->keys()) {
                    if ((*m_mShurtCut[p_eType])[j]->contains(m_eSelectedElement) &&
                            (*(*m_mShurtCut[IN])[j])[m_eSelectedElement]->contains(m_sSelectedReatedChannelName)) {
                        Volume* vol = ((WrappVolume*)(*(*(*m_mShurtCut[IN])[j])[m_eSelectedElement])[m_sSelectedReatedChannelName])->getVolume();
                        vol->setValue(value, true);
                    }
                }
            } else {
                vol->setValue(value, true);
            }
        }
    }
}

void Widget::assigneKey()
{
    middleClick(m_eSelectType, m_sSelectChannel, m_eSelectedElement, m_sSelectedReatedChannelName, NULL);
}

void Widget::resetAllTheLine()
{
    foreach(ChannelType i, m_mShurtCut.keys()) {
        foreach(QString j, m_mShurtCut[i]->keys()) {
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

void Widget::enableAllTheLine()
{
    foreach(ChannelType i, m_mShurtCut.keys()) {
        foreach(QString j, m_mShurtCut[i]->keys()) {
            if ((*m_mShurtCut[i])[j]->contains(m_eSelectedElement) && (*(*m_mShurtCut[i])[j])[m_eSelectedElement]->contains(m_sSelectedReatedChannelName)) {
                Toggle* vol = ((WrappToggle*)(*(*(*m_mShurtCut[i])[j])[m_eSelectedElement])[m_sSelectedReatedChannelName])->getToggle();
                vol->setValue(true, true);
            }
        }
    }
}
void Widget::desableAllTheLine()
{
    foreach(ChannelType i, m_mShurtCut.keys()) {
        foreach(QString j, m_mShurtCut[i]->keys()) {
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
            } else if (p_sChannelName == MAIN && p_sReatedChannelName == MONO) {
                showMessage(trUtf8("Mono selected."));
            } else if (p_sChannelName == MAIN && p_sReatedChannelName == MAIN) {
                showMessage(trUtf8("Main fader selected."));
            } else {
                QString elem = getDisplayFunction(p_eType, p_sChannelName, p_eElement, p_sReatedChannelName);
                /*             elem = elem.left(1).toUpper() + elem.right(elem.size() - 1);
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
void Widget::keyPressEvent(QKeyEvent * p_pEvent)
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
    if (m_pSelectedWrapper != NULL) {
        m_pSelectedWrapper->getVolume()->incValue(p_pEvent->delta() > 0);
    }
}

void Widget::clearKeyToWrapp()
{
    foreach(KeyDo* keyDo, m_mKeyToWrapp) {
        delete keyDo;
    }
    m_mKeyToWrapp.clear();
}

void Widget::clearMidiToWrapp()
{
    foreach(unsigned char p_iChannel, m_mMidiToWrapp.keys()) {
        foreach(KeyDo* keyDo, *m_mMidiToWrapp[p_iChannel]) {
            delete keyDo;
        }
        m_mMidiToWrapp[p_iChannel]->clear();
        delete m_mMidiToWrapp[p_iChannel];
    }
    m_mMidiToWrapp.clear();
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
    } else {
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
    } else {
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
                delete(*(*(*m_mShurtCut[p_eType])[p_sChannelName])[type])[sub];
                (*(*m_mShurtCut[p_eType])[p_sChannelName])[type]->remove(sub);
            }
            delete(*(*m_mShurtCut[p_eType])[p_sChannelName])[type];
            (*m_mShurtCut[p_eType])[p_sChannelName]->remove(type);
        }
        delete(*m_mShurtCut[p_eType])[p_sChannelName];
        m_mShurtCut[p_eType]->remove(p_sChannelName);
    }
    foreach(QKeySequence key, m_mKeyToWrapp.keys()) {
        if (typeid(*m_mKeyToWrapp[key]).name() == typeid(KeyDoSelectChannel).name()) {
            KeyDoSelectChannel* keyDo = (KeyDoSelectChannel*)m_mKeyToWrapp[key];
            if (keyDo->m_eType == p_eType && keyDo->m_sChannelName == p_sChannelName) {
                delete keyDo;
                m_mKeyToWrapp.remove(key);
            }
        } else if (typeid(*m_mKeyToWrapp[key]).name() == typeid(KeyDoDirectAction).name()) {
            KeyDoDirectAction* keyDo = (KeyDoDirectAction*)m_mKeyToWrapp[key];
            if (keyDo->m_eType == p_eType && keyDo->m_sChannelName == p_sChannelName) {
                delete keyDo;
                m_mKeyToWrapp.remove(key);
            }
        }
    }

}
bool Widget::isVisible(ElementType p_eElement, QString p_rChannelTo)
{
    if (m_bVisible.contains(p_eElement)) {
        if (m_bVisible[p_eElement]->contains(p_rChannelTo)) {
            return (*m_bVisible[p_eElement])[p_rChannelTo];
        } else {
            return true;
        }
    } else {
        return true;
    }
}
void Widget::setVisible(bool p_bVisible, ElementType p_eElement, QString p_rChannelTo)
{
    if (!m_bVisible.contains(p_eElement)) {
        m_bVisible.insert(p_eElement, new QMap<QString, bool>);
    }
    m_bVisible[p_eElement]->insert(p_rChannelTo, p_bVisible);

    int height = info_widget->setVisible(p_bVisible, p_eElement, p_rChannelTo);

    foreach(ChannelType i, m_mShurtCut.keys()) {
        foreach(QString j, m_mShurtCut[i]->keys()) {
            if ((*m_mShurtCut[i])[j]->contains(p_eElement) && (*(*m_mShurtCut[i])[j])[p_eElement]->contains(p_rChannelTo)) {
                ((WrappVolume*)(*(*(*m_mShurtCut[i])[j])[p_eElement])[p_rChannelTo])->getVolume()->setVisible(p_bVisible);
            }
        }
    }
    if (p_eElement == TO_PFL) {
        foreach(ChannelType i, m_mShurtCut.keys()) {
            foreach(QString j, m_mShurtCut[i]->keys()) {
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
}
void Widget::setFaderHeight(int p_iHeight)
{
    int diff = m_iFaderHeight - p_iHeight;
    if (p_iHeight < 200) {
        p_iHeight = 200;
    }
    m_iFaderHeight = p_iHeight;

    info_widget->m_pFader->setFixedHeight(m_iFaderHeight);

    foreach(ChannelType i, m_mShurtCut.keys()) {
        foreach(QString j, m_mShurtCut[i]->keys()) {
            foreach(QString sub, (*(*m_mShurtCut[i])[j])[FADER]->keys()) {
                if (sub != PFL && sub != MONO) {
                    Wrapp* w = (*(*(*m_mShurtCut[i])[j])[FADER])[sub];
                    ((Fader*)((WrappVolume*)w)->getVolume())->setFixedHeight(m_iFaderHeight);
//                    ((FWidget*)((WrappVolume*)w)->getVolume()->parent())->getMeter()->setFixedHeight(m_iFaderHeight);
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
void Widget::showAll() 
{
    setVisible(true, GAIN);
    setVisible(true, MUTE);
    setVisible(true, PAN_BAL);
    setVisible(true, TO_MAIN);
//    setVisible(true, FADER);
    setVisible(true, TO_AFL);
    setVisible(true, TO_PFL);
    setVisible(true, PRE_VOL);
    setVisible(true, MUTE_EFFECT);
    foreach(QString name, Backend::instance()->prechannels()) {
        setVisible(true, TO_PRE, name);
    }
    foreach(QString name, Backend::instance()->postchannels()) {
        setVisible(true, TO_POST, name);
    }
    foreach(QString name, Backend::instance()->subchannels()) {
        setVisible(true, TO_SUB, name);
    }
}
void Widget::hideAll()
{
    setVisible(false, GAIN);
    setVisible(false, MUTE);
    setVisible(false, PAN_BAL);
    setVisible(false, TO_MAIN);
//    setVisible(false, FADER);
    setVisible(false, TO_AFL);
    setVisible(false, TO_PFL);
    setVisible(false, PRE_VOL);
    setVisible(false, MUTE_EFFECT);
    foreach(QString name, Backend::instance()->prechannels()) {
        setVisible(false, TO_PRE, name);
    }
    foreach(QString name, Backend::instance()->postchannels()) {
        setVisible(false, TO_POST, name);
    }
    foreach(QString name, Backend::instance()->subchannels()) {
        setVisible(false, TO_SUB, name);
    }
}
void Widget::faderHeight()
{
    bool ok;
    int result = QInputDialog::getInteger(this, trUtf8("Change the fader height"), trUtf8("New fader height"), m_iFaderHeight, 150, 500, 1, &ok);
    if (ok) {
        setFaderHeight(result);
    }
}

void Widget::setEffectFaderHeight(int p_iHeight)
{
    int diff = m_iEffectFaderHeight - p_iHeight;
    if (p_iHeight < 150) {
        p_iHeight = 150;
    }
    m_iEffectFaderHeight = p_iHeight;

    effectName->setFixedHeight(m_iEffectFaderHeight+50);

    m_pEffectScrollArea->setFixedHeight(m_iEffectFaderHeight + 102);
    m_pEffectStart->setFixedHeight(m_iEffectFaderHeight + 64);
    effect_layout->parentWidget()->setFixedHeight(m_iEffectFaderHeight + 82);

    switch (m_eSelectType) {
        case IN: {
            foreach(QString channel, Backend::instance()->inchannels()) {
                list<effect*>* effects = Backend::instance()->getInEffects(m_sSelectChannel);
                for (list<effect*>::iterator i = effects->begin() ; i != effects->end() ; i++) {
                    effect* effect = *i;
                    if (effect->gui != NULL) {
                        effect->gui->setFaderHeight(m_iEffectFaderHeight);
                    }
                }
            }
            break;
        }
        case PRE: {
            foreach(QString channel, Backend::instance()->prechannels()) {
                list<effect*>* effects = Backend::instance()->getPreEffects(m_sSelectChannel);
                for (list<effect*>::iterator i = effects->begin() ; i != effects->end() ; i++) {
                    effect* effect = *i;
                    if (effect->gui != NULL) {
                        effect->gui->setFaderHeight(m_iEffectFaderHeight);
                    }
                }
            }
            break;
        }
        case POST: {
            foreach(QString channel, Backend::instance()->postchannels()) {
                list<effect*>* effects = Backend::instance()->getPostEffects(m_sSelectChannel);
                for (list<effect*>::iterator i = effects->begin() ; i != effects->end() ; i++) {
                    effect* effect = *i;
                    if (effect->gui != NULL) {
                        effect->gui->setFaderHeight(m_iEffectFaderHeight);
                    }
                }
            }
            break;
        }
        case SUB: {
            foreach(QString channel, Backend::instance()->subchannels()) {
                list<effect*>* effects = Backend::instance()->getSubEffects(m_sSelectChannel);
                for (list<effect*>::iterator i = effects->begin() ; i != effects->end() ; i++) {
                    effect* effect = *i;
                    if (effect->gui != NULL) {
                        effect->gui->setFaderHeight(m_iEffectFaderHeight);
                    }
                }
            }
            break;
        }
        case OUT: {
            foreach(QString channel, Backend::instance()->outchannels()) {
                list<effect*>* effects = Backend::instance()->getOutEffects(channel);
                for (list<effect*>::iterator i = effects->begin() ; i != effects->end() ; i++) {
                    effect* effect = *i;
                    if (effect->gui != NULL) {
                        effect->gui->setFaderHeight(m_iEffectFaderHeight);
                    }
                }
            }
            break;
        }
    }
    if (diff > 0) {
        QRect size = this->parentWidget()->geometry();
        size.setHeight(size.height() - diff);
        this->parentWidget()->layout()->invalidate();
        parentWidget()->setGeometry(size);
    }
}
void Widget::effectFaderHeight()
{
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
        , m_sChannelName(p_sChannelName)
{
    m_pVolume = new Fader(this, false, true, Fader::FADER);
    m_pMeter = new Fader(this, false, true, Fader::PK_VU);
    m_pLabelFader = new FaderName(this);

    setFixedSize(CHANNEL_WIDTH, p_fFaderHeignt);
    setFixedHeight(p_fFaderHeignt);
    m_pVolume->move(CHANNEL_WIDTH/2 -13, 0);
    m_pMeter->move(CHANNEL_WIDTH/2 +5, 0);
    m_pLabelFader->move(CHANNEL_WIDTH/2 -25, 0);

    if (p_eType == OUT) {
        m_pLabelFader->setText(trUtf8("Main output"));
    } else {
        m_pLabelFader->setText(Backend::instance()->getChannel(m_eType, m_sChannelName)->display_name);
        connect(m_pLabelFader, SIGNAL(doubleClicked()), this, SLOT(changeName()));
    }
}
Fader* FWidget::getFader()
{
    return (Fader*)m_pVolume;
}
Fader* FWidget::getMeter()
{
    return m_pMeter;
}
FaderName* FWidget::getLabelFader()
{
    return m_pLabelFader;
}
void FWidget::setFixedHeight(int h)
{
    QWidget::setFixedHeight(h);
    ((Fader*)m_pVolume)->setFixedHeight(h);
    m_pMeter->setFixedHeight(h);
    m_pLabelFader->setFixedHeight(h - 10);
}
void FWidget::changeName()
{
    bool ok;
    QString text = QInputDialog::getText(this, trUtf8("%1 \"%2\"").arg(Widget::getDisplayChannelType(m_eType)).arg(m_sChannelName),
                                         trUtf8("Display name"), QLineEdit::Normal, Backend::instance()->getChannel(m_eType, m_sChannelName)->display_name, &ok);
    if (ok && !text.isEmpty()) {
        Backend::instance()->getChannel(m_eType, m_sChannelName)->display_name = text;
        m_pLabelFader->setText(text);
    }
}

FWidget* Widget::createFader(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_rChannelTo)
{
    FWidget *fader = new FWidget(getFaderHeight(), p_eType, p_sChannelName);
    addVolume(fader, p_eType, p_sChannelName, p_eElement, p_rChannelTo);
    fader->setToolTip(getDisplayFunction(p_eType, p_sChannelName, p_eElement, p_rChannelTo));
    fader->getFader()->setToolTip(getDisplayFunction(p_eType, p_sChannelName, p_eElement, p_rChannelTo));
//    connect(fader, SIGNAL( dbValueChanged(QString, float) ), Backend::instance(), SLOT( setInVolume( QString, float ) ) );
    if (p_eType == IN || p_eType == POST) {
        fader->getFader()->setMaxValue(20);
        fader->getMeter()->setMaxPeak(20);
        fader->getFader()->setMinValue(-60);
        fader->getMeter()->setMinPeak(-60);
    } else {
        fader->getFader()->setMaxValue(0);
        fader->getMeter()->setMaxPeak(0);
        fader->getFader()->setMinValue(-80);
        fader->getMeter()->setMinPeak(-80);
    }
    fader->setDbValue(Backend::instance()->getChannel(p_eType, p_sChannelName)->getFloatAttribute(p_eElement, p_rChannelTo));
    return fader;
}


VWidget::VWidget()
{
}
Volume* VWidget::getVolume()
{
    return m_pVolume;
}
QWidget* VWidget::getWidget()
{
    return m_pVolume;
}
void VWidget::setValue(float p_fValue, bool p_bEmit, int p_iSource)
{
    m_pVolume->setValue(p_fValue, p_bEmit, p_iSource);
}
float VWidget::getValue()
{
    return m_pVolume->getValue();
}
void VWidget::setDbValue(float fValue)
{
    m_pVolume->setDbValue(fValue);
}
float VWidget::getDbValue()
{
    return m_pVolume->getDbValue();
}
float VWidget::getMinValue()
{
    return m_pVolume->getMinValue();
}
float VWidget::getMaxValue()
{
    return m_pVolume->getMaxValue();
}
void VWidget::incValue(bool p_bDirection, int p_iStep)
{
    m_pVolume->incValue(p_bDirection, p_iStep);
}

RWidget::RWidget(ElementType p_eElement, QString p_rToolTip)
{
    m_pVolume = new Rotary(this, p_eElement == PAN_BAL ? Rotary::TYPE_CENTER : Rotary::TYPE_NORMAL, p_rToolTip, false, true);
    setFixedSize(CHANNEL_WIDTH, 26);
    m_pVolume->move((CHANNEL_WIDTH - 28) / 2, 0);
    if (p_eElement == PAN_BAL) {
        m_pBackground = new QPixmap();
        if (!m_pBackground->load(":/data/bal_background.png")) {
            qDebug() << "Error loading image: 'bal_background.png'";
            m_pBackground = NULL;
        }
    } else {
        m_pBackground = new QPixmap();
        if (!m_pBackground->load(":/data/rotary_background.png")) {
            qDebug() << "Error loading image: 'rotary_background.png'";
            m_pBackground = NULL;
        }
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
Rotary* RWidget::getRotary()
{
    return (Rotary*)m_pVolume;
}
TWidget::TWidget()
{
    m_pToggle = ToggleButton::create(this);
    setFixedSize(CHANNEL_WIDTH, 16);
    m_pToggle->move((CHANNEL_WIDTH - 21) / 2, 0);
}
ToggleButton* TWidget::getToggle()
{
    return m_pToggle;
}
QWidget* TWidget::getWidget()
{
    return m_pToggle;
}
bool TWidget::getValue()
{
    return m_pToggle->getValue();
}
void TWidget::setValue(bool p_bValue, bool p_bEmit, int p_iSource)
{
    m_pToggle->setValue(p_bValue, p_bEmit, p_iSource);
}

VWidget* Widget::createRotary(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_rChannelTo)
{
    RWidget* rotary = new RWidget(p_eElement, getDisplayFunction(p_eType, p_sChannelName, p_eElement, p_rChannelTo));
    if (p_eElement == PAN_BAL) {
        rotary->setValue(Backend::instance()->getChannel(p_eType, p_sChannelName)->getFloatAttribute(p_eElement, p_rChannelTo));
    } else {
        rotary->setDbValue(Backend::instance()->getChannel(p_eType, p_sChannelName)->getFloatAttribute(p_eElement, p_rChannelTo));
    }
    if (p_eElement == GAIN) {
        rotary->getRotary()->setMaxValue(40);
    }
    if (!isVisible(p_eElement == TO_AFL ? TO_PFL : p_eElement, p_rChannelTo)) {
        rotary->setVisible(false);
    }
    addVolume(rotary, p_eType, p_sChannelName, p_eElement, p_rChannelTo);
    return rotary;
}
TWidget* Widget::createToggle(ChannelType p_eType, QString p_sChannelName, ElementType p_eElement, QString p_rChannelTo)
{
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

LFWidget::LFWidget(Widget *p_pWidget)
        : m_pWidget(p_pWidget)
{
    setCursor(QCursor(Qt::SizeVerCursor));
}
void LFWidget::mousePressEvent(QMouseEvent *p_pEvent)
{
    if (p_pEvent->button() == Qt::LeftButton) {
        m_fMousePressY = p_pEvent->y();
    }
}
void LFWidget::mouseReleaseEvent(QMouseEvent *p_pEvent)
{
    if (p_pEvent->button() == Qt::LeftButton) {
        m_pWidget->setEffectFaderHeight(m_pWidget->getEffectFaderHeight() + m_fMousePressY - p_pEvent->y());
    }
}

}
; //LiveMix
