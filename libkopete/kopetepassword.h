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

class QPixmap;

/**
 * @author Richard Smith       <kde@metafoo.co.uk>
 *
 * The KopetePassword object is responsible for storing and retrieving a
 * password for a plugin or account object.
 *
 * If the KWallet is active, passwords will be stored in it, otherwise, they
 * will be stored in the KConfig, in a slightly mangled form.
 */
class KopetePassword : public QObject
{
	Q_OBJECT

public:
	/**
	 * Create a new KopetePassword object.
	 *
	 * @param configGroup The configuration group to save passwords in
	 *        if using KConfig
	 */
	KopetePassword(const QString &configGroup, const char *name = 0);
	~KopetePassword();

	/**
	 * Get the password. Has the ability to open an input dialog
	 * if the password is not currently set.
	 *
	 * @param image An image to display in the enter password dialog.
	 * @param prompt The message to display to the user, asking for a
	 *        password. Can be any Qt RichText string.
	 * @param error Set this value to true if you previously called
	 *        password and the result was incorrect (the password was
	 *        wrong). It adds a label in the input dialog saying that the
	 *        password was wrong.
	 * @param ok This value is set to false if the user cancelled an
	 *        Enter Password dialog, true otherwise.
	 * @param maxLength The maximum length for a password, restricts the
	 *        length of the password that can be entered. 0 means no limit.
	 * @return The password or QString::null if the user has canceled
	 */
	QString retrieve( const QPixmap &image, const QString &prompt, bool error = false, bool *ok = 0, unsigned int maxLength = 0 );

	/**
	 * @return true if the password is remembered, false otherwise.
	 *
	 * If it returns false, calling @ref password() will
	 * pop up an Enter Password window.
	 */
	bool remembered();

public slots:
	/**
	 * Set the password for this account.
	 * @param pass If set to QString::null, the password is forgotten,
	 *             otherwise sets the password to this value.
	 */
	void set( const QString &pass = QString::null );

private:
	void readConfig();
	void writeConfig();

	struct KopetePasswordPrivate;
	KopetePasswordPrivate *d;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

