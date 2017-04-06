#include "networkstatuscommon.h"
#include "libkopete_debug.h"

QDataStream & operator<<(QDataStream &s, const NetworkStatus::Properties p)
{
    qCDebug(LIBKOPETE_LOG) << "status is: " << (int)p.status;
    s << (int)p.status;
    s << (int)p.onDemandPolicy;
    s << p.service;
    s << (p.internet ? 1 : 0);
    s << p.netmasks;
    return s;
}

QDataStream &operator>>(QDataStream &s, NetworkStatus::Properties &p)
{
    int status, onDemandPolicy, internet;
    s >> status;
    qCDebug(LIBKOPETE_LOG) << "status is: " << status;
    p.status = (NetworkStatus::EnumStatus)status;
    s >> onDemandPolicy;
    p.onDemandPolicy = (NetworkStatus::EnumOnDemandPolicy)onDemandPolicy;
    s >> p.service;
    s >> internet;
    if (internet) {
        p.internet = true;
    } else {
        p.internet = false;
    }
    s >> p.netmasks;
    qCDebug(LIBKOPETE_LOG) << "enum converted status is: " << p.status;
    return s;
}
