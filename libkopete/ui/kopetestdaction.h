#ifndef KOPETESTDACTION_H
#define KOPETESTDACTION_H

#include <kaction.h>
#include <qobject.h>

class KopeteStdAction
{
	public:
		// Contact context menu actions
		static KAction *chat( const QObject *recvr, const char *slot,
			QObject* parent, const char *name = 0 );
		static KAction *sendMessage(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0);

		static KAction *contactInfo(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0);
		static KAction *viewHistory(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0);

		static KAction *addGroup(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0);
		static KListAction *moveContact(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0);
		static KListAction *copyContact( const QObject *recvr,
			const char *slot, QObject* parent, const char *name = 0 );
		static KAction *deleteContact(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0);
};

// Fuckin' moc will only see signals/slots if they're defined in the .h
// This is a private class :(
class KopeteGroupList : public KListAction
{
	Q_OBJECT
	public:
		KopeteGroupList(const QString &, const QString &, const KShortcut &, const QObject *, const char *, QObject *, const char *);
		~KopeteGroupList();

	protected slots:
		void slotUpdateList();
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

