// client.h - main interface to libgroupwise
#ifndef LIBGW_CLIENT_H
#define LIBGW_CLIENT_H

#include <qstring.h>

#include "gwclientstream.h"
#include "gwerror.h"
#include "rtf2html.h"
#include "transfer.h"

class Task;
class RequestFactory;
class UserDetailsManager;

using namespace GroupWise;

class Client : public QObject
{
Q_OBJECT

	public:
	
		/*************
		  EXTERNAL API 
		 *************/
		  
		Client(QObject *parent=0);
		~Client();
		void setOSName( const QString &name );
		void setClientName( const QString &s );
		void setClientVersion( const QString &s );
		void setUserDN( const QString & userDN );

		/**
		 * Start a connection to the server using the supplied @ref ClientStream.
		 * This is only a transport layer connection.
		 * Needed for protocol action P1.
		 * @param s initialised client stream to use for the connection.
		 * @param server the server to connect to - but this is also set on the connector used to construct the clientstream??
		 * @param auth needed for jabber protocol layer only?
		 */
		void connectToServer( ClientStream *s, const NovellDN &server, bool auth=true );
		
		/**
		 * Login to the GroupWise server using the supplied credentials
		 * Protocol action P1, needed for all
		 * @param host - probably could obtain this back from the connector - used for outgoing tasks to determine destination
		 * @param user The user name to log in as.
		 * @param password 
		 */ 
		void start( const QString &host, const uint port, const QString &userId, const QString &pass );
		
		/**
		 * Logout and disconnect
		 * Protocol action P4		void distribute(const QDomElement &);

		 */
		void close();

		/**
		 * Accessors needed for login
		 */
		QString host();
		int port();
		
		/** 
		 * Set the user's presence on the server
		 * Protocol action P2
		 * @param status class containing status, away message.
		 */
		void setStatus( GroupWise::Status status, const QString & reason );

		/**
		 * Send a message 
		 * Protocol action P11
		 * @param message contains the text and the recipient.
		 */
		void sendMessage( const QStringList & addresseeDNs, const OutgoingMessage & message );
		  
		/**
		 * Send a typing notification
		 * Protocol action P12
		 * @param conference The conference where the typing took place.
		 * @param typing True if the user is now typing, false otherwise.
		 */
		void sendTyping( const QString & conferenceGuid, bool typing );
		
		/** 
		 * Request details for one or more users, for example, if we receive a message from someone who isn't on our contact list
		 * @param userDNs A list of one or more user's DNs to fetch details for
		 */
		void requestDetails( const QStringList & userDNs );
		 
		/**
		 * Request the status of a single user, for example, if they have messaged us and are not on our contact list
		 */
		void requestStatus( const QString & userDN );
		
		/**
		 * Add a contact to the contact list
		 * Protocol action P13 
		 */
		 
		/**
		 * Remove a contact from the contact list
		 * Protocol action P14
		 */
		
		/**
		 * Instantiate a conference on the server
		 * Protocol action P5
		 */
		void createConference( const int clientId );
		/**
		 * Overloaded version of the above to create a conference with a supplied list of invitees
		 */
		void createConference( const int clientId, const QStringList & participants );
		
		/*************
		  INTERNAL (FOR USE BY TASKS) METHODS 
		 *************/
		/**
		 * Send an outgoing request to the server
		 */
		void send( Request *request );
		/**
		 * Print a debug statement
		 */
		void debug( const QString &str );
		/**
		 * Generate a unique ID for Tasks.
		 */
		QString genUniqueId();
		
		/**
		 * The current user's user ID
		 */
		QString userId();
		
		/**
		 * The current user's DN
		 */
		QString userDN();
		/**
		 * The current user's password
		 */
		QString password();
		
		/**
		 * User agent details for this host
		 */
		QString userAgent();
		
		/**
		 * Host's IP address
		 */
		QCString ipAddress();
		
