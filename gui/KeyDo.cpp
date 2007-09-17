#include "KeyDo.h"


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

KeyDoChannelAction::KeyDoChannelAction(Widget* p_pMatrix, Backend::ElementType p_eElement, 
		QString p_sReatedChannelName, QString p_sDisplayReatedChannelName)
        : KeyDo(p_pMatrix)
        , m_eElement(p_eElement)
        , m_sReatedChannelName(p_sReatedChannelName)
        , m_sDisplayReatedChannelName(p_sDisplayReatedChannelName)
{
}
KeyDoChannelAction::~KeyDoChannelAction()
{}
void KeyDoChannelAction::action()
{
    m_pMatrix->action(m_pMatrix->getSelectedChanelType(), m_pMatrix->getSetectedChannelName(), m_eElement, m_sReatedChannelName, m_sDisplayReatedChannelName);
}

KeyDoDirectAction::KeyDoDirectAction(Widget* p_pMatrix, Backend::ChannelType p_eType, QString p_sChannelName, Backend::ElementType p_eElement, 
		QString p_sReatedChannelName, QString p_sDisplayReatedChannelName)
        : KeyDo(p_pMatrix)
        , m_eType(p_eType)
        , m_sChannelName(p_sChannelName)
        , m_eElement(p_eElement)
        , m_sReatedChannelName(p_sReatedChannelName)
        , m_sDisplayReatedChannelName(p_sDisplayReatedChannelName)
{
}
KeyDoDirectAction::~KeyDoDirectAction()
{}
void KeyDoDirectAction::action()
{
    m_pMatrix->action(m_eType, m_sChannelName, m_eElement, m_sReatedChannelName, m_sDisplayReatedChannelName);
}

