#include "Wrapp.h"


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
