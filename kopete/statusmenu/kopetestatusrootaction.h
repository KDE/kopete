/*
    kopetestatusrootaction.h - Kopete Status Root Action

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
#ifndef KOPETESTATUSROOTACTION_H
#define KOPETESTATUSROOTACTION_H

#include <QtCore/QObject>

#include "kopete_export.h"

class KActionMenu;
class QAction;

namespace Kopete
{
	class OnlineStatus;
	class Account;
	class StatusMessage;
	class StatusAction;

	namespace Status
	{
		class StatusItem;
		class Status;
	}

/**
	@author Roman Jarosz <kedgedev@centrum.cz>
*/

class KOPETE_STATUSMENU_EXPORT StatusRootAction : public QObject
{
	Q_OBJECT
public:
	enum Filter { UseCategory, UseStatus, UseStatusAndCategory };

	/**
	 * StatusRootAction constructor
	 * @param menu the parent menu
	 *
	 * The menu actions are automatically recreated if anything changes
	 * @note you should catch @p updateMessage, @p changeStatus and @p changeMessage signals
	 **/
	StatusRootAction( KActionMenu* menu );

	/**
	 * StatusRootAction constructor for specific account
	 * @param menu the parent menu
	 * @param filter menu filter type
	 * @param account kopete account
	 * @param account online status
	 *
	 * The menu actions are automatically recreated if anything changes
	 * @note you should catch @p updateMessage signal
	 **/
	StatusRootAction( KActionMenu* menu, Filter filter, Account *account, const OnlineStatus &onlineStatus, QAction * before = 0 );
	~StatusRootAction();

	/**
	 * insert "setStatus" actions from the given account to the specified actionMenu.
	 *  (actions have that menu as parent QObject)
	 * they are connected to the Account::setOnlineStatus signal
	 *
	 * Items are stored by status height.
	 *
	 * @param account the account
	 * @param parent  the ActionMenu where action are inserted
	 */
	static void createAccountStatusActions( Account *account , KActionMenu *parent, QAction * before = 0 );

	/**
	 * @brief Returns menu filter used for filtering the status actions
	 */
	StatusRootAction::Filter filter() const;
	
	/**
	 * @brief Returns default category
	 */
	int category() const;

	/**
	 * @brief Returns default online status
	 */
	OnlineStatus onlineStatus() const;

	/**
	 * @brief Get the account this root action was created for
	 *
	 * @return account for this root action or 0.
	 */
	Account *account() const;

	/**
	 * @brief Set current status message
	 *
	 * @param statusMessage current status message
	 */
	void setCurrentMessage( const Kopete::StatusMessage &statusMessage );

Q_SIGNALS:
	/**
	 * @brief Update status message for this action
	 * @param statusRootAction the StatusRootAction object which emitted this signal
	 *
	 * This signal is emitted before the status menu is shown and a object
	 * that is receiving this signal should update status message of @p statusRootAction
	 * with @p setCurrentMessage
	 */
	void updateMessage( Kopete::StatusRootAction *statusRootAction );

	/**
	 * @brief Status category and message change request
	 * @param category of new status
	 * @param statusMessage of new status
	 *
	 * This signal is emitted when a status action is activated by the user
	 *
	 * @note this is only emitted if this object wasn't create for specific account
	 */
	void changeStatus( uint category, const Kopete::StatusMessage &statusMessage );

	/**
	 * @brief Status message change request
	 * @param statusMessage of new status
	 *
	 * This signal is emitted when a status action is activated by the user
	 *
	 * @note this is only emitted if this object wasn't created for specific account
	 */
	void changeMessage( const Kopete::StatusMessage &statusMessage );

protected:
	friend class Kopete::StatusAction;
	void changeStatus( const Kopete::Status::Status* status );

private Q_SLOTS:
	void rootChanged();

	void childInserted( int i, Kopete::Status::StatusItem* child );
	void childRemoved( Kopete::Status::StatusItem* child );

	void setStatusMessage( const Kopete::StatusMessage &statusMessage );
	void editStatuses();
	void showEditStatusDialog();
	void editStatusDialogFinished(int);
	
private:
	void init();
	void insertChild( QAction * before, Status::StatusItem* child );
	
	class Private;
	Private * const d;
};

}

#endif
