/*
    kopetestdaction.h  -  Kopete Standard Actionds

    Copyright (c) 2001-2002 by Ryan Cumming. <bodnar42@phalynx.dhs.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETESTDACTION_H
#define KOPETESTDACTION_H

#include <kaction.h>
#include <qobject.h>

/**
 * @author Ryan Cumming. <bodnar42@phalynx.dhs.org>
 * //FIXME: I WANT TO GET RIDE OF THIS CLASS  - Olivier
 */
class KopeteStdAction
{
	public:
		// Contact context menu actions
		static KAction *chat( const QObject *recvr, const char *slot,
			QObject* parent, const char *name = 0 );
		static KAction *sendMessage(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0);

		static KAction *contactInfo(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0);
		static KAction *viewHistory(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0);
		static KAction *sendFile(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0);
		
		static KAction *changeMetaContact(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0);
		
		static KAction *addGroup(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0);
		static KListAction *moveContact(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0);
		static KListAction *copyContact( const QObject *recvr, const char *slot, QObject* parent, const char *name = 0 );
		static KAction *deleteContact(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0);
		static KAction *changeAlias(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0);

};

// Fuckin' moc will only see signals/slots if they're defined in the .h
// This is a private class :(
class KopeteGroupListAction : public KListAction
{
	Q_OBJECT
	public:
		KopeteGroupListAction(const QString &, const QString &, const KShortcut &, const QObject *, const char *, QObject *, const char *);
		~KopeteGroupListAction();

	protected slots:
		void slotUpdateList();
	private:
		QStringList m_groupList;
};

#endif
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

