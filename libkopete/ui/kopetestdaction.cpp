#include <kaction.h>
#include <klocale.h>

#include "kopete.h"
#include "kopetestdaction.h"
#include "contactlist.h"

/** KopeteGroupList **/
KopeteGroupList::KopeteGroupList(const QString& text, const QString& pix, const KShortcut& cut, const QObject* receiver, const char* slot, QObject* parent, const char* name)
                : KListAction(text, pix, cut, receiver, slot, parent, name)
{
	connect(kopeteapp->contactList(), SIGNAL(groupAdded(const QString &)), this, SLOT(slotUpdateList()));
	connect(kopeteapp->contactList(), SIGNAL(groupRemoved(const QString &)), this, SLOT(slotUpdateList()));
	slotUpdateList();
}

KopeteGroupList::~KopeteGroupList()
{
}

void KopeteGroupList::slotUpdateList()
{
	setItems(kopeteapp->contactList()->groups());
}

/** KopeteStdAction **/
KAction* KopeteStdAction::sendMessage(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0)
{
	return new KAction("&Send Message", "mail_generic", 0, recvr, slot, parent, name);
}

KAction* KopeteStdAction::contactInfo(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0)
{
	return new KAction("User &Info", "identity", 0, recvr, slot, parent, name);
}

KAction* KopeteStdAction::viewHistory(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0)
{
	return new KAction("View &History", "history", 0, recvr, slot, parent, name);
}

KAction* KopeteStdAction::addGroup(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0)
{
	return new KAction("&Add Group...", "folder", 0, recvr, slot, parent, name);
}

KListAction *KopeteStdAction::moveContact(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0)
{
	return new KopeteGroupList("&Move Contact", "editcut", 0, recvr, slot, parent, name);
}

KAction* KopeteStdAction::deleteContact(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0)
{
	return new KAction("&Delete Contact",  "edittrash", 0, recvr, slot, parent, name);
}
