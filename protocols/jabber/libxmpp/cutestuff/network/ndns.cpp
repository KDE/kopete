/*
 * ndns.cpp - native DNS resolution
 * Copyright (C) 2001, 2002  Justin Karneges
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

//! \class NDns ndns.h
//! \brief Simple DNS resolution using native system calls
//!
//! This class is to be used when Qt's QDns is not good enough.  Because QDns
//! does not use threads, it cannot make a system call asyncronously.  Thus,
//! QDns tries to imitate the behavior of each platform's native behavior, and
//! generally falls short.
//!
//! NDns uses a thread to make the system call happen in the background.  This
//! gives your program native DNS behavior, at the cost of requiring threads
//! to build.
//!
//! \code
//! #include "ndns.h"
//!
//! ...
//!
//! NDns dns;
//! dns.resolve("psi.affinix.com");
//!
//! // The class will emit the resultsReady() signal when the resolution
//! // is finished. You may then retrieve the results:
//!
//! uint ip_address = dns.result();
//!
//! // or if you want to get the IP address as a string:
//!
//! QString ip_address = dns.resultString();
//! \endcode

#include <qapplication.h>
#include "ndns.h"

#ifdef Q_OS_UNIX
#include<netdb.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#endif

#ifdef Q_WS_WIN
#include<windows.h>
#endif


//! \if _hide_doc_
class NDnsWorkerEvent : public QEvent
{
public:
	NDnsWorkerEvent(NDnsWorker *);

	NDnsWorker *worker() const;

private:
	NDnsWorker *p;
};

class NDnsWorker : public QThread
{
public:
	NDnsWorker(QObject *, const QCString &);

	bool success;
	bool cancelled;
	uint addr;
	QString addrString;

protected:
	void run();

private:
	QCString host;
	QObject *par;
};
//! \endif

static QMutex workerCancelled;

//----------------------------------------------------------------------------
// NDns
//----------------------------------------------------------------------------

//! \fn void NDns::resultsReady()
//! This signal is emitted when the DNS resolution succeeds or fails.

//!
//! Constructs an NDns object with parent \a parent.
NDns::NDns(QObject *parent)
:QObject(parent)
{
	v_result = 0;
	v_resultString = "";
	worker = 0;
}

//!
//! Destroys the object and frees allocated resources.
NDns::~NDns()
{
}

//!
//! Resolves hostname \a host (eg. psi.affinix.com)
void NDns::resolve(const QString &host)
{
	if(worker)
		return;

	worker = new NDnsWorker(this, host.latin1());
	worker->start();
}

//!
//! Cancels the lookup action.
//! \note This will not stop the underlying system call, which must finish before the next lookup will proceed.
void NDns::stop()
{
	if ( worker ) {
		workerCancelled.lock();
		worker->cancelled = true;
		workerCancelled.unlock();
	}

	worker = 0;
}

//!
//! Returns the IP address as a 32-bit integer in host-byte-order.  This will be 0 if the lookup failed.
//! \sa resultsReady()
uint NDns::result() const
{
	return v_result;
}

//!
//! Returns the IP address as a string.  This will be an empty string if the lookup failed.
//! \sa resultsReady()
QString NDns::resultString() const
{
	return v_resultString;
}

//!
//! Returns TRUE if busy resolving a hostname.
bool NDns::isBusy() const
{
	return worker ? true: false;
}

bool NDns::event(QEvent *e)
{
	if(e->type() == QEvent::User) {
		NDnsWorkerEvent *we = (NDnsWorkerEvent *)e;
		we->worker()->wait(); // ensure that the thread is terminated

		if ( worker == we->worker() ) {
			if(worker->success) {
				v_result = worker->addr;
				v_resultString = worker->addrString;
			}
			else {
				v_result = 0;
				v_resultString = "";
			}

			emit resultsReady();
		}

		delete we->worker();
		worker = 0;

		return true;
	}
	return false;
}

//----------------------------------------------------------------------------
// NDnsWorkerEvent
//----------------------------------------------------------------------------

NDnsWorkerEvent::NDnsWorkerEvent(NDnsWorker *_p)
:QEvent(QEvent::User)
{
	p = _p;
}

NDnsWorker *NDnsWorkerEvent::worker() const
{
	return p;
}

//----------------------------------------------------------------------------
// NDnsWorker
//----------------------------------------------------------------------------

NDnsWorker::NDnsWorker(QObject *_par, const QCString &_host)
{
	success = cancelled = false;
	par = _par;
	host = _host;
}

static QMutex wm;

void NDnsWorker::run()
{
	// lock during the gethostbyname call (anything that returns data into a static buffer is obviously not thread-safe)
	QMutexLocker locker( &wm );

	workerCancelled.lock();
	bool cancel = cancelled;
	workerCancelled.unlock();

	if ( !cancel ) {
		//qWarning("resolving [%s]", host.data());
		hostent *h;
		h = gethostbyname(host.data());
		//qWarning("done.");
		if(!h) {
			//qWarning("error");
			success = false;
			QApplication::postEvent(par, new NDnsWorkerEvent(this));
			return;
		}

		in_addr a = *((struct in_addr *)h->h_addr_list[0]);
		addr = ntohl(a.s_addr);
		addrString = inet_ntoa(a);

		//qWarning("success: [%s]", addrString.latin1());
		success = true;
	}
	else {
		//qWarning("cancelled.");
		success = false;
	}

	QApplication::postEvent(par, new NDnsWorkerEvent(this));
}
