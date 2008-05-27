/*
    kopetestatusmessage.h - Describle a status message and it's metadata.

    Copyright (c) 2006  by Michaël Larouche          <larouche@kde.org>

    Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#ifndef KOPETESTATUSMESSAGE_H
#define KOPETESTATUSMESSAGE_H

#include <QtCore/QVariant>

#include <ksharedptr.h>
#include "kopete_export.h"

namespace Kopete
{

/**
 * @brief This class encapsulate a status message.
 * A status message to today(as 2006) standards is more than a simple text. 
 * It can be used to show the current listening song, current playing game, show our mood etc..
 * So this class allows to add metadata to the status message where protocols will be able to use properly.
 * 
 * @code
 * // Create a new status message.
 * Kopete::StatusMessage message;
 * message.setMessage( QString("Writing APIDOX") );
 * message.addMetaData( "musicPlayer", "amaroK" );
 * message.addMetaData( "artist", "Liquid Tension Experiment" );
 * message.addMetaData( "title", "Acid Rain" );
 * message.addMetaData( "album", "Liquid Tension Experiment 2" );
 * 
 * account->setStatusMessage(message);
 * @endcode
 * This class is implicit shared.
 * @author Michaël Larouche
 */
class KOPETE_EXPORT StatusMessage
{
public:
	/**
	 * Create a empty status message.
	 */
	StatusMessage();
	/**
	 * Create a new status message with the specified status message.
	 * This constructor is not explicit so it's allow implicit QString 
	 * conversation to this class.
	 * @param statusMessage the status message.
	 */
	StatusMessage(const QString &statusMessage); /* implicit */
	/**
	 * Create a new status message with the specified status message and title.
	 * @param statusTitle the status title.
	 * @param statusMessage the status message.
	 */
	StatusMessage(const QString &statusTitle, const QString &statusMessage);
	/**
	 * StatusMessage copy constructor. 
	 * Very cheap because the class is implicit shared.
	 */
	StatusMessage(const StatusMessage &copy);
	/**
	 * StatusMessage destructor
	 */
	~StatusMessage();
	/**
	 * StatusMessage copy-assignment operator.
	 * Very cheap because the class is implicit shared.
	 */
	StatusMessage &operator=(const StatusMessage &other);

	/**
	 * Verify if the status message is empty.
	 * @return true if the status message is empty.
	 */
	bool isEmpty() const;

	/**
	 * Add a metadata to the status message.
	 * @param key Key to identity the metadata.
	 * @param value Value for the metadata.
	 */
	void addMetaData(const QString &key, const QVariant &value);
	/**
	 * Add a hash of metadata to the status message.
	 * If a key already exists, it gets replaced (it doesn't use QHash::unite).;
	 * @param otherHash The hash to add.
	 */
	void addMetaData(const QHash<QString,QVariant> &otherHash);

	/**
	 * Check if the status message has the specified metadata.
	 * @param key Key of the metadata.
	 * @return true if the metadata is present.
	 */
	bool hasMetaData(const QString &key) const;
	/**
	 * Retrieve the specified metadata.
	 * @param key Key of the metadata
	 * @return The medata value
	 */
	QVariant metaData(const QString &key) const;

	/**
	 * Set a new status message.
	 * @param message New status message.
	 */ 
	void setMessage(const QString &message);
	/**
	 * Return the current status message.
	 * @return The current status message.
	 */
	QString message() const;

	/**
	 * Set a new status title.
	 * @param title New status title.
	 */
	void setTitle(const QString &title);

	/**
	 * Return the current status title.
	 * @return The current status title.
	 */
	QString title() const;

private:
	class Private;
	KSharedPtr<Private> d;
};

}

#endif
