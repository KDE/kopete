//
// C++ Interface: joinconferencetask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef JOINCONFERENCETASK_H
#define JOINCONFERENCETASK_H

#include "requesttask.h"

/**
Sends Join Conference messages when the user accepts an invitation

@author SUSE AG
*/
class JoinConferenceTask : public RequestTask
{
Q_OBJECT
public:
	JoinConferenceTask(Task* parent);
	~JoinConferenceTask();
	void join( const QString & guid );
	bool take( Transfer * transfer );
	QStringList participants() const;
	QString guid() const;
public slots:
	void slotReceiveUserDetails( const GroupWise::ContactDetails & details );
signals:
	void temporaryContact( const GroupWise::ContactDetails & );
private:
	QString m_guid;
	QStringList m_participants;
	QStringList m_unknowns;
};

#endif
