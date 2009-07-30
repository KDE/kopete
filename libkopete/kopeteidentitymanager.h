/*
    kopeteidentitymanager.h - Kopete Identity Manager

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>

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

#ifndef __kopeteidentitymanager_h__
#define __kopeteidentitymanager_h__

#include <QtCore/QObject>

#include "kopete_export.h"
#include "kopeteonlinestatus.h"
#include "kopeteidentity.h"
#include "kopetestatusmessage.h"

namespace Kopete 
{

/**
 * IdentityManager manages all defined identities in Kopete. You can
 * query them and globally set them all online or offline from here.
 *
 * IdentityManager is a singleton, you may uses it with @ref IdentityManager::self()
 *
 * @author Gustavo Pichorim Boiko <gustavo.boiko\@kdemail.net>
 */
class KOPETE_EXPORT IdentityManager : public QObject
{
	Q_OBJECT

public:
	/**
	 * \brief Retrieve the instance of IdentityManager.
	 *
	 * The identity manager is a singleton class of which only a single
	 * instance will exist. If no manager exists yet this function will
	 * create one for you.
	 *
	 * \return the instance of the IdentityManager
	 */
	static IdentityManager* self();

	~IdentityManager();

	/**
	 * \brief Retrieve the list of identities
	 * \return a list of all the identities
	 */
	const Identity::List & identities() const;

	/**
	 * \brief Return the identity asked
	 * \param identityId is the ID for the identity
	 * \return the Identity object found or NULL if no identity was found
	 */
	Identity* findIdentity( const QString &identityId );


	/**
	 * \brief Returs the default identity to be used
	 *
	 * This is the default identity configured in kopete. If no identity was created
	 * yet, this function will create a new identity, set it as the default identity
	 * and return it.
	 * If there are identities already created, but none of them was set as the default,
	 * it will return the first identity of the list.
	 * @return the default identity
	 */
	Identity* defaultIdentity();

	/**
	 * @brief Sets a new default identity
	 *
	 * By changing the default identity, you do NOT change the accounts' identity
	 * association. They are kept as if nothing has changed
	 */
	void setDefaultIdentity(Identity *ident);
	
	/**
	 * \brief Delete the identity and clean the config data
	 *
	 * This will mostly be called when no account is assigned to an identity
	 */
	void removeIdentity( Identity *identity );

	/**
	 * @brief Register the identity.
	 *
	 * This adds the identity in the manager's identity list.
	 * It will check no identities already exist with the same ID, if any, the identity is deleted. and not added
	 *
	 * @return @p identity, or 0L if the identity was deleted because id collision
	 */
	Identity *registerIdentity( Identity *identity );

public slots:
	
	/**
	 * @brief Set all identities a status in the specified category
	 *
	 * @param category is one of the Kopete::OnlineStatusManager::Categories
	 * @param statusMessage is the new status message
	 * @param flags is a bitmask of SetOnlineStatusFlag
	 */
	void setOnlineStatus( /*Kopete::OnlineStatusManager::Categories*/ uint category,
	                      const Kopete::StatusMessage &statusMessage = Kopete::StatusMessage(), uint flags=0);

	/**
	 * \internal
	 * Save the identity data to KConfig
	 */
	void save();

	/**
	 * \internal
	 * Load the identity data from KConfig
	 */
	void load();

signals:
	/**
	 * \brief Signals when an identity is ready for use
	 */
	void identityRegistered( Kopete::Identity *identity );

	/**
	 * \brief Signals when an identity has been unregistered
	 *
	 * At this state, we are already in the Identity destructor.
	 */
	void identityUnregistered( const Kopete::Identity *identity );

	/**
	 * \brief Signals when the default identity has changed
	 */
	void defaultIdentityChanged( Kopete::Identity *identity );
	
	void identityOnlineStatusChanged( Kopete::Identity *identity );

private:
	/**
	 * Private constructor, because we're a singleton
	 */
	IdentityManager();

private slots:
	void slotIdentityOnlineStatusChanged( Kopete::Identity *i );

	/**
	 * \internal
	 * Unregister the identity.
	 */
	void unregisterIdentity( const Kopete::Identity *identity );

private:
	static IdentityManager *s_self;
	class Private;
	Private * const d;
};

} //END namespace Kopete


#endif


