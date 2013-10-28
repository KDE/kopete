// *************************************************************************
// * Taken from the GSM TA/ME library
// *
// * File:    gsm_unix_port.h
// *
// * Purpose: UNIX serial port implementation with extras
// *
// * Original Author:  Peter Hofmann (software@pxh.de)
// * Modified by: Justin Huff (jjhuff@mspin.net)
// *
// * Created: 4.5.1999
// *************************************************************************

#ifndef KOPETE_UNIX_SERIAL_H
#define KOPETE_UNIX_SERIAL_H

#ifdef INCLUDE_SMSGSM

#include <string>
#include <gsmlib/gsm_error.h>
#include <gsmlib/gsm_port.h>
#include <gsmlib/gsm_util.h>
#include <sys/types.h>
#include <termios.h>

#include <qobject.h>

class QSocketNotifier;
namespace gsmlib
{
	
class KopeteUnixSerialPort : public QObject, public Port
{
	Q_OBJECT;
	
protected:
	int _fd;                    // file descriptor for device
	int _oldChar;               // character set by putBack() (-1 == none)
	long int _timeoutVal;       // timeout for getLine/readByte

	QSocketNotifier* _readNotifier;
	
	// throw GsmException include UNIX errno
	void throwModemException(std::string message) throw(GsmException);

public:
	// create Port given the UNIX device name
	explicit KopeteUnixSerialPort(std::string device, speed_t lineSpeed = DEFAULT_BAUD_RATE,
				   std::string initString = DEFAULT_INIT_STRING,
				   bool swHandshake = false)
	  throw(GsmException);
	virtual ~KopeteUnixSerialPort();

	// inherited from Port
	void putBack(unsigned char c);
	int readByte() throw(GsmException);
	std::string getLine() throw(GsmException);
	void putLine(std::string line,
						 bool carriageReturn = true) throw(GsmException);
	bool wait(GsmTime timeout) throw(GsmException);
	void setTimeOut(unsigned int timeout);

signals:
	void activated();
};

}
#endif

#endif // KOPETE_UNIX_SERIAL_H
