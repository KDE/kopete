// client.h - main interface to libgroupwise
#ifndef LIBGW_CLIENT_H
#define LIBGW_CLIENT_H

#include <qstring.h>

#include "gwclientstream.h"
#include "gwerror.h"
#include "transfer.h"

class Task;
class RequestFactory;

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

		void start( const QString &host, const QString &user, const QString &pass );
		
		/**
		 * Logout and disconnect
		 * Protocol action P4		void distribute(const QDomElement &);

		 */
		void close();
		
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
		void sendMessage( const Message &message );
		  
		/**
		 * Send a typing notification
		 * Protocol action P12
		 * @param conference The conference where the typing took place.
		 * @param typing True if the user is now typing, false otherwise.
		 */
		void sendTyping( /*Conference &conference ,*/ bool typing );
		
		/**
		 * Add a contact to the contact list
		 * Protocol action P13 
		 */
		 
		/**
		 * Remove a contact from the contact list
		 * Protocol action P14
		 */
		
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
	signals:
		/**
		 * Notifies that the login process has succeeded.
		 */
		void loggedIn();
		/**
		 * Notifies tasks and account so they can react properly
		 */
		void disconnected();
		/** 
		 * Notify that we've just received a message 
		 */
		void messageReceived( const ConferenceEvent &, const Message & );
		/**
		 * We've just got the user's own details from the server.
		 */
		void accountDataReceived( const ContactItem & );
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
	public slots:
		// INTERNAL, FOR USE BY TASKS' SIGNALS //
		void lt_loginFinished();
		void slotMessageReceived( const ConferenceEvent &, const Message & );
		void sst_statusChanged();
	protected:
		/**
		 * Instantiate all the event handling tasks
		 */
		void initialiseEventTasks();
	protected slots:
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
		Task* rootTask();
		class ClientPrivate;
		ClientPrivate* d;
};

#endif
