/*
   challengetask.h - Answer to challenge string given by Notification Server.

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   Portions used from Kopete with Gregg's approval on LGPL license:
   Copyright (c) 2005 by Gregg Edghill       <gregg.edghill@gmail.com>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#ifndef PAPILLONCHALLENGETASK_H
#define PAPILLONCHALLENGETASK_H

#include <Papillon/Macros>
#include <Papillon/Task>

namespace Papillon
{
/**
 * @class ChallengeTask challengetask.h <Papillon/Tasks/ChallengeTask>
 * @author Michaël Larouche <larouche@kde.org>
 */
class PAPILLON_EXPORT ChallengeTask : public Papillon::Task
{
	Q_OBJECT
public:
	explicit ChallengeTask(Papillon::Task *parent);
	~ChallengeTask();

	virtual bool take(Papillon::Transfer *transfer);

private:
	/**
	 * @brief Compute the challenge hash to be sended on server.
	 * @param challenge Challenge string receive on server.
	 */
	// Defined here to be unittested.
	QString createChallengeHash(const QString &challengeString);

private:
	class Private;
	Private *d;
};

}
#endif
