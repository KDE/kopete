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
class QAction;

namespace Kopete
{

class Account;

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

	/**
	 * @brief Duplicates an existing identity
	 *
	 * This will create a new identity name @param id, that will clone all properties
	 * of the identity @param existing.
	 */
	Identity(Identity &existing);

	~Identity();

	/**
	 * @brief Initialize the identity actions
	 */
	void initActions();

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
	 * @brief Sets the online status for this identity
	 *
	 * FIXME: describe a bit more
	 */
	void setOnlineStatus( uint category, const QString &awayMessage );

	/**
	 * @brief Get the online status of the identity
	 * @return the online status of the identity
	 */
	OnlineStatus::StatusType onlineStatus() const;
	
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
	 * @brief Return the menu for this identity
	 */
	KActionMenu* actionMenu();

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
	 * @brief Removes an account from the identity
	 *
	 * @param account the account to be removed
	 */
	void removeAccount( const Kopete::Account *account );

	void updateOnlineStatus();

protected slots:
	void slotSaveProperty( PropertyContainer *container, const QString &key,
				const QVariant &oldValue, const QVariant &newValue );

private slots:
	void slotChangeStatus(QAction *);

signals:
	void onlineStatusChanged(Kopete::Identity*,Kopete::OnlineStatus::StatusType, Kopete::OnlineStatus::StatusType);
	void identityDestroyed( const Kopete::Identity *identity );
	void identityChanged(Kopete::Identity *identity);

private:
	class Private;
	Private *d;

};

} //END namespace Kopete

#endif

