/*
    kopetestatusaction.h - Kopete Status Action

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
#ifndef KOPETESTATUSACTION_H
#define KOPETESTATUSACTION_H

#include <KAction>

namespace Kopete
{
	class StatusRootAction;
	namespace Status
	{
		class Status;
	}

/**
	@author Roman Jarosz <kedgedev@centrum.cz>
*/
class StatusAction : public KAction
{
	Q_OBJECT
public:
	/**
	 * StatusAction constructor
	 * @param status the corresponding Status object
	 * @param rootAction the StatusRootAction object this status status belongs to
	 * @param parent the parent object
	 **/
	StatusAction( Status::Status *status, StatusRootAction* rootAction, QObject * parent );

private:
	void init();

private Q_SLOTS:
	void triggered();
	void changed();

private:
	Status::Status *mStatus;
	StatusRootAction* mRootAction;
};

}

#endif
