// *************************************************************************
// * Taken from the GSM TA/ME library
// *
// * File:	  gsm_unix_port.cc
// *
// * Purpose: UNIX serial port implementation with extras
// *
// * Copyright (c)       by Peter Hofmann (Originial Author) <software@pxh.de>
// * Copyright (c)       by Justin Huff (Modifier) <jjhuff@mspin.net>
// *
// * Created: 10.5.1999
// *************************************************************************
#include "kopete_unix_serial.h"
#ifdef INCLUDE_SMSGSM

#include <gsmlib/gsm_util.h>
#include <termios.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <pthread.h>
#include <cassert>
#include <assert.h>

#include <qsocketnotifier.h>

using namespace std;
using namespace gsmlib;

static const int holdoff[] = {2000000, 1000000, 400000};
static const int holdoffArraySize = sizeof(holdoff)/sizeof(int);

// alarm handling for socket read/write
// the timerMtx is necessary since several threads cannot use the
// timer indepently of each other

static pthread_mutex_t timerMtx = PTHREAD_MUTEX_INITIALIZER;
#define pthread_mutex_lock(x)
#define pthread_mutex_unlock(x)

// for non-GNU systems, define alarm()
#ifndef HAVE_ALARM
unsigned int alarm(unsigned int seconds)
{
  struct itimerval old, newt;
  newt.it_interval.tv_usec = 0;
  newt.it_interval.tv_sec = 0;
  newt.it_value.tv_usec = 0;
  newt.it_value.tv_sec = (long int)seconds;
  if (setitimer(ITIMER_REAL, &newt, &old) < 0)
	return 0;
  else
	return old.it_value.tv_sec;
}
#endif

// this routine is called in case of a timeout
static void catchAlarm(int)
{
  // do nothing
}

// start timer
static void startTimer()
{
  pthread_mutex_lock(&timerMtx);
  struct sigaction newAction;
  newAction.sa_handler = catchAlarm;
  newAction.sa_flags = 0;
  sigaction(SIGALRM, &newAction, NULL);
  alarm(1);
}

// reset timer
static void stopTimer()
{
  alarm(0);
  sigaction(SIGALRM, NULL, NULL);
  pthread_mutex_unlock(&timerMtx);
}

// KopeteUnixSerialPort members

void KopeteUnixSerialPort::throwModemException(string message) throw(GsmException)
{
  ostringstream os;
  os << message << " (errno: " << errno << "/" << strerror(errno) << ")";
  throw GsmException(os.str(), OSError, errno);
}

void KopeteUnixSerialPort::putBack(unsigned char c)
{
  assert(_oldChar == -1);
  _oldChar = c;
}

int KopeteUnixSerialPort::readByte() throw(GsmException)
{
  if (_oldChar != -1)
  {
	int result = _oldChar;
	_oldChar = -1;
	return result;
  }

  unsigned char c;
  int timeElapsed = 0;
  struct timeval oneSecond;
  bool readDone = false;

  while (! readDone && timeElapsed < _timeoutVal)
  {
	if (interrupted())
	  throwModemException("interrupted when reading from TA");

	// setup fd_set data structure for select()
	fd_set fdSet;
	oneSecond.tv_sec = 1;
	oneSecond.tv_usec = 0;
	FD_ZERO(&fdSet);
	FD_SET(_fd, &fdSet);

	switch (select(FD_SETSIZE, &fdSet, NULL, NULL, &oneSecond))
	{
	case 1:
	{
	  int res = read(_fd, &c, 1);
	  if (res != 1)
		throwModemException("end of file when reading from TA");
	  else
		readDone = true;
	  break;
	}
	case 0:
	  ++timeElapsed;
	  break;
	default:
	  if (errno != EINTR)
		throwModemException("reading from TA");
	  break;
	}
  }
  if (! readDone)
	throwModemException("timeout when reading from TA");

#ifndef NDEBUG
  if (debugLevel() >= 2)
  {
	// some useful debugging code
	if (c == LF)
	  cerr << "<LF>";
	else if (c == CR)
	  cerr << "<CR>";
	else cerr << "<'" << (char) c << "'>";
	cerr.flush();
  }
#endif
  return c;
}

