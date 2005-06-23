/*  This file is part of the KDE project
    Copyright (C) 2005 Michal Vaner <michal.vaner@kdemail.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/
#ifndef SKYPECONTACT_H
#define SKYPECONTACT_H

#include <kopetecontact.h>

class SkypeAccount;
class QString;
class SkypeContactPrivate;
namespace Kopete {
	class MetaContact;
	class ChatSession;
}
class KAction;
template <class T> class QPtrList;
class SkypeChatSession;

/**
 * @author Kopete Developers
 */
class SkypeContact : public Kopete::Contact
{
	Q_OBJECT
	private:
		///some internal things
		SkypeContactPrivate *d;
		///This examines all factors of users online status and sets the status acordingly
		void resetStatus();
	private slots:
		///This will note that the session was destroyed and therefore can't be used again. As well used when the chat becomes multi-user so it no longer belongs to this contact
		void removeChat();
		///Enables or disables the call action depending on if it can be called or not.
		void enableCall(bool value);
		///The status changed, so there should be update of the availiblity of some things
		void statusChanged();
	public:
		/**
		 * Constructor.
		 * @param account Account to wich it belongs
		 * @param id ID of the new contact
		 * @param parent Metacontact to put it inside
		 */
		SkypeContact(SkypeAccount *account, const QString &id, Kopete::MetaContact *parent, bool user = true);
		/**
		 * Destructor.
		 */
		~SkypeContact();
		/**
		 * Creates a chat session.
		 * @param flags Can I create it?
		 * @return Pointer to that session
		 */
		virtual Kopete::ChatSession *manager(Kopete::Contact::CanCreateFlags flags);
		/**
		 * Save this contact (resp. set what should be saved and it will be written automatically by kopete)
		 */
		virtual void serialize(QMap<QString, QString> &serializedData, QMap<QString, QString> &addressBookData);
		///Returns full name for the contact
		virtual QString formattedName() const;
		///Is it reachable now?
		virtual bool isReachable();
		///Does this contact has opened chat session?
		bool hasChat() const;
		///Tell kopete which actions to show in the contact pop-up menu
		QPtrList<KAction> *customContextMenuActions();
		///Give me actually existing chat session
		SkypeChatSession *getChatSession();
		///Can this contact be called now?
		bool canCall() const;
	public slots:
		/**
		 * Please ask for the contact information (emit infoReques with your name)
		 */
		void requestInfo();
		/**
		 * Chnages something in the contact.
		 * @param change What change was it? It looks like [property] [value]
		 */
		void setInfo(const QString &change);
		/**
		 * This one showes message in the chat session.
		 * @param message The message to show
		 * @param chat The chat ID of the chat the message belongs to
		 */
		void receiveIm(const QString &message, const QString &chat);
		/**
		 * connection status changed
		 * @param connected Are we connected now?
		 */
		void connectionStatus(bool connected);
		///This slot calls a contact
		void call();
	signals:
		/**
		 * There is a request to get/refresh the contact info from skype
		 * @param contact Wich contact wants it?
		 */
		void infoRequest(const QString &contact);
		/**
		 * The possibility to call this contact has changed, so GUI should enable/disable some buttons.
		 * @param value Is it possible to call it now?
		 */
		void setCallPossible(bool value);
};

#endif
