/*
    kopeteidentity.h - Kopete Identity

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
              (c) 2007         Will Stephenson <wstephenson@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEIDENTITY_H
#define KOPETEIDENTITY_H

#include <kdemacros.h>
#include "kopeteglobal.h"
#include "kopetepropertycontainer.h"
#include "kopeteonlinestatus.h"
#include "kopete_export.h"

class KConfigGroup;

namespace Kopete
{

class Account;
class StatusMessage;

/**
 * @author Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
 *
 * An identity that might contain one or more accounts associated to it 
 */
class KOPETE_EXPORT Identity : public PropertyContainer
{
	Q_OBJECT
public:	
	typedef QList<Identity*> List;
	/**
	 * @brief The main constructor for Kopete Identities
	 *
	 * This will create an empty identity with a random id
	 * @param label the label to use for this identity
	 */
	Identity(const QString &label);

	/**
	 * @brief Constructor for deserialising stored Identities
	 * @param id the stored Identity's id
	 * @param label the stored Identity's label
	 */
	Identity(const QString &id, const QString &label);

	~Identity();

	/**
	 * @brief Duplicates an existing identity
	 *
	 * This will create a new identity name @param id, that clones all properties
	 * @return duplicate Identity
	 */
	Identity * clone() const;
	/**
	 * The id is a unique internal handle and should not be exposed in the UI
	 * @return the identity's id
	 */
	QString id() const;

	/**
	 * The label is used to identify the identity in the UI
	 * @return the identity's label
	 */
	QString label() const;

	/**
	 * Sets the label
	 * @param newLabel a new label for the identity
	 */
	void setLabel( const QString & label );

	/**
	 * This identity should be connected when connect all is called?
	 */
	bool excludeConnect() const;

	/**
	 * @brief Get the online status of the identity
	 * @return the online status of the identity
	 */
	OnlineStatus::StatusType onlineStatus() const;

	/**
	 * @brief Get the current status message of the identity
	 */
	Kopete::StatusMessage statusMessage() const;
	
	/**
	 * \brief Get the tooltip for this identity
	 * \return an RTF tooltip depending on Kopete::AppearanceSettings settings
	 **/
	QString toolTip() const;

	/**
	 * \brief Return the icon for this identity
	 */
	QString customIcon() const;

	/**
	 * Returns the accounts assigned to this identity
	 */
	QList<Account*> accounts() const;

	/**
	 * @brief Adds an account to the identity
	 *
	 * @param account the account to be added
	 */
	void addAccount( Kopete::Account *account );

	/**
	 * Returns the @ref KConfigGroup that should be used to read/write settings 
	 * of this identity
	 */
	KConfigGroup *configGroup() const;

	/**
	 * Load the identity information
	 */
	void load();

	/**
	 * Save the identity information
	 */
	void save();

public slots:
	/**
	 * @brief Sets the online status for this identity
	 * Sets the online status for each account in this identity, except those which are set to
	 * 'Exclude from connect all' (Kopete::Account::excludeConnect()).
	 * @param category generic OnlineStatusManager::Categories identifying status to set
	 * @param statusMessage is the new status message to use in this onlinestatus
	 */
	void setOnlineStatus( uint category, const Kopete::StatusMessage &statusMessage );

	/**
	 * @brief Sets the status message for this identity
	 * Sets the status message for each account in this identity, except those which are set to
	 * 'Exclude from connect all' (Kopete::Account::excludeConnect()).
	 * @param statusMessage is the new status message to use
	 */
	void setStatusMessage( const Kopete::StatusMessage &statusMessage );
	
	/**
	 * @brief Removes an account from the identity
	 *
	 * @param account the account to be removed
	 */
	void removeAccount( const Kopete::Account *account );

	void updateOnlineStatus();

protected slots:
	void slotSaveProperty( Kopete::PropertyContainer *container, const QString &key,
				const QVariant &oldValue, const QVariant &newValue );

signals:
	void onlineStatusChanged( Kopete::Identity* );
	void toolTipChanged( Kopete::Identity* );
	void identityDestroyed( const Kopete::Identity *identity );
	void identityChanged(Kopete::Identity *identity);

private:
	class Private;
	Private * const d;

};

} //END namespace Kopete

#endif

