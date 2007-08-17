#ifndef KDED_NETWORKSTATUS_CLIENTIFACE_H
#define KDED_NETWORKSTATUS_CLIENTIFACE_H

#include "networkstatuscommon.h"

#include <dcopobject.h>

class ClientIface : virtual public DCOPObject
{
K_DCOP
k_dcop:
	/** Get the set of networks that the daemon is aware of.  Mostly for debug */
	virtual QStringList networks() = 0;
	/**
	 * Get the status of the connection to the given host.
	 * @param host 
	 * @return a NetworkStatus::EnumStatus representing the state of the connection to the given host
	 */
	virtual int status( const QString & host = QString() ) = 0;
	/**
	 * Request a connection to the named host, registering the application's usage of this connection
	 * @param host The hostname the client wants to connect to.
	 * @param userInitiated Indicates whether the connection is a direct result of a user action or is a background task.  Used by the daemon to decide whether to create an on-demand connection.
	 * @return An NetworkStatus::EnumRequestResult indicating whether the request was accepted
	 */
	virtual int request( const QString &  host, bool userInitiated ) = 0;
	/**
	 * Indicate that a previously registered connection to the given host is no longer needed by this client
	 * @param host The hostname being relinquished.
	 */
	virtual void relinquish( const QString & host ) = 0;
	/**
	 * Indicate that a communication failure has occurred for a given host
	 * @param host The hostname for which the failure occurred.
	 * @return True indicates the caller should try again to lookup the host, as the daemon has another IP address available.
	 */
	virtual bool reportFailure( const QString &  host ) = 0;
	/**
	 * Utility method to check the daemon's status
	 */
k_dcop_signals:
	/**
	 * A status change occurred for the network(s) used to connect to the given host.
	 * @param host The host which the application has indicated it is using
	 * @param status The new status of the network used to reach host.
	 */
	void statusChange( QString host, int status );
	/**
	 * The network would like to shut down - any clients using this host are to finish using it immediately and call 
	 * relinquish() when done.
	 * @param host The host, registered as in use by applications, which is about to be disconnected.
	 */
	void shutdownRequested( QString host );
};

#endif
