/*
    kopetestatusgroupaction.h - Kopete Status Group Action

    Copyright (c) 2008      by Roman Jarosz          <kedgedev@centrum.cz>
    Kopete    (c) 2008      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#ifndef KOPETESTATUSGROUPACTION_H
#define KOPETESTATUSGROUPACTION_H

#include <KActionMenu>
#include "kopete_export.h"
namespace Kopete
{
	class StatusRootAction;

	namespace Status
	{
		class StatusGroup;
		class StatusItem;
		class Status;
	}

/**
	@author Roman Jarosz <kedgedev@centrum.cz>
*/

class KOPETE_STATUSMENU_EXPORT StatusGroupAction : public KActionMenu
{
	Q_OBJECT
public:
	/**
	 * StatusGroupAction constructor
	 * @param group the corresponding StatusGroup object
	 * @param rootAction the StatusRootAction object this status status belongs to
	 * @param parent the parent object
	 **/
	StatusGroupAction( Status::StatusGroup *group, StatusRootAction* rootAction, QObject * parent );

	/**
	 * Returns number of StatusActions and StatusGroupActions for this menu
	 **/
	int childCount() const;

private:
	void init();

private Q_SLOTS:
	void changed();

	void childInserted( int i, Kopete::Status::StatusItem* child );
	void childRemoved( Kopete::Status::StatusItem* child );

private:
	void insertChild( QAction * before, Status::StatusItem* child );
		
	Status::StatusGroup *mStatusGroup;
	StatusRootAction* mRootAction;
	QMap<Status::StatusItem *, QAction* > mChildMap;
};

}

#endif
