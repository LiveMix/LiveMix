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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ChannelsWidgets.h"


namespace LiveMix
{

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

    Rotary *gain = m_pMatrix->createRotary(IN, p_sChannel, GAIN);
	gain->setVisible(m_pMatrix->isGainVisible());
    layout->addWidget(gain);

    ToggleButton* mute = m_pMatrix->createToggle(IN, p_sChannel, MUTE);
    layout->addWidget(mute);

    ToggleButton* plf = m_pMatrix->createToggle(IN, p_sChannel, TO_PLF);
    layout->addWidget(plf);

    Widget::addLine(layout);

    wPre = new QWidget;
    lPre = new QVBoxLayout;
    lPre->setSpacing(0);
    lPre->setMargin(0);
    wPre->setLayout(lPre);
    layout->addWidget(wPre);
    lPre->addWidget(new QLabel(m_pMatrix->getShortDisplayChannelType(PRE)));
    Widget::addLine(layout);
    lPre->parentWidget()->hide();

    wPost = new QWidget;
    lPost = new QVBoxLayout;
    lPost->setSpacing(0);
    lPost->setMargin(0);
    wPost->setLayout(lPost);
    layout->addWidget(wPost);
    lPost->addWidget(new QLabel(m_pMatrix->getShortDisplayChannelType(POST)));
    Widget::addLine(layout);
    lPost->parentWidget()->hide();

    wSub = new QWidget;
    lSub = new QVBoxLayout;
    lSub->setSpacing(0);
    lSub->setMargin(0);
    wSub->setLayout(lSub);
    layout->addWidget(wSub);
    lSub->addWidget(new QLabel(m_pMatrix->getShortDisplayChannelType(SUB)));
    Widget::addLine(layout);
    lSub->parentWidget()->hide();

    Rotary *bal = m_pMatrix->createRotary(IN, p_sChannel, PAN_BAL);
    layout->addWidget(bal);

    ToggleButton* main_on = m_pMatrix->createToggle(IN, p_sChannel, TO_MAIN);
    layout->addWidget(main_on);