KopeteUnixSerialPort::KopeteUnixSerialPort(string device, speed_t lineSpeed,
							   string initString, bool swHandshake)
  throw(GsmException) :
  _oldChar(-1), _timeoutVal(TIMEOUT_SECS)
{
	_readNotifier = NULL;

  struct termios t;

  // open device
  _fd = open(device.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (_fd == -1)
	throwModemException("opening device");

  // switch off non-blocking mode
  int fdFlags;
  if ((fdFlags = fcntl(_fd, F_GETFL)) == -1)
  {
	close(_fd);
	throwModemException("getting file status flags failed");
  }
  fdFlags &= ~O_NONBLOCK;
  if (fcntl(_fd, F_SETFL, fdFlags) == -1)
  {
	close(_fd);
	throwModemException("switching of non-blocking mode failed");
  }

	// Set the close on exec flag
	if ((fdFlags = fcntl(_fd, F_GETFD)) == -1)
	{
		close(_fd);
		throwModemException("getting file status flags failed");
	}
	fdFlags |= FD_CLOEXEC;
	if (fcntl(_fd, F_SETFD, fdFlags) == -1)
	{
		close(_fd);
		throwModemException("switching of non-blocking mode failed");
	}

  long int saveTimeoutVal = _timeoutVal;
  _timeoutVal = 3;
  int initTries = holdoffArraySize;
  while (initTries-- > 0)
  {
	// flush all pending output
	tcflush(_fd, TCOFLUSH);

	// toggle DTR to reset modem
	int mctl = TIOCM_DTR;
	if (ioctl(_fd, TIOCMBIC, &mctl) < 0 && errno != ENOTTY)
	{
	  close(_fd);
	  throwModemException("clearing DTR failed");
	}
	// the waiting time for DTR toggling is increased with each loop
	usleep(holdoff[initTries]);
	if (ioctl(_fd, TIOCMBIS, &mctl) < 0 && errno != ENOTTY)
	{
	  close(_fd);
	  throwModemException("setting DTR failed");
	}
	// get line modes
	if (tcgetattr(_fd, &t) < 0)
	{
	  close(_fd);
	  throwModemException("tcgetattr device");
	}

	// set line speed
	cfsetispeed(&t, lineSpeed);
	cfsetospeed(&t, lineSpeed);

	// set the device to a sane state
	t.c_iflag |= IGNPAR | (swHandshake ? IXON | IXOFF : 0);
	t.c_iflag &= ~(INPCK | ISTRIP | IMAXBEL |
				   (swHandshake ? 0 : IXON |  IXOFF)
				   | IXANY | IGNCR | ICRNL | IMAXBEL | INLCR | IGNBRK);
	t.c_oflag &= ~(OPOST);
	// be careful, only touch "known" flags
	t.c_cflag &= ~(CSIZE | CSTOPB | PARENB | PARODD |
				  (swHandshake ? CRTSCTS : 0 ));
	t.c_cflag |= CS8 | CREAD | HUPCL | (swHandshake ? 0 : CRTSCTS) | CLOCAL;
	t.c_lflag &= ~(ECHO | ECHOE | ECHOPRT | ECHOK | ECHOKE | ECHONL |
				   ECHOCTL | ISIG | IEXTEN | TOSTOP | FLUSHO | ICANON);
	t.c_lflag |= NOFLSH;
	t.c_cc[VMIN] = 1;
	t.c_cc[VTIME] = 0;

	t.c_cc[VSUSP] = 0;

	// write back
	if(tcsetattr (_fd, TCSANOW, &t) < 0)
	{
	  close(_fd);
	  throwModemException("tcsetattr device");
	}
	// the waiting time for writing to the ME/TA is increased with each loop
	usleep(holdoff[initTries]);

	// flush all pending input
	tcflush(_fd, TCIFLUSH);

	try
	{
	  // reset modem
	  putLine("ATZ");
	  bool foundOK = false;
	  int readTries = 5;
	  while (readTries-- > 0)
	  {
		// for the first call getLine() waits only 3 seconds
		// because of _timeoutVal = 3
		string s = getLine();
		if (s.find("OK") != string::npos ||
			s.find("CABLE: GSM") != string::npos)
		{
		  foundOK = true;
		  readTries = 0;		// found OK, exit loop
		}
		else if (s.find("ERROR") != string::npos)
		  readTries = 0;		// error, exit loop
	  }

	  // set getLine/putLine timeout back to old value
	  _timeoutVal = saveTimeoutVal;

	  if (foundOK)
	  {
		// init modem
		readTries = 5;
		putLine("AT" + initString);
		while (readTries-- > 0)
		{
		  string s = getLine();
		  if (s.find("OK") != string::npos ||
			  s.find("CABLE: GSM") != string::npos)
		  {
				_readNotifier = new QSocketNotifier(_fd, QSocketNotifier::Read);
					connect( _readNotifier, SIGNAL(activated(int)), this, SIGNAL(activated()));
				return;					// found OK, return
		  }
		}
	  }
	}
	catch (GsmException &e)
	{
	  _timeoutVal = saveTimeoutVal;
	  if (initTries == 0)
	  {
		close(_fd);
		throw e;
	  }
	}
  }
  // no response after 3 tries
  close(_fd);
  throwModemException("reset modem failed");
}

string KopeteUnixSerialPort::getLine() throw(GsmException)
{
  string result;
  int c;
  while ((c = readByte()) >= 0)
  {
	while (c == CR)
	{
	  c = readByte();
	}
	if (c == LF)
	  break;
	result += c;
  }

#ifndef NDEBUG
  if (debugLevel() >= 1)
	cerr << "<-- " << result << endl;
#endif

  return result;
}

void KopeteUnixSerialPort::putLine(string line,
							 bool carriageReturn) throw(GsmException)
{
#ifndef NDEBUG
  if (debugLevel() >= 1)
	cerr << "--> " << line << endl;
#endif

  if (carriageReturn) line += CR;
  const char *l = line.c_str();

  int timeElapsed = 0;
  struct timeval oneSecond;

  ssize_t bytesWritten = 0;
  while (bytesWritten < (ssize_t)line.length() && timeElapsed < _timeoutVal)
  {
	if (interrupted())
	  throwModemException("interrupted when writing to TA");

	// setup fd_set data structure for select()
	fd_set fdSet;
	oneSecond.tv_sec = 1;
	oneSecond.tv_usec = 0;
	FD_ZERO(&fdSet);
	FD_SET(_fd, &fdSet);

	switch (select(FD_SETSIZE, NULL, &fdSet, NULL, &oneSecond))
	{
	case 1:
	{
	  ssize_t bw = write(_fd, l + bytesWritten, line.length() - bytesWritten);
	  if (bw < 0)
		throwModemException("writing to TA");
	  bytesWritten += bw;
	  break;
	}
	case 0:
	  ++timeElapsed;
	  break;
	default:
	  if (errno != EINTR)
		throwModemException("writing to TA");
	  break;
	}
  }

  while (timeElapsed < _timeoutVal)
  {
	if (interrupted())
	  throwModemException("interrupted when writing to TA");
	::startTimer();
	int res = tcdrain(_fd);		// wait for output to be read by TA
	::stopTimer();
	if (res == 0)
	  break;
	else
	{
	  assert(errno == EINTR);
	  ++timeElapsed;
	}
  }
  if (timeElapsed >= _timeoutVal)
	throwModemException("timeout when writing to TA");

  // echo CR LF must be removed by higher layer functions in gsm_at because
  // in order to properly handle unsolicited result codes from the ME/TA
}

bool KopeteUnixSerialPort::wait(GsmTime timeout) throw(GsmException)
{
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(_fd, &fds);
  return select(FD_SETSIZE, &fds, NULL, NULL, timeout) != 0;
}

// set timeout for read or write in seconds.
void KopeteUnixSerialPort::setTimeOut(unsigned int timeout)
{
  _timeoutVal = timeout;
}

KopeteUnixSerialPort::~KopeteUnixSerialPort()
{
	delete _readNotifier;
	_readNotifier = NULL;
	if (_fd != -1)
		close(_fd);
}


#endif
