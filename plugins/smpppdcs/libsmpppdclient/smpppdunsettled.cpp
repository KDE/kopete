/*
	smpppdunsettled.cpp
 
	Copyright (c) 2006      by Heiko Schaefer        <heiko@rangun.de>
 
	Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>
 
	*************************************************************************
	*                                                                       *
	* This program is free software; you can redistribute it and/or modify  *
	* it under the terms of the GNU General Public License as published by  *
	* the Free Software Foundation; version 2 of the License.               *
	*                                                                       *
	*************************************************************************
*/

#include <cstdlib>
#include <openssl/md5.h>

#include <qregexp.h>

#include <kdebug.h>
#include <kstreamsocket.h>

#include "smpppdready.h"
#include "smpppdunsettled.h"

using namespace SMPPPD;

Unsettled * Unsettled::m_instance = NULL;

Unsettled::Unsettled() {}

Unsettled::~Unsettled() {}

Unsettled * Unsettled::instance() {
    if(!m_instance) {
        m_instance = new Unsettled();
    }

    return m_instance;
}

bool Unsettled::connect(Client * client, const QString& server, uint port) {
	if(!socket(client) ||
		socket(client)->state() != KNetwork::KStreamSocket::Connected ||
		socket(client)->state() != KNetwork::KStreamSocket::Connecting) {

        QString resolvedServer = server;

		changeState(client, Ready::instance());
        disconnect(client);

        // since a lookup on a non-existant host can take a lot of time we
        // try to get the IP of server before and we do the lookup ourself
        KNetwork::KResolver resolver(server);
        resolver.start();
        if(resolver.wait(500)) {
            KNetwork::KResolverResults results = resolver.results();
            if(!results.empty()) {
                QString ip = results[0].address().asInet().ipAddress().toString();
                kdDebug(14312) << k_funcinfo << "Found IP-Address for " << server << ": " << ip << endl;
                resolvedServer = ip;
            } else {
                kdWarning(14312) << k_funcinfo << "No IP-Address found for " << server << endl;
                return false;
            }
        } else {
            kdWarning(14312) << k_funcinfo << "Looking up hostname timed out, consider to use IP or correct host" << endl;
            return false;
        }

		setSocket(client, new KNetwork::KStreamSocket(resolvedServer, QString::number(port)));
		socket(client)->setBlocking(TRUE);

		if(!socket(client)->connect()) {
			kdDebug(14312) << k_funcinfo << "Socket Error: " << KNetwork::KStreamSocket::errorString(socket(client)->error()) << endl;
        } else {
            kdDebug(14312) << k_funcinfo << "Successfully connected to smpppd \"" << server << ":" << port << "\"" << endl;

            static QString verRex = "^SuSE Meta pppd \\(smpppd\\), Version (.*)$";
            static QString clgRex = "^challenge = (.*)$";

            QRegExp ver(verRex);
            QRegExp clg(clgRex);

            QString response = read(client)[0];

            if(response != QString::null &&
                    ver.exactMatch(response)) {
				setServerID(client, response);
				setServerVersion(client, ver.cap(1));
                changeState(client, Ready::instance());
                return true;
            } else if(response != QString::null &&
                      clg.exactMatch(response)) {
				if(password(client) != QString::null) {
                    // we are challenged, ok, respond
					write(client, QString("response = %1\n").arg(make_response(clg.cap(1).stripWhiteSpace(), password(client))).latin1());
                    response = read(client)[0];
                    if(ver.exactMatch(response)) {
						setServerID(client, response);
						setServerVersion(client, ver.cap(1));
                        return true;
                    } else {
                        kdWarning(14312) << k_funcinfo << "SMPPPD responded: " << response << endl;
						changeState(client, Ready::instance());
                        disconnect(client);
                    }
                } else {
                    kdWarning(14312) << k_funcinfo << "SMPPPD requested a challenge, but no password was supplied!" << endl;
					changeState(client, Ready::instance());
                    disconnect(client);
                }
            }
        }
    }

    return false;
}

QString Unsettled::make_response(const QString& chex, const QString& password) const {

	int size = chex.length ();
	if (size & 1)
		return "error";
	size >>= 1;

    // convert challenge from hex to bin
	QString cbin;
	for (int i = 0; i < size; i++) {
		QString tmp = chex.mid (2 * i, 2);
		cbin.append ((char) strtol (tmp.ascii (), 0, 16));
	}

    // calculate response
	unsigned char rbin[MD5_DIGEST_LENGTH];
	MD5state_st md5;
	MD5_Init (&md5);
	MD5_Update (&md5, cbin.ascii (), size);
	MD5_Update (&md5, password.ascii(), password.length ());
	MD5_Final (rbin, &md5);

    // convert response from bin to hex
	QString rhex;
	for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
		char buffer[3];
		snprintf (buffer, 3, "%02x", rbin[i]);
		rhex.append (buffer);
	}

	return rhex;
}