    fader = m_pMatrix->createFader(IN, p_sChannel, FADER);
    layout->addWidget(fader);
}
InWidget::~InWidget()
{
	m_pMatrix->removeShurtCut(IN, m_Channel);
}
void InWidget::mouseReleaseEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton) {
        emit clicked(IN, m_Channel);
    }
}
void InWidget::addPre(QString channelIn, QString channelPre)
{
    Rotary *elem = m_pMatrix->createRotary(IN, channelIn, TO_PRE, channelPre);
    lPre->addWidget(elem);
    pre[channelPre] = elem;
    if (Backend::instance()->prechannels().size() > 0) {
        lPre->parentWidget()->show();
    }
}
void InWidget::addPost(QString channelIn, QString channelPost)
{
    Rotary *elem = m_pMatrix->createRotary(IN, channelIn, TO_POST, channelPost);
    lPost->addWidget(elem);
    post[channelPost] = elem;
    if (Backend::instance()->postchannels().size() > 0) {
        lPost->parentWidget()->show();
    }
}
void InWidget::addSub(QString channelIn, QString channelSub)
{
    ToggleButton* elem = m_pMatrix->createToggle(IN, channelIn, TO_SUB, channelSub);
    elem->setToolTip(channelSub);
    lSub->addWidget(elem);
    sub[channelSub] = elem;
    if (Backend::instance()->subchannels().size() > 0) {
        lSub->parentWidget()->show();
    }
}
void InWidget::removePre(QString /*channelIn*/, QString channelPre)
{
    Rotary *elem = pre[channelPre];
    lPre->removeWidget(elem);
    pre.remove(channelPre);
    if (Backend::instance()->prechannels().size() == 0) {
        lPre->parentWidget()->hide();
    }
    delete elem;
}
void InWidget::removePost(QString /*channelIn*/, QString channelPost)
{
    Rotary *elem = post[channelPost];
    lPost->removeWidget(elem);
    post.remove(channelPost);
    if (Backend::instance()->postchannels().size() == 0) {
        lPost->parentWidget()->hide();
    }
    delete elem;
}
void InWidget::removeSub(QString /*channelIn*/, QString channelSub)
{
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

    ToggleButton* mute = m_pMatrix->createToggle(PRE, channel, MUTE);
    layout->addWidget(mute);

    ToggleButton* alf = m_pMatrix->createToggle(PRE, channel, TO_ALF);
    layout->addWidget(alf);

    Widget::addSpacer(layout);

    if (Backend::instance()->getPre(channel)->stereo) {
        Rotary *bal = m_pMatrix->createRotary(PRE, channel, PAN_BAL);
        layout->addWidget(bal);
    }

    fader = m_pMatrix->createFader(PRE, channel, FADER);
    layout->addWidget(fader);
}
PreWidget::~PreWidget()
{
	m_pMatrix->removeShurtCut(PRE, m_Channel);
}
void PreWidget::mouseReleaseEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton) {
        emit clicked(PRE, m_Channel);
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

    Rotary *prevol = m_pMatrix->createRotary(POST, channel, PRE_VOL);
    layout->addWidget(prevol);

    ToggleButton* mute = m_pMatrix->createToggle(POST, channel, MUTE);
    layout->addWidget(mute);

    ToggleButton* alf = m_pMatrix->createToggle(POST, channel, TO_ALF);
    layout->addWidget(alf);
    Widget::addLine(layout, true);

    Widget::addSpacer(layout);

    layout->addWidget(new QLabel(trUtf8("Return")));
    ToggleButton* plf = m_pMatrix->createToggle(POST, channel, TO_PLF);
    layout->addWidget(plf);

    wSub = new QWidget;
    lSub = new QVBoxLayout;
    lSub->setSpacing(0);
    lSub->setMargin(0);
    wSub->setLayout(lSub);
    layout->addWidget(wSub);
    lSub->addWidget(new QLabel(m_pMatrix->getShortDisplayChannelType(SUB)));
    Widget::addLine(layout);
    lSub->parentWidget()->hide();

    Rotary *bal = m_pMatrix->createRotary(POST, channel, PAN_BAL);
    layout->addWidget(bal);

    ToggleButton* main_on = m_pMatrix->createToggle(POST, channel, TO_MAIN);
    layout->addWidget(main_on);

    fader = m_pMatrix->createFader(POST, channel, FADER);
    layout->addWidget(fader);
}
PostWidget::~PostWidget()
{
	m_pMatrix->removeShurtCut(POST, m_Channel);
}
void PostWidget::mouseReleaseEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton) {
        emit clicked(POST, m_Channel);
    }
}
void PostWidget::addSub(QString channelPost, QString channelSub)
{
    ToggleButton* elem = m_pMatrix->createToggle(POST, channelPost, TO_SUB, channelSub);
    lSub->addWidget(elem);
    sub[channelSub] = elem;
    if (Backend::instance()->subchannels().size() > 0) {
        lSub->parentWidget()->show();
    }
}
void PostWidget::removeSub(QString /*channelPost*/, QString channelSub)
{
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

    ToggleButton* mute = m_pMatrix->createToggle(SUB, channel, MUTE);
    layout->addWidget(mute);

    ToggleButton* alf = m_pMatrix->createToggle(SUB, channel, TO_ALF);
    layout->addWidget(alf);

    Widget::addSpacer(layout);

    Rotary *bal = m_pMatrix->createRotary(SUB, channel, PAN_BAL);
    layout->addWidget(bal);

    ToggleButton* main_on = m_pMatrix->createToggle(SUB, channel, TO_MAIN);
    layout->addWidget(main_on);

    fader = m_pMatrix->createFader(SUB, channel, FADER);
    layout->addWidget(fader);
}
SubWidget::~SubWidget()
{
	m_pMatrix->removeShurtCut(SUB, m_Channel);
}
void SubWidget::mouseReleaseEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton) {
        emit clicked(SUB, m_Channel);
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

    phone = m_pMatrix->createRotary(OUT, MAIN, FADER, PLF);
    layout->addWidget(phone);

    Widget::addSpacer(layout);
    layout->addWidget(new QLabel(trUtf8("Main")));

    mute = m_pMatrix->createToggle(OUT, MAIN, MUTE);
    layout->addWidget(mute);
    Widget::addLine(layout);

    mono = m_pMatrix->createRotary(OUT, MAIN, FADER, MONO);
    layout->addWidget(mono);
    
    Widget::addLine(layout);

    bal = m_pMatrix->createRotary(OUT, MAIN, PAN_BAL);
    layout->addWidget(bal);

    alf = m_pMatrix->createToggle(OUT, MAIN, TO_ALF);
    layout->addWidget(alf);

    fader = m_pMatrix->createFader(OUT, MAIN, FADER, MAIN);
    layout->addWidget(fader);
}
MainWidget::~MainWidget()
{
	m_pMatrix->removeShurtCut(OUT, MAIN);
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
        emit clicked(OUT, MAIN);
    }
}

}
; // LiveMix
