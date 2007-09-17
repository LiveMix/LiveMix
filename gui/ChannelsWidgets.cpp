#include "ChannelsWidgets.h"


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