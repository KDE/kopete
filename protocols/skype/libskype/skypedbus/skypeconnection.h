//
// C++ Interface: skypeconnection
//
// Description:
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SKYPECONNECTION_H
#define SKYPECONNECTION_H

#include <qobject.h>

typedef enum {
	///The connection was successfull
	seSuccess = 0,
	///No runnign DBUS found
	seNoDBus,
	///No running skype found and launching disabled or did not worked
	seNoSkype,
	///User did not accept this app
	seAuthorization,
	///Some other error
	seUnknown,
	///It was canceled (by disconnectSkype)
	seCanceled
} skypeConnectionError;

///This describes why was the connection closed
typedef enum {
	///It was closed by this application
	crLocalClosed,
	///It was closed by skype (reserverd for future versions of protocol, does not work yet)
	crRemoteClosed,
	///The connection was lost, skype does not respond for the ping command or messages can not be sent
	crLost,
} skypeCloseReason;

class SkypeConnectionPrivate;
namespace DBusQt {
	class Message;
};
class KProcess;

/**
 * This class is classs wrapping DBUS so it can be used easilly to connect to skype, disconnect send and receive messages from it.
 * @author Kopete Developers
*/
class SkypeConnection : public QObject
{
	Q_OBJECT
	private:
		///The D-pointer for internal things
		SkypeConnectionPrivate *d;
	private slots:
		///This one listens for incoming messages
		void gotMessage(const DBusQt::Message &);
		///This one takes care of incoming messages if they have some sence for the connection (protocol, pings and so on)
		void parseMessage(const QString &message);
		///Set environment variables set from dbus-launch command (private DBus session)
		void setEnv(KProcess *, char *buff, int len);
		///Starts logging into skype
		void startLogOn();
		///Another interval try to connect to just started Skype
		void tryConnect();
	public slots:
		/**
		 * Connects to skype
		 * After connection (bosth successfull or unsuccessfull) connectionDone is emited
		 * @see connectionDone
		 * @param start By what command start Skype if it is not running (empty string means nothing is started)
		 * @param appName tells as what application it should authorise itself (this will user see on the "do you want to allow" dialog box)
		 * @param protocolVer Maximal protocol version that this app manages
		 * @param bus 0 - session bus, 1 - system bus
		 * @param startDbus Start session DBUs if needed (etc. not running and session DBus should be used)
		 * @param launchTimeout How long max. should wait to tell that launching skype did not work
		 * @param waitBeforeConnect Do we need to wait a while after skype starts?
		 */
		void connectSkype(const QString &start, const QString &appName, int protocolVer, int bus, bool startDBus, int launchTimeout, int waitBeforeConnect);
		/**
		 * Disconnects from skype
		 * @see connectionClosed
		 */
		void disconnectSkype(skypeCloseReason reason = crLocalClosed);
		/**
		 * Sends a message to skype. You must be connected before !
		 * @param message Contains the message to send
		 */
		void send(const QString &message);
	public:
		/**
		 * Constructor. Creates UNCONECTED connection (sounds oddly ?)
		 */
		SkypeConnection();
		/**
		 * Destructor
		 */
		~SkypeConnection();
		/**
		 * This enables/disables pings to skype and sets interval of the pings and timeout
		 * @param enable Enable or not the pings? If this is false, no pings are done and the rest of parameters has no effect
		 * @param interval Interval in witch pings should be sent, in miliseconds
		 * @param timeout When the ping should be considered unanwsered? (should be shorter than interval), in miliseconds
		 */
		void setPing(bool enable, int interval, int timeout);
		/**
		 * @return Are the pings enabled?
		 */
		bool getPing() const;
		/**
		 * @return What is the interval of pings? (ms)
		 */
		int getPingInterval() const;
		/**
		 * @return What is the timeout od pings? (ms)
		 */
		int getPingTimeout() const;
		/**
		 * @return Are we connected to the skype?
		 */
		bool connected() const;
		/**
		 * @return What is the protocol version?
		 */
		int protocolVer() const;
		/**
		 * This operator makes it possible to just send messages by writing connection << SomeString << anotherString. They are sent as separate objects;
		 * @param message What will be sent
		 */
		SkypeConnection &operator<<(const QString &message);
		/**
		 * This operator sends a message to skype and returns the response from it. Note that this one blocks.
		 * @param message What should be sent to skype
		 * @return The response from skype
		 */
		QString operator%(const QString &message);
	signals:
		/**
		 * This signal is emited when an atempt to connect to skype application is done. It is done in both cases, success or not.
		 * @param error Indicates error code. seSuccess means there was no error and the connection was successfull.
		 * @param protocolVer Protocol version used by this connection. Is less or equal to the version set in connect
		 * @see connect
		 */
		void connectionDone(int error, int protocolVer);
		/**
		 * This signal is emited when the connection is closed due to error or because it was disconnetcted
		 * @param reason Describes why it was closed (you can typecast it to skypeCloseReason if you are interested, or just use the numeric values)
		 */
		void connectionClosed(int reason);
		/**
		 * This slot is emited when something is comming from skype.
		 * It contains pongs as well (responses to ping) and if you do not care about them, you should ignore them.
		 * @param message The message that arrived
		 */
		void received(const QString &message);
		/**
		 * This is emited when some error ocurs
		 * @param message Describes the error
		 */
		void error(const QString &message);
		/**
		 * This is provided for debuging so you can see what you have sent to skype
		 * @param message The message that was sent to skype
		 */
		void sent(const QString &message);
};

#endif
