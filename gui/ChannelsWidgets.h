#ifndef CHANNELSWIDGETS_H_
#define CHANNELSWIDGETS_H_

class InWidget : public QWidget
{
    Q_OBJECT
public:
    InWidget(QString p_sChannel, Widget* p_pMatrix);
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
    Widget* m_pMatrix;

    QWidget* wPre;
    QWidget* wPost;
    QWidget* wSub;

    QVBoxLayout* lPre;
    QVBoxLayout* lPost;
    QVBoxLayout* lSub;

    QMap<QString, Rotary*> pre;
    QMap<QString, Rotary*> post;
    QMap<QString, ToggleButton*> sub;

// QMap<QWidget*, Wrapp*> m_mWrapps;
};

class PreWidget : public QWidget
{
    Q_OBJECT
public:
    PreWidget(QString channel, Widget* p_pMatrix);
    ~PreWidget();

    Fader* fader;

    void mouseReleaseEvent(QMouseEvent* ev);

signals:
    void clicked(Backend::ChannelType, QString channel);

private:
    QString m_Channel;
    Widget* m_pMatrix;

// QMap<QWidget*, Wrapp*> m_mWrapps;
};

class PostWidget : public QWidget
{
    Q_OBJECT
public:
    PostWidget(QString channel, Widget* p_pMatrix);
    ~PostWidget();

    void addSub(QString channelPost, QString channelSub);
    void removeSub(QString channelIn, QString channelSub);

    Fader* fader;

    void mouseReleaseEvent(QMouseEvent* ev);

signals:
    void clicked(Backend::ChannelType, QString channel);

private:
    QString m_Channel;
    Widget* m_pMatrix;

    QWidget* wSub;
    QVBoxLayout* lSub;
    QMap<QString, ToggleButton*> sub;

// QMap<QWidget*, Wrapp*> m_mWrapps;
};

class SubWidget : public QWidget
{
    Q_OBJECT
public:
    SubWidget(QString channel, Widget* p_pMatrix);
    ~SubWidget();

    Fader* fader;

    void mouseReleaseEvent(QMouseEvent* ev);

signals:
    void clicked(Backend::ChannelType, QString channel);

private:
    QString m_Channel;
    Widget* m_pMatrix;

// QMap<QWidget*, Wrapp*> wrapps;
};

class MainWidget : public QWidget
{
    Q_OBJECT
public:
    MainWidget(Widget* p_pMatrix);
    ~MainWidget();

    void update();

    void mouseReleaseEvent(QMouseEvent* ev);

    Fader* fader;

signals:
    void clicked(Backend::ChannelType, QString channel);

private:
    Widget* m_pMatrix;

    Rotary *phone;
    ToggleButton* mute;
    Rotary *mono;
    Rotary *bal;
    ToggleButton* alf;
// QMap<QWidget*, Wrapp*> m_mWrapps;
};

#endif /*CHANNELSWIDGETS_H_*/
