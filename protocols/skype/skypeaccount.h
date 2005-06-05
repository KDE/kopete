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
#ifndef SKYPEACCOUNT_H
#define SKYPEACCOUNT_H

#include <kopeteaccount.h>

class SkypeProtocol;
class QString;
class SkypeAccountPrivate;
class SkypeContact;

namespace Kopete {
	class MetaContact;
	class OnlineStatus;
	class Message;
}

#define DBUS_SESSION 0
#define DBUS_SYSTEM 1

/**
 * @author Michal Vaner
 * @short Skype account
 * Account to use external skype program. At this time, it only supports one skype account at one, may be more in future.
 */
class SkypeAccount : public Kopete::Account
{
Q_OBJECT
	private:
		///Some internal things
		SkypeAccountPrivate *d;
	private slots:
		/**
		 * This sets the right icon for the status - online
		 */
		void wentOnline();
		/**
		 * This changes the account icon to offline
		 */
		void wentOffline();
		/**
		 * This changes the account icon to away
		 */
		void wentAway();
		/**
		 * This changes the account icon to not available
		 */
		void wentNotAvailable();
		/**
		 * This changes the account icon to Do not disturb
		 */
		void wentDND();
		/**
		 * This changes the status icon of account to Invisible
		 */
		void wentInvisible();
		/**
		 * This changes the status indicator to Skype me
		 */
		void wentSkypeMe();
		/**
		 * The status changed to actually connecting
		 */
		void statusConnecting();
		/**
		 * This adds user to the contact list if it is not there
		 * @param name The skype name of the contact
		 */
		void newUser(const QString &name);
		/**
		 * This is used for receiving messages from skype network
		 * @param user The user that sent it
		 * @param message The text of the message
		 */
		void receivedIm(const QString &user, const QString &message);
		/**
		 * New cal to show (or not, depending on setup) the call control window.
		 * @param callId ID of the new call
		 * @param userId User that is on the other end. If conference, list of IDs divided by spaces.
		 */
		void newCall(const QString &callId, const QString &userId);
		/**
		 * This one sets name of myself
		 * @param name What the new name is.
		 */
		void setMyselfName(const QString &name);
	protected:
		/**
		 * Creates new skype contact and adds it into the parentContact.
		 * @param contactID ID of the contact (the skype name)
		 * @param parentContact Metacontact to add it into.
		 * @return True if it worked, false otherwise.
		 */
		virtual bool createContact(const QString &contactID, Kopete::MetaContact *parentContact);
	public:
		/**
		 * Constructor.
		 * @param protocol The skype protocol pointer.
		 */
		SkypeAccount(SkypeProtocol *protocol);
		/**
		 * Destructor 
		 */
		~SkypeAccount();
		/**
		 * Finds contact of given id
		 * @param id id of the wanted contact
		 * @return eather pointer to that contact or 0L of it was not found.
		 */
		SkypeContact *contact(const QString &id);
		/**
		 * How to launch the Skype
		 */
		int launchType;
		/**
		 * How shoul kopete authorize it self? (empty means as kopete)
		 */
		QString author;
		/**
		 * This saves properties to the config file
		 */
		void save();
		/**
		 * Prepares this contact for life and integrates it into this account. Should be called only by the contact in its constructor.
		 * @param conntact The contact to prepare
		 */
		void prepareContact(SkypeContact *contact);
		///Can we comunicate with the skype? (not with the network, just with the program)
		bool canComunicate();
		///returns the protocol
		SkypeProtocol * protocol();
		/**
		 * @return Is the HitchHike mode enabled or not?
		 * @see setHitchHike
		 */
		bool getHitchHike() const;
		/**
		 * @return Is the MarkRead mode enabled or not?
		 * @see setMarkRead
		 */
		bool getMarkRead() const;
		/**
		 * Is the scan for unread message on login enabled?
		 * @return Is it enabled or not?
		 * @see setSearchForUnread
		 */
		bool getScanForUnread() const;
		/**
		 * @return true if this user already has opened chat session, false if he doesn't have opened chat session or the user do not exist
		 * @param userId ID of the user in interest
		 */
		bool userHasChat(const QString &userId);
		/**
		 * @return Should a control window be showed for calls?
		 */
		bool getCallControl() const;
		/**
		 * Is that call incoming or not?
		 * @param callId What call you want to know?
		 * @return true if the call is incoming call (someone calls you), false otherwise (outgoing, not a call at all..)
		 */
		bool isCallIncoming(const QString &callId);
		/**
		 * @return The time after the call finished to auto-closing the window. If auto-closing is disabled, 0 is returned
		 * @see setCallWindowTimeout
		 */
		int closeCallWindowTimeout() const;
		/**
		 * @return Returns name that shouls be showed by a call window
		 */
		QString getUserLabel(const QString &userId);
		/**
		 * Are pings to Skype enabled?
		 * @return You guess..
		 */
		bool getPings() const;
		/**
		 * What bus is set to use now?
		 * @return 0 as session bus, 1 as system wide
		 */
		int getBus() const;
		/**
		 * Is starting Dbus when it is not running enabled?
		 * @return You guess..
		 */
		bool getStartDBus() const;
		/**
		 * How long does it try to connect to newly started skype, until it gives up (seconds)
		 */
		int getLaunchTimeout() const;
		/**
		 * What is the command that launches skype?
		 */
		const QString &getSkypeCommand() const;
		/**
		 * Do we wait before connecting?
		 */
		int getWaitBeforeConnect() const;
	public slots:
		/**
		 * Disconnects from server.
		 */
		virtual void disconnect();
		/**
		 * Sets online status to away/online.
		 * @param away If true, it sets to away, otherwise it sets to online.
		 * @param reason Message to set. Ignored with skype as it does not support away messages. (Or I don't know about it))
		 */
		virtual void setAway(bool away, const QString &reason);
		/**
		 * Sets online status for the account.
		 * @param status Status to set.
		 * @param reason Away message. Ignored by skype.
		 */
		virtual void setOnlineStatus(const Kopete::OnlineStatus &status, const QString &reason);
		/**
		 * Connect to the skype with given status
		 * @param status The status to connect with. If it is something unusual (like offline or something unknown), online is used
		 */
		virtual void connect(const Kopete::OnlineStatus &status);
		/**
		 * This notifies contact of some change of its information
		 * @param contact What contact is it?
		 * @param change And what happende.
		 */
		void updateContactInfo(const QString &contact, const QString &change);
		/**
		 * This will send message by the skype connection. Will take care of all notifications when it is done and so. (means it will emit messageSent when it is sent)
		 * @param message What to send.
		 * @return Id of thet message.
		 */
		void sendMessage(Kopete::Message &message);
		/**
		 * Enables or disables the HitchHike mode of getting messages. If it is enabled, a new message to unstarted chat will be showed. If not, they will be ignored and you will have to open them in Skype
		 * @param value True enables HitchHike mode, false disables.
		 * @see getHitchHike
		 */
		void setHitchHike(bool value);
		/**
		 * Enables reading messages by kopete. If it is on, all messages showed in Kopete will be marked as read, if disable, you will have to read them in Skype/something else.
		 * If HitchHike mode is disabled, messages that creates chats are NOT marked as read, because they are not showed.
		 * @param value Enable or disable the mode
		 * @see getMarkRead
		 * @see getHitchHake
		 * @see setHitchHike
		 */
		void setMarkRead(bool value);
		/**
		 * Set if there should be scan for unread messages when kopete connects to Skype.
		 * @param value Enable or disable the scan.
		 * @see getScanForUnread
		 */
		void setScanForUnread(bool value);
		/**
		 * Make a call to that user
		 * @param user To who we call.
		 */
		void makeCall(SkypeContact *user);
		/**
		 * Set if a control window will be showed for calls.
		 * @param value Is it enabled or disabled now?
		 */
		void setCallControl(bool value);
		/**
		 * Sets timeout in seconds how long will be call window visible after the call finished. If you want to disable it, set to 0.
		 */
		void setCloseWindowTimeout(int timeout);
		/**
		 * Turns pinging skype on/off
		 * If it is on, every second a ping message is sent to skype so track of if Skype is running is still hold. f it is off, skype can be turned off and you won't know it.
		 * @param enabled Are they on or off from now?
		 */
		void setPings(bool enabled);
		/**
		 * Sets bus on which Skype listens
		 * @param bus 0 -> session bus, 1 -> system wide bus
		 */
		void setBus(int bus);
		/**
		 * Should be DBus started when needed?
		 */
		void setStartDBus(bool enabled);
		/**
		 * Set the timeout for giving up launching Skype
		 */
		void setLaunchTimeout(int seconds);
		/**
		 * Set command by what the Skype will be started
		 */
		void setSkypeCommand(const QString &command);
		/**
		 * Set if we wait a while before connecting to just started skype
		 */
		void setWaitBeforeConnect(int value);
	signals:
		/**
		 * This is emited when the ID of the last sent message is known
		 * @param messageId The ID of the message
		 */
		void gotMessageId(const QString &messageId);
		/**
		 * This is emited when the message has been sent by skype
		 * @param messageId Id of the message that has been sent
		 */
		void sentMessage(const QString &messageId);
		/**
		 * This slot notifies of connecting/disconnecting. Needed to be sure, if alling is possible.
		 * @param online Are we online now?
		 */
		void connectionStatus(bool online);
};

#endif
