#ifndef ASSIGNETOPANNEL_H_
#define ASSIGNETOPANNEL_H_

#import "GetKeyField.h"

#import <QDialog>

namespace LiveMix
{

class AssigneToPannel : public QDialog
{
    Q_OBJECT
public:
	AssigneToPannel(QString p_sChannel, QString p_sFunction, bool p_bVolume, QKeySequence p_rActionOnChannelKeySequence
			, QKeySequence p_rSelectChannelKeySequence, QKeySequence p_rActionOnSelectedChannelKeySequence);
	virtual ~AssigneToPannel();
	
	QKeySequence getActionOnChannelKeySequence();
	QKeySequence getSelectChannelKeySequence();
	QKeySequence getActionOnSelectedChannelKeySequence();
	
public slots:
	void okClicked(bool p_bChecked);
	void cancelClicked(bool p_bChecked);
	
private:
	GetKeyField* m_pActionOnChannel;
	GetKeyField* m_pSelectChannel;
	GetKeyField* m_pActionOnSelectedChannel;
};

}
; // LiveMix

#endif /*ASSIGNETOPANNEL_H_*/
