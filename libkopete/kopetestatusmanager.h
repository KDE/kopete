/*
    kopetestatusmanager.h - Kopete Status Manager

    Copyright (c) 2008      by Roman Jarosz          <kedgedev@centrum.cz>
    Kopete    (c) 2008      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#ifndef KOPETESTATUSMANAGER_H
#define KOPETESTATUSMANAGER_H

#include <QtCore/QObject>

#include "kopete_export.h"
#include "kopetestatusmessage.h"

class QDomElement;

namespace Kopete {

class Account;
class OnlineStatus;

namespace Status {
	class StatusGroup;
	class StatusItem;
}

/**
 * StatusManager manages status items that are used to build status menu.
 * It also stores current status message and sets auto away status.
 *
 * StatusManager is a singleton, you may uses it with @ref StatusManager::self()
 *
 * @author Roman Jarosz <kedgedev@centrum.cz>
 */
class KOPETE_EXPORT StatusManager : public QObject
{
	Q_OBJECT
public:
	/**
	 * Get the only instance of StatusManager
	 * @return StatusManager single instance
	 */
	static StatusManager *self();

	~StatusManager();

	/**
	 * Save status data tree into XML file
	 */
	void saveXML();
	
	/**
	 * Load status data tree into XML file
	 */
	void loadXML();

	/**
	 * Set new status data tree
	 */
	void setRootGroup( Status::StatusGroup * rootGroup );
	
	/**
	 * Get current status data tree
	 */
	Status::StatusGroup *getRootGroup() const;

	/**
	 * Copy current status data tree
	 */
	Status::StatusGroup *copyRootGroup() const;

	/**
	 * Find status item for given uid
	 */
	const Status::StatusItem *itemForUid( const QString &uid ) const;

	/**
	 * Convert status item to XML structure
	 *
	 * @note it's public because it's also used for drag&drop
	 */
	static QDomElement storeStatusItem( const Status::StatusItem *item );
	
	/**
	 * Restore status item from XML structure
	 *
	 * @note it's public because it's also used for drag&drop
	 */
	static Status::StatusItem *parseStatusItem( QDomElement element );

	/**
	 * Remember current global status
	 */
	void setGlobalStatus( uint category, const Kopete::StatusMessage &statusMessage = Kopete::StatusMessage() );
	
	/**
	 * Remember current global status message
	 */
	void setGlobalStatusMessage( const Kopete::StatusMessage &statusMessage = Kopete::StatusMessage() );

	/**
	 * Get current global status message
	 */
	Kopete::StatusMessage globalStatusMessage() const;

	/**
	 * Get current global status category
	 */
	uint globalStatusCategory() const;

	/**
	 * Returns true if auto away status was set
	 */
	bool autoAway();

	/**
	 * Returns true if global away status was set
	 */
	bool globalAway();

public Q_SLOTS:
	/**
	 * Undo auto away
	 */
	void setActive();

	/**
	 * Confirm with the user, then set auto away
	 */
	void askAndSetActive();
	
	/**
	 * Set all online account to auto away status
	 */
	void setAutoAway();

Q_SIGNALS:
	/**
	 * This signal is emitted when root item of status data tree has changed.
	 */
	void changed();

	/**
	 * This signal is emitted when global status has changed.
	 */
	void globalStatusChanged();

private Q_SLOTS:
	void accountUnregistered( const Kopete::Account *account );
	void checkIdleTimer();
	void loadSettings();
	void loadBehaviorSettings();
	
private:
	StatusManager();
	void updateUidHash( Status::StatusItem *item );

	Status::StatusGroup *defaultStatuses() const;

	static StatusManager *instance;

	class Private;
	Private * const d;
};

}

#endif
