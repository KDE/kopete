/*
	kopeteaccountmanager.h - Kopete Identity Manager

	Copyright (c) 2002      by Martijn Klingens      <klingens@kde.org>
	Copyright (c) 2003      by Olivier Goffart       <ogoffart@tiscalinet.be>

	Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

	*************************************************************************
	*                                                                       *
	* This program is free software; you can redistribute it and/or modify  *
	* it under the terms of the GNU General Public License as published by  *
	* the Free Software Foundation; either version 2 of the License, or     *
	* (at your option) any later version.                                   *
	*                                                                       *
	*************************************************************************
	*/

#ifndef __kopeteaccountmanager_h__
#define __kopeteaccountmanager_h__

#include <qobject.h>
#include <qptrlist.h>
#include <qdict.h>
#include <qdom.h>

class KopeteIdentity;
class KopeteProtocol;
class KopetePlugin;

/**
 * @author Martijn Klingens <klingens@kde.org>
 *
 * KopeteIdentityManager manages all defined identities in Kopete. You can
 * query them, and globally set them all online or offline from here.
 */
class KopeteIdentityManager : public QObject
{
		Q_OBJECT

public:
	/**
	 * Retrieve the instance of KopeteIdentityManager.
	 * The identity manager is a singleton class of which only a single
	 * instance will exist. If no manager exists yet this function will
	 * create one for you.
	 */
	static KopeteIdentityManager* manager();
		
	~KopeteIdentityManager();
	
	/**
	 * @internal
	 * Register the identity. 
	 * To be called ONLY from KopeteContact, not from any other class!
	 * (Not even a derived class).
	 */
	void registerIdentity(KopeteIdentity* );

	/**
	 * Retrieve the list of identities
	 */
	const QPtrList<KopeteIdentity>& identities() const;
	
	/**
	 * Retrieve a QDict of identities for the given protocol 
	 *
	 * The list is guaranteed to contain only contacts for the specified
	 * protocol
	 */
	QDict<KopeteIdentity> identities(const KopeteProtocol *p);

	KopeteIdentity* findIdentity(const QString& protocolId, const QString& identityId);

public slots:
	/**
	 * Connect all identities which have auto connect enabled
	 */
	 void autoConnect();

	/**
	 * Connect all identities at once.
	 * This is a slot, so you can connect directly to it from e.g. a KAction.
	 */
	void connectAll();
		
	/**
	 * Disconnect all identities at once.
	 * This is a slot, so you can connect directly to it from e.g. a KAction.
	 */
	void disconnectAll();

	/**
	 * Set all identities to away at once.
	 * This is a slot, so you can connect directly to it from e.g. a KAction.
	 */
	void setAwayAll();
	
	/**
	 * Remove the away status from all identities at once.
	 * This is a slot, so you can connect directly to it from e.g. a KAction.
	 */
	void setAvailableAll();
		
	/**
	 * save to identities.xml
	 */
	void save();
	/**
	 * load identities.xml
	 */
	void load();

private:
		/**
		 * Private constructor, because we're a singleton
		 */
	KopeteIdentityManager();
	static KopeteIdentityManager *s_manager;

	QPtrList<KopeteIdentity> m_identities;
	QDomDocument m_identityList;

private slots:
	void slotIdentityDestroyed(KopeteIdentity* );
	void loadProtocol(KopetePlugin* );
	
};

#endif
/*
 * Local variables:
 * mode: c++
 * c-indentation-style: k&r
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

