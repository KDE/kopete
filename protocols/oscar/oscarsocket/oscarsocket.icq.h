
#ifndef OSCARSOCKET_ICQ_H
#define OSCARSOCKET_ICQ_H

const QString ICQ_SERVER = "login.icq.com";
const unsigned int ICQ_PORT = 5190;

// Internal status for the ICQ protocol
// some are dupliated because setting and getting the status
// do not use the same value
const unsigned long ICQ_STATUS_OFFLINE			= 0xFFFFFFFF;
const unsigned long ICQ_STATUS_ONLINE			= 0x00000000;

const unsigned long ICQ_STATUS_IS_INVIS		= 0x00000100;
const unsigned long ICQ_STATUS_IS_DND			= 0x00000002;
const unsigned long ICQ_STATUS_IS_OCC			= 0x00000010;
const unsigned long ICQ_STATUS_IS_NA			= 0x00000004;
const unsigned long ICQ_STATUS_IS_AWAY			= 0x00000001;
const unsigned long ICQ_STATUS_IS_FFC			= 0x00000020;

const unsigned long ICQ_STATUS_SET_INVIS		= 0x00000100;
const unsigned long ICQ_STATUS_SET_DND			= 0x00000013;
const unsigned long ICQ_STATUS_SET_OCC			= 0x00000011;
const unsigned long ICQ_STATUS_SET_NA			= 0x00000005;
const unsigned long ICQ_STATUS_SET_AWAY		= 0x00000001;
const unsigned long ICQ_STATUS_SET_FFC			= 0x00000020;

const unsigned long ICQ_STATUS_WEBAWARE		= 0x00010000;
const unsigned long ICQ_STATUS_SHOWIP			= 0x00020000;

const unsigned short ICQ_SEARCHSTATE_OFFLINE = 0;
const unsigned short ICQ_SEARCHSTATE_ONLINE = 1;
const unsigned short ICQ_SEARCHSTATE_DISABLED = 2;

const unsigned char ICQ_TCP_VERSION = 0x0008;

#endif
// vim: set noet ts=4 sts=4 sw=4:
