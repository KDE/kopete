//
// C++ Interface: createconferencetask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CREATECONFERENCETASK_H
#define CREATECONFERENCETASK_H

#include <requesttask.h>

/**
This task is responsible for creating a conference at the server, and confirming that the server allowed the conference to be created.

@author SUSE AG
*/
class CreateConferenceTask : public RequestTask
{
Q_OBJECT
public:
	CreateConferenceTask(Task* parent);
	~CreateConferenceTask();
	/**
	 * Set up a create conference request
	 * @param confId The client-unique conference Id.
	 * @param participants A list of Novell DNs of the people taking part in the conference.
	 */
	void conference( const int confId, const QStringList &participants );
	void onGo();
	bool take( Transfer * transfer );
	int  conferenceId();
	QString conferenceGUID();
	
signals:
	void created( const QString & guid );
private: 
	int m_confId; // the conference id given us before making the request
	QString m_guid;
};

#endif
