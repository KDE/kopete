/*
    kopetestdaction.h  -  Kopete Standard Actionds

    Copyright (c) 2001-2002 by Ryan Cumming     <bodnar42@phalynx.dhs.org>
    Copyright (c) 2002-2003 by Martijn Klingens <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETESTDACTION_H
#define KOPETESTDACTION_H

#undef KDE_NO_COMPAT
#include <kaction.h>
#include <qobject.h>

#include "kopete_export.h"

/**
 * @author Ryan Cumming <bodnar42@phalynx.dhs.org>
 */
class KOPETE_EXPORT KopeteStdAction
{
public:
	/**
	 * Standard action to start a chat
	 */
	static KAction *chat( const QObject *recvr, const char *slot,
		QObject* parent, const char *name = 0 );
	/**
	 * Standard action to send a single message
	 */
	static KAction *sendMessage(const QObject *recvr, const char *slot,
		QObject* parent, const char *name = 0);
	/**
	 * Standard action to open a user info dialog
	 */
	static KAction *contactInfo(const QObject *recvr, const char *slot,
		QObject* parent, const char *name = 0);
	/**
	 * Standard action to open a history dialog or something similar
	 */
	static KAction *viewHistory(const QObject *recvr, const char *slot,
		QObject* parent, const char *name = 0);
	/**
	 * Standard action to initiate sending a file to a contact
	 */
	static KAction *sendFile(const QObject *recvr, const char *slot,
		QObject* parent, const char *name = 0);
	/**
	 * Standard action to change a contacts @ref Kopete::MetaContact
	 */
	static KAction *changeMetaContact(const QObject *recvr, const char *slot,
		QObject* parent, const char *name = 0);
	/**
	 * Standard action to add a group
	 */
	static KAction *addGroup(const QObject *recvr, const char *slot,
		QObject* parent, const char *name = 0);
	/**
	 * Standard action to delete a contact
	 */
	static KAction *deleteContact(const QObject *recvr, const char *slot,
		QObject* parent, const char *name = 0);
	/**
	 * Standard action to change a contact alias/nickname in your contactlist
	 */
	static KAction *changeAlias(const QObject *recvr, const char *slot,
		QObject* parent, const char *name = 0);
	/**
	 * Standard action to block a contact
	 */
	static KAction *blockContact(const QObject *recvr, const char *slot,
		QObject* parent, const char *name = 0);
	/**
	 * Standard action to unblock a contact
	 */
	static KAction *unblockContact(const QObject *recvr, const char *slot,
		QObject* parent, const char *name = 0);

	/**
	 * Return an action to change the Kopete preferences.
	 *
	 * The object has no signal/slot, the prefs are automatically shown
	 */
	static KAction *preferences(KActionCollection *parent, const char *name = 0);
};


namespace KSettings
{
	  class Dialog;
}

class KOPETE_EXPORT KopetePreferencesAction : public KAction
{
	  Q_OBJECT

	  public:
			 KopetePreferencesAction( KActionCollection *parent, const char *name = 0 );
			 ~KopetePreferencesAction();

	  protected slots:
			 void slotShowPreferences();
	  private:
			 static KSettings::Dialog *s_settingsDialog;
};

#endif
// vim: set noet ts=4 sts=4 sw=4:
