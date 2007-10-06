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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "ChannelsWidgets.h"

#include "PixmapWidget.h"

namespace LiveMix
{

void TbWrapp::clicked() {
	m_pMatrix->setVisible(!m_pMatrix->isVisible(m_eType, m_rRefChannel), m_eType, m_rRefChannel);
}
TbWrapp::TbWrapp(Widget *p_pMatrix, ToggleButton *p_pButton, ElementType p_eType, QString p_rRefChannel) 
 : m_pButton(p_pButton)
 , m_eType(p_eType)
 , m_rRefChannel(p_rRefChannel)
 , m_pMatrix(p_pMatrix)
{
	connect(p_pButton, SIGNAL(clicked()), this, SLOT(clicked()));
}

InfoWidget::InfoWidget(Widget* p_pMatrix)
        : QWidget()
        , m_pMatrix(p_pMatrix)
{
    QVBoxLayout* layout = new QVBoxLayout();
    setLayout(layout);
    setFixedWidth(23);
    layout->setSpacing(0);
    layout->setMargin(0);

	{
		QWidget *w = new QWidget();
		w->setFixedHeight(18);
    	layout->addWidget(w);
	}

    layout->addWidget(createToggleButton(m_pMatrix, GAIN));
    layout->addWidget(createLabel(26, GAIN));

    layout->addWidget(createToggleButton(m_pMatrix, MUTE));
    layout->addWidget(createLabel(16, MUTE));

    layout->addWidget(createToggleButton(m_pMatrix, TO_PFL));
    layout->addWidget(createLabel(16, TO_PFL));

    wPre = new QWidget;
    lPre = new QVBoxLayout;
    lPre->setSpacing(0);
    lPre->setMargin(0);
    wPre->setLayout(lPre);
    layout->addWidget(wPre);

	{
		PixmapWidget *pw = new PixmapWidget(NULL);
		pw->setPixmap("separator.svg");
		pw->setFixedSize(CHANNEL_WIDTH, 5);
	    layout->addWidget(pw);
	}

    wPost = new QWidget;
    lPost = new QVBoxLayout;
    lPost->setSpacing(0);
    lPost->setMargin(0);
    wPost->setLayout(lPost);
    layout->addWidget(wPost);

    wSub = new QWidget;
    lSub = new QVBoxLayout;
    lSub->setSpacing(0);
    lSub->setMargin(0);
    wSub->setLayout(lSub);
    layout->addWidget(wSub);

    layout->addWidget(createToggleButton(m_pMatrix, PAN_BAL));
    layout->addWidget(createLabel(26, PAN_BAL));

    layout->addWidget(createToggleButton(m_pMatrix, TO_MAIN));
    layout->addWidget(createLabel(16, TO_MAIN));

    Widget::addSpacer(layout);
}
InfoWidget::~InfoWidget()
{
}
ToggleButton* InfoWidget::createToggleButton(Widget* p_pMatrix, ElementType p_eType, QString p_rRefChannel) {
    ToggleButton *tb = new ToggleButton(NULL, "intro-open.svg", "intro-close.svg", "intro-open.svg", QSize(20, 5), false);
    TbWrapp *wrapp = new TbWrapp(p_pMatrix, tb, p_eType, p_rRefChannel);
    if (!m_rToggleButtons.contains(p_eType)) {
    	m_rToggleButtons.insert(p_eType, new QMap<QString, TbWrapp*>);
    }
    m_rToggleButtons[p_eType]->insert(p_rRefChannel, wrapp);
    tb->setToolTip(m_pMatrix->getDisplayFunction(IN, "", p_eType, p_rRefChannel, true));
	tb->setValue(m_pMatrix->isVisible(p_eType, p_rRefChannel));
	return tb;
}
QLabel* InfoWidget::createLabel(int p_iHeight, ElementType p_eType, QString p_rRefChannel) {
	QLabel *lab = new QLabel(m_pMatrix->getMediumDisplayFunction(IN, "", p_eType, p_rRefChannel, true));
	lab->setToolTip(m_pMatrix->getDisplayFunction(IN, "", p_eType, p_rRefChannel, true));
	lab->setWordWrap(true);
	QFont font;
	font.setPixelSize(11);
	font.setStretch(QFont::SemiCondensed);
 	lab->setFont(font);

    if (!m_rLabels.contains(p_eType)) {
	    m_rLabels.insert(p_eType, new QMap<QString, QLabel*>);
    }
    m_rLabels[p_eType]->insert(p_rRefChannel, lab);
	lab->setFixedSize(20, p_iHeight);
	if (!m_pMatrix->isVisible(p_eType, p_rRefChannel)) {
		lab->setVisible(false);
	}
	return lab;
}
void InfoWidget::setVisible(bool p_bVisible, ElementType p_eElement, QString p_rChannelTo) {
    if (m_rToggleButtons.contains(p_eElement) && m_rToggleButtons[p_eElement]->contains(p_rChannelTo)) {
    	(*m_rToggleButtons[p_eElement])[p_rChannelTo]->m_pButton->setValue(p_bVisible);
    }
    if (m_rLabels.contains(p_eElement) && m_rLabels[p_eElement]->contains(p_rChannelTo)) {
    	(*m_rLabels[p_eElement])[p_rChannelTo]->setVisible(p_bVisible);
    }
}
void InfoWidget::addPre(QString channelPre)
{
    lPre->addWidget(createToggleButton(m_pMatrix, TO_PRE, channelPre));
    lPre->addWidget(createLabel(26, TO_PRE, channelPre));
}
void InfoWidget::addPost(QString channelPost)
{
    lPost->addWidget(createToggleButton(m_pMatrix, TO_POST, channelPost));
    lPost->addWidget(createLabel(26, TO_POST, channelPost));
}
void InfoWidget::addSub(QString channelSub)
{
    lSub->addWidget(createToggleButton(m_pMatrix, TO_SUB, channelSub));
    lSub->addWidget(createLabel(16, TO_SUB, channelSub));
}
void InfoWidget::removePre(QString channelPre)
{
    if (m_rToggleButtons.contains(TO_PRE) && m_rToggleButtons[TO_PRE]->contains(channelPre)) {
	    TbWrapp *elem = (*m_rToggleButtons[TO_PRE])[channelPre];
	    lPre->removeWidget(elem->m_pButton);
	    m_rToggleButtons[TO_PRE]->remove(channelPre);
	    delete elem->m_pButton;
	    delete elem;
    }
    if (m_rLabels.contains(TO_PRE) && m_rLabels[TO_PRE]->contains(channelPre)) {
    	QWidget *elem = (*m_rLabels[TO_PRE])[channelPre];
	    lPre->removeWidget(elem);
	    m_rLabels[TO_PRE]->remove(channelPre);
	    delete elem;
    }
}
void InfoWidget::removePost(QString channelPost)
{
    if (m_rToggleButtons.contains(TO_POST) && m_rToggleButtons[TO_POST]->contains(channelPost)) {
	    TbWrapp *elem = (*m_rToggleButtons[TO_POST])[channelPost];
	    lPost->removeWidget(elem->m_pButton);
	    m_rToggleButtons[TO_POST]->remove(channelPost);
	    delete elem->m_pButton;
	    delete elem;
    }
    if (m_rLabels.contains(TO_POST) && m_rLabels[TO_POST]->contains(channelPost)) {
    	QWidget *elem = (*m_rLabels[TO_POST])[channelPost];
	    lPost->removeWidget(elem);
	    m_rLabels[TO_POST]->remove(channelPost);
	    delete elem;
    }
}
void InfoWidget::removeSub(QString channelSub)
{
    if (m_rToggleButtons.contains(TO_SUB) && m_rToggleButtons[TO_SUB]->contains(channelSub)) {
	    TbWrapp *elem = (*m_rToggleButtons[TO_SUB])[channelSub];
	    lPost->removeWidget(elem->m_pButton);
	    m_rToggleButtons[TO_SUB]->remove(channelSub);
	    delete elem->m_pButton;
	    delete elem;
    }
    if (m_rLabels.contains(TO_SUB) && m_rLabels[TO_SUB]->contains(channelSub)) {
    	QWidget *elem = (*m_rLabels[TO_SUB])[channelSub];
	    lPost->removeWidget(elem);
	    m_rLabels[TO_SUB]->remove(channelSub);
	    delete elem;
    }
}


InWidget::InWidget(QString p_sChannel, Widget* p_pMatrix)
        : QWidget()
        , m_Channel(p_sChannel)
        , m_pMatrix(p_pMatrix)
{
    QVBoxLayout* layout = new QVBoxLayout();
    setLayout(layout);
    setFixedWidth(CHANNEL_WIDTH);
    layout->setSpacing(0);
    layout->setMargin(0);

    layout->addWidget(new QLabel(p_sChannel));

	{
		PixmapWidget *pw = new PixmapWidget(NULL);
		pw->setPixmap("separator.svg");
		pw->setFixedSize(CHANNEL_WIDTH, 5);
	    layout->addWidget(pw);
	}
    Rotary *gain = m_pMatrix->createRotary(IN, p_sChannel, GAIN);
    layout->addWidget(gain);

	{
		PixmapWidget *pw = new PixmapWidget(NULL);
		pw->setPixmap("separator.svg");
		pw->setFixedSize(CHANNEL_WIDTH, 5);
	    layout->addWidget(pw);
	}
    ToggleButton* mute = m_pMatrix->createToggle(IN, p_sChannel, MUTE);
    layout->addWidget(mute);

	{
		PixmapWidget *pw = new PixmapWidget(NULL);
		pw->setPixmap("separator.svg");
		pw->setFixedSize(CHANNEL_WIDTH, 5);
	    layout->addWidget(pw);
	}
    ToggleButton* pfl = m_pMatrix->createToggle(IN, p_sChannel, TO_PFL);
    layout->addWidget(pfl);

    wPre = new QWidget;
    lPre = new QVBoxLayout;
    lPre->setSpacing(0);
    lPre->setMargin(0);
    wPre->setLayout(lPre);
    layout->addWidget(wPre);

	{
		PixmapWidget *pw = new PixmapWidget(NULL);
		pw->setPixmap("separator.svg");
		pw->setFixedSize(CHANNEL_WIDTH, 5);
	    layout->addWidget(pw);
	}

    wPost = new QWidget;
    lPost = new QVBoxLayout;
    lPost->setSpacing(0);
    lPost->setMargin(0);
    wPost->setLayout(lPost);
    layout->addWidget(wPost);

    wSub = new QWidget;
    lSub = new QVBoxLayout;
    lSub->setSpacing(0);
    lSub->setMargin(0);
    wSub->setLayout(lSub);
    layout->addWidget(wSub);

	{
		PixmapWidget *pw = new PixmapWidget(NULL);
		pw->setPixmap("separator.svg");
		pw->setFixedSize(CHANNEL_WIDTH, 5);
	    layout->addWidget(pw);
	}
    Rotary *bal = m_pMatrix->createRotary(IN, p_sChannel, PAN_BAL);
    layout->addWidget(bal);

	{
		PixmapWidget *pw = new PixmapWidget(NULL);
		pw->setPixmap("separator.svg");
		pw->setFixedSize(CHANNEL_WIDTH, 5);
	    layout->addWidget(pw);
	}
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
	{
		PixmapWidget *pw = new PixmapWidget(NULL);
		pw->setPixmap("separator.svg");
		pw->setFixedSize(CHANNEL_WIDTH, 5);
	    lPre->addWidget(pw);
	    pre_tb[channelPre] = pw;
	}
    Rotary *elem = m_pMatrix->createRotary(IN, channelIn, TO_PRE, channelPre);
    lPre->addWidget(elem);
    pre[channelPre] = elem;
}
void InWidget::addPost(QString channelIn, QString channelPost)
{
	{
		PixmapWidget *pw = new PixmapWidget(NULL);
		pw->setPixmap("separator.svg");
		pw->setFixedSize(CHANNEL_WIDTH, 5);
	    lPost->addWidget(pw);
	    post_tb[channelPost] = pw;
	}
    Rotary *elem = m_pMatrix->createRotary(IN, channelIn, TO_POST, channelPost);
    lPost->addWidget(elem);
    post[channelPost] = elem;
}
void InWidget::addSub(QString channelIn, QString channelSub)
{
	{
		PixmapWidget *pw = new PixmapWidget(NULL);
		pw->setPixmap("separator.svg");
		pw->setFixedSize(CHANNEL_WIDTH, 5);
	    lSub->addWidget(pw);
	    sub_tb[channelSub] = pw;
	}
    ToggleButton* elem = m_pMatrix->createToggle(IN, channelIn, TO_SUB, channelSub);
    elem->setToolTip(channelSub);
    lSub->addWidget(elem);
    sub[channelSub] = elem;
}
void InWidget::removePre(QString /*channelIn*/, QString channelPre)
{
	{
	    PixmapWidget *elem = pre_tb[channelPre];
	    lPre->removeWidget(elem);
	    pre_tb.remove(channelPre);
	    delete elem;
	}

    Rotary *elem = pre[channelPre];
    lPre->removeWidget(elem);
    pre.remove(channelPre);
    delete elem;
}
void InWidget::removePost(QString /*channelIn*/, QString channelPost)
{
	{
	    PixmapWidget *elem = post_tb[channelPost];
	    lPost->removeWidget(elem);
	    post_tb.remove(channelPost);
	    delete elem;
	}

    Rotary *elem = post[channelPost];
    lPost->removeWidget(elem);
    post.remove(channelPost);
    delete elem;
}
void InWidget::removeSub(QString /*channelIn*/, QString channelSub)
{
	{
	    PixmapWidget *elem = pre_tb[channelSub];
	    lSub->removeWidget(elem);
	    sub_tb.remove(channelSub);
	    delete elem;
	}

    ToggleButton *elem = sub[channelSub];
    lSub->removeWidget(elem);
    sub.remove(channelSub);
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

    ToggleButton* afl = m_pMatrix->createToggle(PRE, channel, TO_AFL);
    layout->addWidget(afl);

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

    ToggleButton* afl = m_pMatrix->createToggle(POST, channel, TO_AFL);
    layout->addWidget(afl);

    Widget::addSpacer(layout);

    layout->addWidget(new QLabel(trUtf8("Return")));
    ToggleButton* pfl = m_pMatrix->createToggle(POST, channel, TO_PFL);
    layout->addWidget(pfl);

    wSub = new QWidget;
    lSub = new QVBoxLayout;
    lSub->setSpacing(0);
    lSub->setMargin(0);
    wSub->setLayout(lSub);
    layout->addWidget(wSub);

	{
		PixmapWidget *pw = new PixmapWidget(NULL);
		pw->setPixmap("separator.svg");
		pw->setFixedSize(CHANNEL_WIDTH, 5);
	    layout->addWidget(pw);
	}
    Rotary *bal = m_pMatrix->createRotary(POST, channel, PAN_BAL);
    layout->addWidget(bal);

	{
		PixmapWidget *pw = new PixmapWidget(NULL);
		pw->setPixmap("separator.svg");
		pw->setFixedSize(CHANNEL_WIDTH, 5);
	    layout->addWidget(pw);
	}
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
	{
		PixmapWidget *pw = new PixmapWidget(NULL);
		pw->setPixmap("separator.svg");
		pw->setFixedSize(CHANNEL_WIDTH, 5);
	    lSub->addWidget(pw);
	    sub_tb[channelSub] = pw;
	}
    ToggleButton* elem = m_pMatrix->createToggle(POST, channelPost, TO_SUB, channelSub);
    lSub->addWidget(elem);
    sub[channelSub] = elem;
}
void PostWidget::removeSub(QString /*channelPost*/, QString channelSub)
{
	{
	    PixmapWidget *elem = sub_tb[channelSub];
	    lSub->removeWidget(elem);
	    sub_tb.remove(channelSub);
	    delete elem;
	}

    ToggleButton *elem = sub[channelSub];
    lSub->removeWidget(elem);
    sub.remove(channelSub);
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
    layout->addWidget(m_pMatrix->createToggle(SUB, channel, MUTE));
    layout->addWidget(m_pMatrix->createToggle(SUB, channel, TO_AFL));
    Widget::addSpacer(layout);
    layout->addWidget(m_pMatrix->createRotary(SUB, channel, PAN_BAL));
    layout->addWidget(m_pMatrix->createToggle(SUB, channel, TO_MAIN));

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

    phone = m_pMatrix->createRotary(OUT, MAIN, FADER, PFL);
    layout->addWidget(phone);

    Widget::addSpacer(layout);
    layout->addWidget(new QLabel(trUtf8("Main")));

    mute = m_pMatrix->createToggle(OUT, MAIN, MUTE);
    layout->addWidget(mute);
//    Widget::addLine(layout);
	{
		PixmapWidget *pw = new PixmapWidget(NULL);
		pw->setPixmap("separator.svg");
		pw->setFixedSize(CHANNEL_WIDTH, 5);
	    layout->addWidget(pw);
	}

    mono = m_pMatrix->createRotary(OUT, MAIN, FADER, MONO);
    layout->addWidget(mono);
    
//    Widget::addLine(layout);
	{
		PixmapWidget *pw = new PixmapWidget(NULL);
		pw->setPixmap("separator.svg");
		pw->setFixedSize(CHANNEL_WIDTH, 5);
	    layout->addWidget(pw);
	}

    bal = m_pMatrix->createRotary(OUT, MAIN, PAN_BAL);
    layout->addWidget(bal);

    afl = m_pMatrix->createToggle(OUT, MAIN, TO_AFL);
    layout->addWidget(afl);

    fader = m_pMatrix->createFader(OUT, MAIN, FADER, MAIN);
    layout->addWidget(fader);
}
MainWidget::~MainWidget()
{
	m_pMatrix->removeShurtCut(OUT, MAIN);
}
void MainWidget::update()
{
    phone->setDbValue(Backend::instance()->getOutput(PFL)->volume);
    mute->setValue(Backend::instance()->getOutput(MAIN)->mute);
    mono->setDbValue(Backend::instance()->getOutput(MONO)->volume);
    bal->setValue(Backend::instance()->getOutput(MAIN)->bal);
    afl->setValue(Backend::instance()->getOutput(MAIN)->afl);
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
