#ifndef KOPETESTDACTION_H
#define KOPETESTDACTION_H

#include <kaction.h>

class QObject;

class KopeteStdAction
{
	public:
		// Contact context menu actions
		static KAction *sendMessage(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0);

		static KAction *contactInfo(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0);
		static KAction *viewHistory(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0);

		static KAction *addGroup(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0);
		static KListAction *moveContact(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0);
		static KAction *deleteContact(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0);
};

#endif
