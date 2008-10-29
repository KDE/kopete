/*
    Kopete Groupwise Protocol
    movecontacttask.h - Move a contact between folders on the server

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef MOVECONTACTTASK_H
#define MOVECONTACTTASK_H

#include "libgroupwise_export.h"
#include "needfoldertask.h"

/**
Moves a contact between folders on the server

@author SUSE AG
*/
class LIBGROUPWISE_EXPORT MoveContactTask : public NeedFolderTask
{
Q_OBJECT
public:
	MoveContactTask(Task* parent);
	~MoveContactTask();
	void moveContact( const ContactItem & contact, const int newParent );
	void moveContactToNewFolder( const ContactItem & contact, const int newSequenceNumber, const QString & folderDisplayName );
	void onGo();
protected:
	void onFolderCreated();
private:
	int m_targetFolder;
	QString m_dn;
	QString m_displayName;
	ContactItem m_contactToMove;	
};

#endif
