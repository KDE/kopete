#include <kaction.h>
#include <klocale.h>

#include "kopete.h"
#include "kopetestdaction.h"
#include "contactlist.h"

KAction* KopeteStdAction::sendMessage(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0)
{
	return new KAction("&Send Message", "mail_generic", 0, recvr, slot, parent, name);
}

KAction* KopeteStdAction::contactInfo(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0)
{
	return new KAction("Contact &Info", "identity", 0, recvr, slot, parent, name);
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
	KListAction *tmp = new KListAction("&Move Contact", "editcut", 0, recvr, slot, parent, name);
	tmp->setItems(kopeteapp->contactList()->groups());
	return tmp;
}

KAction* KopeteStdAction::deleteContact(const QObject *recvr, const char *slot, QObject* parent, const char *name = 0)
{
	return new KAction("&Delete Contact",  "edittrash", 0, recvr, slot, parent, name);
}
