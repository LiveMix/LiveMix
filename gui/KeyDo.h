#ifndef KEYDO_H_
#define KEYDO_H_


class KeyDo
{
public:
    virtual ~KeyDo();
    virtual void action() =0;
    virtual QString name() =0;

protected:
    KeyDo(Widget* p_pMatrix);
    Widget* m_pMatrix;
};
class KeyDoSelectChannel : public KeyDo
{
public:
    KeyDoSelectChannel(Widget* p_pMatrix, Backend::ChannelType p_eType, QString p_sChannelName);
    ~KeyDoSelectChannel();
    void action();
    QString name() {return "KeyDoSelectChannel";};

    Backend::ChannelType m_eType;
    QString m_sChannelName;
};
class KeyDoChannelAction : public KeyDo
{
public:
    KeyDoChannelAction(Widget* p_pMatrix, Backend::ElementType p_eElement, QString p_sReatedChannelName, QString p_sDisplayReatedChannelName);
    ~KeyDoChannelAction();
    void action();
    QString name() {return "KeyDoChannelAction";};

    Backend::ElementType m_eElement;
    QString m_sReatedChannelName;
    QString m_sDisplayReatedChannelName;
};
class KeyDoDirectAction : public KeyDo
{
public:
    KeyDoDirectAction(Widget* p_pMatrix, Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, 
    		QString p_sReatedChannelName, QString p_sDisplayReatedChannelName);
    ~KeyDoDirectAction();
    void action();
    QString name() {return "KeyDoDirectAction " + m_sChannelName + " " + m_sReatedChannelName + " " + m_pMatrix->getDisplayElement(m_eElement);};

    Backend::ChannelType m_eType;
    QString m_sChannelName;
    Backend::ElementType m_eElement;
    QString m_sReatedChannelName;
    QString m_sDisplayReatedChannelName;
};

#endif /*KEYDO_H_*/
