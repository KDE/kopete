// client.h - main interface to libgroupwise

class Client : public QObject
{
Q_OBJECT

	public:
		Client(QObject *parent=0);
		~Client();
		
		/**
		 * Start a connection to the server using the supplied @ref ClientStream.
		 * This is only a transport layer connection.
		 * Needed for protocol action P1.
		 * @param s initialised client stream to use for the connection
		 * @param server the server to connect to - but this is also set on the connector used to construct the clientstream??
		 * @param auth needed for jabber protocol layer only?
		 */
		void connectToServer(ClientStream *s, const QString &server, bool auth=true);
		
		/**
		 * Login to the GroupWise server using the supplied credentials
		 * Protocol action P1, needed for all
		 * @param host - probably could obtain this back from the connector - used for outgoing tasks to determine destination
		 * @param user user name to log in as
		 * @param password 
		 */ 
		void start(const QString &host, const QString &user, const QString &pass );
		
		

};