		/**
		 * Get a reference to the RequestFactory for this Client. 
		 * Used by Tasks to generate Requests with an ascending sequence of transaction IDs 
		 * for this connection
		 */
		RequestFactory * requestFactory();
		/**
		 * Get a reference to the UserDetailsManager for this Client.
		 * Used to track known user details and issue new details requests
		 */
		UserDetailsManager * userDetailsManager();
		/**
		 * Access the root Task for this client, so tasks may be added to it.
		 */
		Task* rootTask();

	signals:
		/** CONNECTION EVENTS */
		/**
		 * Notifies that the login process has succeeded.
		 */
		void loggedIn();
		/**
		 * Notifies tasks and account so they can react properly
		 */
		void disconnected();
		/**
		 * We were disconnected because we connected elsewhere
		 */
		void connectedElsewhere();
		
		/** STATUS AND METADATA EVENTS */
		/**
		 * We've just got the user's own details from the server.
		 */
		void accountDetailsReceived( const ContactDetails & );
		/** 
		 * We've just found out about a folder from the server.
		 */
		void folderReceived( const FolderItem & );
		/** 
		 * We've just found out about a folder from the server.
		 */
		void contactReceived( const ContactItem & );
		/** 
		 * We've just received a contact's metadata from the server.
		 */
		void contactUserDetailsReceived( const ContactDetails & );
		/** 
		 * A remote contact changed status
		 */
		void statusReceived( const QString & contactId, Q_UINT16 status, const QString & statusText );
		/** 
		 * Our status changed on the server
		 */
		void ourStatusChanged( GroupWise::Status status, const QString & statusText, const QString & autoReply );
		
		/** CONFERENCE (MANAGEMENT) EVENTS */
		/** 
		 * Notify that we've just received a message.  Sender may not be on our contact list
		 */
		void messageReceived( const ConferenceEvent & );
		/** 
		 * A conference was successfully created on the server
		 */
		void conferenceCreated( const int clientId, const QString & guid );
		/**
		 * A third party was invited to join a chat.  They may not be on our contact list.
		 */
		void inviteNotifyReceived( const ConferenceEvent & );
		/**
		 * We were invited to join a chat.  The inviter may not be on our contact list
		 */
		void invitationReceived( const ConferenceEvent & );
		/**
		 * Someone joined a chat.  They may not be on our contact list if it is a group chat
		 * and they were invited to join the chat prior to our being invited to join and joining
		 */
		void conferenceJoined( const ConferenceEvent & );
		/**
		 * Someone left a conference. This may close a conference, see @ref conferenceClosed.
		 */
		void conferenceLeft( const ConferenceEvent & );
		/**
		 * Someone declined an invitation to join a conference. This may close a conference, see @ref conferenceClosed.
		 */
		void invitationDeclined( const ConferenceEvent & );
		/**
		 * A conference was closed by the server. This occurs if we are the only participant and there
		 * are no outstanding invitations.
		 */
		void conferenceClosed( const ConferenceEvent & );
		/**
		 * We received an "is typing" event in a conference
		 */
		void contactTyping( const ConferenceEvent & );
		/**
		 * We received an "is not typing event" in a conference
		 */
		void contactNotTyping( const ConferenceEvent & );
		/**
		 * An attempt to create a conference failed.
		 */
		 void conferenceCreationFailed( const int clientId, const int error );
		
		/** CONTACT LIST MANAGEMENT EVENTS */
		/** TBD! */
	protected:
		/**
		 * Instantiate all the event handling tasks
		 */
		void initialiseEventTasks();
	protected slots:
		// INTERNAL, FOR USE BY TASKS' finished() SIGNALS //
		void lt_loginFinished();
		void sst_statusChanged();
		void cct_conferenceCreated();
		/**
		 * Transforms an RTF message into an HTML message and emits messageReceived()
		 */ 
		void ct_messageReceived( const ConferenceEvent & );
		
		/**
		 * Used by the client stream to notify errors to upper layers.
		 */
		void streamError( int error );
		
		/**
		 * The client stream has data ready to read.
		 */
		void streamReadyRead();
		
		
		
	private:
		void distribute( Transfer *transfer );
		class ClientPrivate;
		ClientPrivate* d;
};

#endif
