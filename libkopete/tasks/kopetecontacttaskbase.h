/*
    kopetecontacttaskbase.h - Base task for all contact tasks

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
#ifndef KOPETE_CONTACTTASKBASE_H
#define KOPETE_CONTACTTASKBASE_H

#include <kopetetask.h>
#include <kopete_export.h>

namespace Kopete
{

class Contact;
/**
 * @brief Base tasks for all contact related tasks
 *
 * This class contains all the common code and data
 * for all contact tasks.
 *
 * Derived must implement tastType() to allow setContact()
 * to automatically add the correct subtask from the protocol.
 *
 * @author Michaël Larouche <larouche@kde.org>
 */
class KOPETE_EXPORT ContactTaskBase : public Kopete::Task
{
	Q_OBJECT
public:
	/**
	 * @brief Default constructor
	 *
	 / You must set the contact with setContact()
	 */
	explicit ContactTaskBase(QObject *parent = 0);
	/**
	 * destructor
	 */
	virtual ~ContactTaskBase();

	/**
	 * @brief Set the contact
	 *
	 * This method also add children tasks for
	 * the specific protocol. You don't need
	 * to do it yourself.
	 */
	void setContact(Kopete::Contact *contact);

protected:
	/**
	 * @brief Get access to Kopete::Contact
	 * @return a Kopete::Contact pointer.
	 */
	Kopete::Contact *contact();

	/**
	 * @brief Get the taskType
	 *
	 * This is the type of task from the devired class.
	 * Used by setContact()
	 */
	virtual QString taskType() const = 0;

private:
	class Private;
	Private * const d;
};

}
#endif
