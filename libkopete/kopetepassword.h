/*
    kopetepassword.h - Kopete Password

    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>
    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEPASSWORD_H
#define KOPETEPASSWORD_H

#include <qobject.h>
#include "kopete_export.h"

namespace KWallet { class Wallet; }

class QPixmap;

/** @internal */
class KopetePasswordGetRequest;
/** @internal */
class KopetePasswordSetRequest;
/** @internal */
class KopetePasswordClearRequest;

namespace Kopete
{

/**
 * @author Richard Smith       <kde@metafoo.co.uk>
 *
 * The Kopete::Password object is responsible for storing and retrieving a
 * password for a plugin or account object.
 *
 * If the KWallet is active, passwords will be stored in it, otherwise, they
 * will be stored in the KConfig, in a slightly mangled form.
 */
class KOPETE_EXPORT Password : public QObject
{
	Q_OBJECT

public:
	/**
	 * Create a new Kopete::Password object.
	 *
	 * @param configGroup The configuration group to save passwords in.
	 * @param maxLength The maximum length of the password, or 0 if no maximum exists.
	 * @param name The name for this object
	 *
	 * @deprecated Use the constructor that specifies if a blank password is allowed
	 */
	explicit Password( const QString &configGroup, uint maxLength = 0, const char *name = 0 );

	/**
	 * Create a new Kopete::Password object.
	 *
	 * @param configGroup The configuration group to save passwords in.
	 * @param maxLength The maximum length of the password, or 0 if no maximum exists.
	 * @param allowBlankPassword If this password is allowed to be blank
	 * @param name The name for this object
	 */
	explicit Password( const QString &configGroup, uint maxLength = 0,
		bool allowBlankPassword = false, const char *name = 0 );

	/**
	 * Create a shallow copy of this object
	 */
	Password( Password &other, const char *name = 0 );
	~Password();

	/**
	 * Assignment operator for passwords: make this object represent a different password
	 */
	Password &operator=( Password &other );

	/**
	 * Returns the preferred size for images passed to the retrieve and request functions.
	 */
	static int preferredImageSize();

	/**
	 * @brief Returns the maximum allowed length of the password, or 0 if there is no maximum.
	 */
	uint maximumLength();
	/**
	 * Sets the maximum allowed length of the password.
	 * @param max The new maximum allowed length, or 0 if there is no maximum.
	 */
	void setMaximumLength( uint max );

	/**
	 * @brief Returns whether the password currently stored by this object is known to be incorrect.
	 * This flag gets reset whenever the user enters a new password, and is
	 * expected to be set by the user of this class if it is detected that the
	 * password the user entered is wrong.
	 */
	bool isWrong();
	/**
	 * Flag the password as being incorrect.
	 * @see isWrong
	 */
	void setWrong( bool bWrong = true );

	/**
	 * Type of password request to perform:
	 * FromConfigOrUser : get the password from the config file, or from the user
	 * if no password in config.
	 * FromUser : always ask the user for a password (ie, if last password was
	 * wrong or you know the password has changed).
	 */
	enum PasswordSource { FromConfigOrUser, FromUser };

	/**
	 * @brief Start an asynchronous call to get the password.
	 * Causes a password entry dialog to appear if the password is not set. Triggers
	 * a provided slot when done, but not until after this function has returned (you
	 * don't need to worry about reentrancy or nested event loops).
	 *
	 * @param receiver The object to notify when the password request finishes
	 * @param slot The slot on receiver to call at the end of the request. The signature
	 *        of this function should be slot( const QString &password ). password will
	 *        be the password if successful, or QString::null if failed.
	 * @param image The icon to display in the dialog when asking for the password
	 * @param prompt The message to display to the user, asking for a
	 *        password. Can be any Qt RichText string.
	 * @param source The source the password is taken from if a wrong or
	 *        invalid password is entered or the password could not be found in the wallet
	 */
	void request( QObject *receiver, const char *slot, const QPixmap &image,
		const QString &prompt, PasswordSource source = FromConfigOrUser );

	/**
	 * @brief Start an asynchronous password request without a prompt
	 *
	 * Starts an asynchronous password request. Does not pop up a password entry dialog
	 * if there is no password.
	 * @see request(QObject*,const char*,const QPixmap&,const QString&,bool,unsigned int)
	 * The password given to the provided slot will be NULL if no password could be retrieved for
	 * some reason, such as the user declining to open the wallet, or no password being found.
	 */
	void requestWithoutPrompt( QObject *receiver, const char *slot );

	/**
	 * @return true if the password is remembered, false otherwise.
	 *
	 * If it returns false, calling @ref request() will
	 * pop up an Enter Password window.
	 */
	bool remembered();

	/**
	 * @return true if you are allowed to have a blank password
	 */
	bool allowBlankPassword();

	/**
	 * When a password request succeeds, the password is cached. This function
	 * returns the cached password, if there is one, or QString::null if there
	 * is not.
	 */
	QString cachedValue();

public slots:
	/**
	 * Set the password for this account.
	 * @param pass If set to QString::null, the password is forgotten unless you
	 *	specified to allow blank passwords. Otherwise, sets the password to
	 *	this value.
	 *
	 * Note: this function is asynchronous; changes will not be instant.
	 */
	void set( const QString &pass = QString::null );

	/**
	 * Unconditionally clears the stored password
	 */
	void clear();

private:
	void readConfig();
	void writeConfig();

	class Private;
	Private *d;

	//TODO: can we rearrange things so these aren't friends?
	friend class ::KopetePasswordGetRequest;
	friend class ::KopetePasswordSetRequest;
	friend class ::KopetePasswordClearRequest;
};

}

/**
 * This class is an implementation detail of KopetePassword.
 * @internal
 * @see KopetePassword
 */
class KopetePasswordRequestBase : public virtual QObject
{
	Q_OBJECT
signals:
	void requestFinished( const QString &password );
public slots:
	virtual void walletReceived( KWallet::Wallet *wallet ) = 0;
	virtual void slotOkPressed() = 0;
	virtual void slotCancelPressed() = 0;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
