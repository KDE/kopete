/*
   setpersonalinformationtask.h - Set personal information for myself contact.

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#ifndef PAPILLONSETPERSONALINFORMATIONTASK_H
#define PAPILLONSETPERSONALINFORMATIONTASK_H

#include <Papillon/Macros>
#include <Papillon/Enums>
#include <Papillon/Task>

namespace Papillon
{

/**
 * @class SetPersonalInformationTask setpersonalinformationtask.h <Papillon/Tasks/SetPersonalInformationTask>
 * @brief Set personal information for myself contact on server.
 * Personal information such as nickname(my friendly name), home phone number,
 * work phone number, mobile phone number, authorization to contact on MSN Mobile,
 * and if a mobile device is enabled.
 *
 * Note that you can only set/change one information at time.
 *
 * Usage:
 * @code
 * SetPersonalInformationTask *setInfo = new SetPersonalInformationTask( connection()->rootTask() );
 * setInfo->setPersonalInformation( Papillon::ClientInfo::Nickname, QString("Libpapillon Test Buddy") );
 * setInfo->go(Task::AutoDelete);
 * @endcode
 *
 * @author Michaël Larouche <larouche@kde.org>
 */
class PAPILLON_EXPORT SetPersonalInformationTask : public Papillon::Task
{
	Q_OBJECT
public:
	/**
	 * @brief Create a new SetPersonalInformationTask task.
	 * @param parent Root Task
	 */
	explicit SetPersonalInformationTask(Papillon::Task *parent);
	/**
	 * d-tor
	 */
	~SetPersonalInformationTask();

	/**
	 * @brief Set the personal information to be updated on server.
	 * @param type The type of the personal information it need to update on server.
	 * @param value New value for the given personal information. Set an empty string to reset the value.
	 */
	void setPersonalInformation(Papillon::ClientInfo::PersonalInformation type, const QString &value);

	/**
	 * @brief Check for task acknowledge if the task executed successfully.
	 * @param transfer the Transfer
	 * @return true if this is the task acknowledge
	 */
	virtual bool take(Papillon::Transfer *transfer);

protected:
	/**
	 * @brief Send the command to set personal information on server.
	 */
	virtual void onGo();

private:
	class Private;
	Private *d;
};

}
#endif
