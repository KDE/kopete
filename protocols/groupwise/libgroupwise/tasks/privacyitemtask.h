//
// C++ Interface: createprivacyitemtask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PRIVACYITEMTASK_H
#define PRIVACYITEMTASK_H

#include "requesttask.h"

/**
Adds a contact to the server side allow or deny lists

@author SUSE AG
*/
class PrivacyItemTask : public RequestTask
{
Q_OBJECT
public:
	PrivacyItemTask( Task* parent);
	~PrivacyItemTask();
	void allow( const QString & dn );
	void deny( const QString & dn );
	void removeAllow( const QString & dn );
	void removeDeny( const QString & dn );
	void defaultPolicy( bool defaultDeny );
	QString dn() const;
	bool defaultDeny() const;
	// void contacts( const QStringList & contacts );
private:
	bool m_default;
	QString m_dn;
};

#endif
