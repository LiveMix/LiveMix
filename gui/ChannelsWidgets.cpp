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

QLabel* InfoWidget::label(QString p_rName) {
	QLabel *l = new QLabel(p_rName);
	l->setToolTip(p_rName);
	l->setWordWrap(true);
	QFont font;
	font.setPixelSize(11);
	font.setStretch(QFont::SemiCondensed);
 	l->setFont(font);
 	return l; 
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

    layout->addWidget(createToggleButton(GAIN));
    layout->addWidget(26, createLabel(GAIN));

    layout->addWidget(createToggleButton(GAIN));
    layout->addWidget(26, createLabel(GAIN));

	{
	    mute_tb = new ToggleButton(NULL, "intro-open.svg", "intro-close.svg", "intro-open.svg", QSize(20, 5), false);
//	    gain_tb->setToolTip(m_pMatrix->getDisplayFunction(IN, "", GAIN, "", true));
//		mute_tb->setValue(true);
//		connect(gain_tb, SIGNAL(clicked()), m_pMatrix, SLOT(showGain()));
	    layout->addWidget(mute_tb);
	}
	mute = label(m_pMatrix->getMediumDisplayFunction(IN, "", MUTE, "", true));
	mute->setFixedSize(20, 16);
    layout->addWidget(mute);

	{
	    plf_tb = new ToggleButton(NULL, "intro-open.svg", "intro-close.svg", "intro-open.svg", QSize(20, 5), false);
//	    gain_tb->setToolTip(m_pMatrix->getDisplayFunction(IN, "", GAIN, "", true));
//		plf_tb->setValue(m_pMatrix->isGainVisible());
//		connect(plf_tb, SIGNAL(clicked()), m_pMatrix, SLOT(showGain()));
	    layout->addWidget(plf_tb);
	}
	plf = label(m_pMatrix->getMediumDisplayFunction(IN, "", TO_PLF, "", true));
	plf->setFixedSize(20, 16);
    layout->addWidget(plf);


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
	    bal_tb = new ToggleButton(NULL, "intro-open.svg", "intro-close.svg", "intro-open.svg", QSize(20, 5), false);
//	    gain_tb->setToolTip(m_pMatrix->getDisplayFunction(IN, "", GAIN, "", true));
//		bal_tb->setValue(m_pMatrix->isGainVisible());
//		connect(bal_tb, SIGNAL(clicked()), m_pMatrix, SLOT(showGain()));
	    layout->addWidget(bal_tb);
	}
	bal = label(m_pMatrix->getMediumDisplayFunction(IN, "", PAN_BAL, "", true));
	bal->setFixedSize(20, 26);
    layout->addWidget(bal);

	{
	    main_on_tb = new ToggleButton(NULL, "intro-open.svg", "intro-close.svg", "intro-open.svg", QSize(20, 5), false);
//	    gain_tb->setToolTip(m_pMatrix->getDisplayFunction(IN, "", GAIN, "", true));
//		bal_tb->setValue(m_pMatrix->isGainVisible());
//		connect(bal_tb, SIGNAL(clicked()), m_pMatrix, SLOT(showGain()));
	    layout->addWidget(bal_tb);
	}
	main_on = label(m_pMatrix->getMediumDisplayFunction(IN, "", TO_MAIN, "", true));
	main_on->setFixedSize(20, 16);
    layout->addWidget(main_on);

    Widget::addSpacer(layout);
}
InfoWidget::~InfoWidget()
{
}
void InfoWidget::createToggleButton(ElementType p_eType, String p_rRefChannel) {
    ToggleButton tb = new ToggleButton(NULL, "intro-open.svg", "intro-close.svg", "intro-open.svg", QSize(20, 5), false);
    if (!m_rToggleButtons.contains(p_eType)) {
    	m_rToggleButtons.insert(p_eType, new QMap<QString, ToggleButton>);
    }
    m_rToggleButtons[p_eType]->insert(p_rRefChannel, tb);
    tb->setToolTip(m_pMatrix->getDisplayFunction(IN, "", p_eType, p_rRefChannel, true));
	tb->setValue(m_pMatrix->isVisible(p_eType, p_rRefChannel));
//	connect(gain_tb, SIGNAL(clicked()), m_pMatrix, SLOT(showGain())); // TODO wrapp
}
void InfoWidget::createLabel(int p_iHeight, ElementType p_eType, String p_rRefChannel) {
	QLabel gain = label(m_pMatrix->getMediumDisplayFunction(IN, "", p_eType, p_rRefChannel, true));
    if (!m_rLabel.contains(p_eType)) {
	    m_rLabel.insert(p_eType, new QMap<QString, ToggleButton>);
    }
    m_rLabel[p_eType]->insert(p_rRefChannel, tb);
	gain->setFixedSize(20, p_iHeight);
	gain->setVisible(m_pMatrix->isVisible(p_eType, p_rRefChannel));
}
void InfoWidget::setVisible(bool p_bVisible, ElementType p_eElement, QString p_rChannelTo) {
	info_widget->gain->setVisible(p_bVisible);
	info_widget->gain_tb->setValue(p_bVisible);
}
void InfoWidget::addPre(QString channelPre)
{
	{
	    ToggleButton *tb = new ToggleButton(NULL, "intro-open.svg", "intro-close.svg", "intro-open.svg", QSize(20, 5), false);
//	    gain_tb->setToolTip(m_pMatrix->getDisplayFunction(IN, "", GAIN, "", true));
//		tb->setValue(m_pMatrix->isGainVisible());
//		connect(tb, SIGNAL(clicked()), m_pMatrix, SLOT(showGain()));
	    lPre->addWidget(tb);
	    pre_tb[channelPre] = tb;
	}

	QLabel *elem = label(channelPre);
	elem->setToolTip(m_pMatrix->getDisplayFunction(IN, "", TO_PRE, channelPre, true));
	elem->setFixedSize(20, 26);
    lPre->addWidget(elem);
    pre[channelPre] = elem;
}
void InfoWidget::addPost(QString channelPost)
{
	{
	    ToggleButton *tb = new ToggleButton(NULL, "intro-open.svg", "intro-close.svg", "intro-open.svg", QSize(20, 5), false);
//	    gain_tb->setToolTip(m_pMatrix->getDisplayFunction(IN, "", GAIN, "", true));
//		tb->setValue(m_pMatrix->isGainVisible());
//		connect(tb, SIGNAL(clicked()), m_pMatrix, SLOT(showGain()));
	    lPost->addWidget(tb);
	    post_tb[channelPost] = tb;
	}

	QLabel *elem = label(channelPost);
	elem->setToolTip(m_pMatrix->getDisplayFunction(IN, "", TO_POST, channelPost, true));
	elem->setFixedSize(20, 26);
    lPost->addWidget(elem);
    post[channelPost] = elem;
}
void InfoWidget::addSub(QString channelSub)
{
	{
	    ToggleButton *tb = new ToggleButton(NULL, "intro-open.svg", "intro-close.svg", "intro-open.svg", QSize(20, 5), false);
//	    gain_tb->setToolTip(m_pMatrix->getDisplayFunction(IN, "", GAIN, "", true));
//		tb->setValue(m_pMatrix->isGainVisible());
//		connect(tb, SIGNAL(clicked()), m_pMatrix, SLOT(showGain()));
	    lSub->addWidget(tb);
	    sub_tb[channelSub] = tb;
	}

	QLabel *elem = label(channelSub);
	elem->setToolTip(m_pMatrix->getDisplayFunction(IN, "", TO_SUB, channelSub, true));
	elem->setFixedSize(20, 16);
    elem->setToolTip(channelSub);
    lSub->addWidget(elem);
    sub[channelSub] = elem;
}
void InfoWidget::removePre(QString channelPre)
{
	{
	    ToggleButton *elem = pre_tb[channelPre];
	    lPre->removeWidget(elem);
	    pre_tb.remove(channelPre);
	    delete elem;
	}

    QWidget *elem = pre[channelPre];
    lPre->removeWidget(elem);
    pre.remove(channelPre);
    delete elem;
}
void InfoWidget::removePost(QString channelPost)
{
	{
	    ToggleButton *elem = post_tb[channelPost];
	    lPost->removeWidget(elem);
	    post_tb.remove(channelPost);
	    delete elem;
	}

    QWidget *elem = post[channelPost];
    lPost->removeWidget(elem);
    post.remove(channelPost);
    delete elem;
}
void InfoWidget::removeSub(QString channelSub)
{
	{
	    ToggleButton *elem = pre_tb[channelSub];
	    lSub->removeWidget(elem);
	    sub_tb.remove(channelSub);
	    delete elem;
	}

    QWidget *elem = sub[channelSub];
    lSub->removeWidget(elem);
    sub.remove(channelSub);
    delete elem;
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
	gain->setVisible(m_pMatrix->isGainVisible());
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
    ToggleButton* plf = m_pMatrix->createToggle(IN, p_sChannel, TO_PLF);
    layout->addWidget(plf);


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
//    Widget::addLine(layout);

    mono = m_pMatrix->createRotary(OUT, MAIN, FADER, MONO);
    layout->addWidget(mono);
    
//    Widget::addLine(layout);

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
