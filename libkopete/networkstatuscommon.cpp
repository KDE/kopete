#include "networkstatuscommon.h"
#include <kdebug.h>

QDataStream & operator<< ( QDataStream & s, const NetworkStatus::Properties p )
{
	kdDebug() << k_funcinfo << "status is: " << (int)p.status << endl;
	s << (int)p.status;
	s << (int)p.onDemandPolicy;
	s << p.service;
	s << ( p.internet ? 1 : 0 );
	s << p.netmasks;
	return s;
}

QDataStream & operator>> ( QDataStream & s, NetworkStatus::Properties &p )
{
	int status, onDemandPolicy, internet;
	s >> status;
	kdDebug() << k_funcinfo << "status is: " << status << endl;
	p.status = ( NetworkStatus::EnumStatus )status;
	s >> onDemandPolicy;
	p.onDemandPolicy = ( NetworkStatus::EnumOnDemandPolicy )onDemandPolicy;
	s >> p.service;
	s >> internet;
	if ( internet )
		p.internet = true;
	else
		p.internet = false;
	s >> p.netmasks;
	kdDebug() << k_funcinfo << "enum converted status is: " << p.status << endl;
	return s;
}
