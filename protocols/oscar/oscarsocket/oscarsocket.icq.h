
#ifndef OSCARSOCKET_ICQ_H
#define OSCARSOCKET_ICQ_H

#define ICQ_SERVER "login.icq.com"
#define ICQ_PORT 5190

//** Internal status for the ICQ protocol **/
const unsigned short ICQ_STATUS_ONLINE		= 0x0000;
const unsigned short ICQ_STATUS_OFFLINE	= 0xFFFF;
const unsigned short ICQ_STATUS_AWAY		= 0x0001;
const unsigned short ICQ_STATUS_DND			= 0x0002;
const unsigned short ICQ_STATUS_NA			= 0x0004;
const unsigned short ICQ_STATUS_OCC			= 0x0010;
const unsigned short ICQ_STATUS_FFC			= 0x0020;

const unsigned short ICQ_SEARCHSTATE_OFFLINE = 0;
const unsigned short ICQ_SEARCHSTATE_ONLINE = 1;
const unsigned short ICQ_SEARCHSTATE_DISABLED = 2;

const unsigned char ICQ_TCP_VERSION = 0x0008;
#endif
// vim: set noet ts=4 sts=4 sw=4:
