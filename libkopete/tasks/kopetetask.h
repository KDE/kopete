/*
    kopetetask.h - Kopete Task

    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>
    Copyright (c) 2007      by Michaël Larouche      <larouche@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETETASK_H
#define KOPETETASK_H

#include <kopete_export.h>
#include <kcompositejob.h>

namespace Kopete
{

/**
 * @brief Base class for all Kopete task.
 *
 * Use the Command/Job/Task pattern to encapsulate a job.
 *
 * @author Michaël Larouche <larouche@kde.org>
 */
class KOPETE_EXPORT Task : public KCompositeJob
{
	Q_OBJECT
public:
	Task(QObject *parent = 0);
	virtual ~Task();

private:
	class Private;
	Private *d;
};

}

#endif
