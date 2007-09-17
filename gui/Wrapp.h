#ifndef WRAPP_H_
#define WRAPP_H_

class Wrapp : public QObject
{
    Q_OBJECT
public:
    Wrapp(Widget* p_pMatrix, Action* p_pWidget, Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, QString p_sReatedChannelName, QString p_sDisplayReatedChannelName);

    virtual bool exec();

public slots:
    void leftClick(QMouseEvent* p_ev);
    void middleClick(QMouseEvent* p_ev);
    void rightClick(QMouseEvent* p_ev);

protected:
    Widget* m_pMatrix;

    Backend::ChannelType m_eType;
    QString m_sChannelName;
    Backend::ElementType m_eElement;
    QString m_sReatedChannelName;
	QString m_sDisplayChannelName;
	QString m_sDisplayReatedChannelName;
};

class WrappVolume : public Wrapp
{
    Q_OBJECT
public:
    WrappVolume(Widget* p_pMatrix, Volume* p_pWidget, Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, QString p_sReatedChannelName, QString p_sDisplayReatedChannelName);
    Volume* getVolume();
private:
    Volume* m_pWidget;
private slots:
	void displayValueChanged(QString p_sValue); 
};

class WrappToggle : public Wrapp
{
    Q_OBJECT
public:
    WrappToggle(Widget* p_pMatrix, ToggleButton* p_pWidget, Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, QString p_sReatedChannelName, QString p_sDisplayReatedChannelName);

    bool exec();
private:
    ToggleButton* m_pWidget;
};


#endif /*WRAPP_H_*/
