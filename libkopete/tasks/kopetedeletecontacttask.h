/*
    kopetedeletecontacttask.h - Kopete Delete Contact Task

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
#ifndef KOPETE_DELETECONTACTTASK_H
#define KOPETE_DELETECONTACTTASK_H

#include <kopete_export.h>
#include <kopetecontacttaskbase.h>

namespace Kopete
{

class Contact;
/**
 * @brief Delete a contact in Kopete
 *
 * Example code:
 * @code
Kopete::DeleteContactTask *deleteTask = new Kopete::DeleteContactTask(aContact);
deleteTask->addSubTask( new JabberDeleteContactTask(aContact) );
connect(deleteTask, SIGNAL(result(KJob*)), receiver, SLOT(slotResult(KJob*)));
deleteTask->start();
 * @endcode
 *
 * @section protocol_delete Implementing protocol subtask for deleting
 * It is a good idea to inherit from DeleteContactTask. In your implementation
 * of start() method, please DO NOT call parent start() method from DeleteContactTask.
 *
 * DeleteContactTask will delete the contact after the subjob,
 * so you don't need to explicit call deleteLater() on contact.
 *
 * Also, you don't need to check if the network(or account if you prefer)
 * is available, DeleteContactTask do it for you.
 *
 * The name of this task is "DeleteContactTask".
 *
 * @author Michaël Larouche <larouche@kde.org>
 */
class KOPETE_EXPORT DeleteContactTask : public Kopete::ContactTaskBase
{
	Q_OBJECT
public:
	/**
	 * @brief Default constructor
	 *
	 * You must set the contact to delete with setContact()
	 */
	DeleteContactTask(QObject *parent = 0); //implicit
	
	/**
	 * @brief Delete the given contact
	 * @param contact Kopete contact to delete
	 */
	explicit DeleteContactTask(Kopete::Contact *contact);

	/**
	 * @internal
	 * Destructor
	 */
	~DeleteContactTask();

	/**
	 * @brief Begin the task.
	 * Inherited from Kopete::Task::start()
	 */
	virtual void start();

protected Q_SLOTS:
	/**
	 * @brief Execute the next sub job
	 *
	 * This slot is called when a subjob has finished.
	 * @param subJob sub job that has been finished.
	 */
	virtual void slotResult(KJob *subJob);

protected:
	/**
	 * @brief Return Task Type
	 * @return DeleteContactTask
	 */
	QString taskType() const;
private:
	class Private;
	Private * const d;
};

}

#endif
