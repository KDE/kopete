#ifndef NETWORKSTATUS_COMMON_H
#define NETWORKSTATUS_COMMON_H

#include <qstringlist.h>

namespace NetworkStatus
{
	enum EnumStatus { NoNetworks = 1, Unreachable, OfflineDisconnected,  OfflineFailed, ShuttingDown, Offline, Establishing, Online };
	enum EnumRequestResult { RequestAccepted = 1, Connected, UserRefused, Unavailable };
	enum EnumOnDemandPolicy { All, User, None, Permanent };
	struct Properties
	{
		QString name;
		// status of the network
		EnumStatus status;
		// policy for on-demand usage as defined by the service
		EnumOnDemandPolicy onDemandPolicy;
		// identifier for the service
		QCString service;
		// indicate that the connection is to 'the internet' - similar to default gateway in routing
		bool internet;
		// list of netmasks that the network connects to - overridden by above internet
		QStringList netmasks;
		// for future expansion consider
		// EnumChargingModel - FlatRate, TimeCharge, VolumeCharged
		// EnumLinkStatus - for WLANs - VPOOR, POOR, AVERAGE, GOOD, EXCELLENT
	};
};

QDataStream & operator>> ( QDataStream & s, NetworkStatus::Properties &p );
QDataStream & operator<< ( QDataStream & s, const NetworkStatus::Properties p );

#endif
