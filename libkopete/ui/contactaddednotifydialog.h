/*
    Copyright (c) 2005      Olivier Goffart           <ogoffart@kde.org>

    Kopete    (c) 2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/


#ifndef KOPETE_UICONTACTADDEDNOTIFYDIALOG_H
#define KOPETE_UICONTACTADDEDNOTIFYDIALOG_H

#include <kdialog.h>
#include "kopete_export.h"

namespace KABC {
	class Addressee;
}

namespace Kopete {

class Group;
class Account;
class MetaContact;

namespace UI {

/**
 * @brief Dialog which is shown when a contact added you in the contact list.
 *
 * This dialog asks the user to give authorization for the addition to the
 * person who added the user and also asks the user if the contact who you've
 * received the notification for should be added to the user's contact list
 *
 * example of usage
 * @code

	Kopete::UI::ContactAddedNotifyDialog *dialog =
			new ContactAddedNotifyDialog(contactId, QString(),account);
	QObject::connect(dialog,SIGNAL(applyClicked(const QString&)),this,SLOT(contactAddedDialogApplied()));
	QObject::connect(dialog,SIGNAL(infoClicked(const QString&)),this,SLOT(contactAddedDialogInfo()));
	dialog->show();

 * @endcode
 *
 * and in your contactAddedDialogApplied slot
 * @code
	const Kopete::UI::ContactAddedNotifyDialog *dialog =
			dynamic_cast<const Kopete::UI::ContactAddedNotifyDialog *>(sender());
	if(!dialog)
		return;
	if(dialog->authorized())
		socket->authorize(contactId);
	if(dialog->added())
		dialog->addContact();
 * @endcode
 *
 * Note that you can also use exec() but this is not recommended
 *
 * @author Olivier Goffart
 */
class KOPETE_EXPORT ContactAddedNotifyDialog : public KDialog
{
Q_OBJECT
public:
	/**
	 * All widget in the dialog that may be hidden.
	 */
	enum HideWidget
	{
		DefaultHide = 0x00, ///< Internal default.
		InfoButton = 0x01, /**< the button which ask for more info about the contact */
		AuthorizeCheckBox = 0x02, /**< the checkbox which ask for authorize the contact */
		AddCheckBox = 0x04, /**< the checkbox which ask if the contact should be added */
		AddGroupBox = 0x08 /**< all the widget about metacontact properties */
	};
	Q_DECLARE_FLAGS(HideWidgetOptions, HideWidget)

	/**
	 * @brief Constructor
	 *
	 * The dialog is by default not modal, and will delete itself when closed
	 *
	 * @param contactId the contactId of the contact which just added the user
	 * @param contactNick the nickname of the contact if available.
	 * @param account is used to display the account icon and informaiton about the account
	 * @param hide a bitmask of HideWidget used to hide some widget. By default, everything is shown.
	 *
	 */
	explicit ContactAddedNotifyDialog(const QString& contactId, const QString& contactNick=QString(),
					Kopete::Account *account=0L, const HideWidgetOptions &hide=DefaultHide);

	/**
	 * @brief Destructor
	 */
	~ContactAddedNotifyDialog();

	/**
	 * @brief return if the user has checked the "authorize" checkbox
	 * @return true if the authorize checkbox is checked, false otherwise
	 */
	bool authorized() const;

	/**
	 * @brief return if the user has checked the "add" checkbox
	 * @return true if the add checkbox is checked, false otherwise
	 */
	bool added() const;

	/**
	 * @brief return the display name the user has entered
	 */
	QString displayName() const;

	/**
	 * @brief return the group the user has selected
	 *
	 * If the user has entered a group which doesn't exist yet, it will be created now
	 */
	Group* group() const;

public slots:

	/**
	 * @brief create a metacontact.
	 *
	 * This function only works if the add checkbox is checked, otherwise,
	 * it will return 0L.
	 *
	 * it uses the Account::addContact function to add the contact
	 *
	 * @return the new metacontact created, or 0L if the operation failed.
	 */
	MetaContact *addContact() const;

signals:
	/**
	 * @brief the dialog has been applied
	 * @param contactId is the id of the contact passed in the constructor.
	 */
	void applyClicked(const QString &contactId);

	/**
	 * @brief the button "info" has been pressed
	 * If you haven't hidden the more info button, you should connect this
	 * signal to a slot which show a dialog with more info about the
	 * contact.
	 *
	 * hint: you can use sender() as parent of the new dialog
	 * @param contactId is the id of the contact passed in the constructor.
	 */
	void infoClicked(const QString &contactId);


private slots:
	void slotAddresseeSelected( const KABC::Addressee &);
	void slotInfoClicked();
	void slotFinished();

private:
	struct Private;
	Private * const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( ContactAddedNotifyDialog::HideWidgetOptions )

} // namespace UI
} // namespace Kopete
#endif
