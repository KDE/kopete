/*
 * sm.cpp - XMPP Stream Management protocol
 * Copyright (C) 2016  Aleksey Andreev
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

#ifdef IRIS_SM_DEBUG
#include <QDebug>
#endif


#include "sm.h"

using namespace XMPP;

SMState::SMState()
{
	reset();
}

void SMState::reset()
{
	enabled = false;
	received_count = 0;
	server_last_handled = 0;
	send_queue.clear();
	resumption_id.clear();
	resumption_location.host.clear();
	resumption_location.port = 0;
}


StreamManagement::StreamManagement(QObject *parent)
	: QObject(parent)
	, sm_started(false)
	, sm_resumed(false)
	, sm_stanzas_notify(0)
	, sm_resend_pos(0)
{
}

void StreamManagement::reset()
{
	sm_started = false;
	sm_resumed = false;
	sm_stanzas_notify = 0;
	sm_resend_pos = 0;
	sm_timeout_data.pause_mode = true;
	sm_timeout_data.point_time = QDateTime();
	//state_.reset();
}

void StreamManagement::started(const QString &resumption_id)
{
	reset();
	state_.resumption_id = resumption_id;
	sm_started = true;
}

void StreamManagement::resumed(quint32 last_handled)
{
	sm_resumed = true;
	sm_resend_pos = 0;
	processAcknowledgement(last_handled);
}

void StreamManagement::setLocation(const QString &host, int port)
{
	state_.resumption_location.host = host;
	state_.resumption_location.port = port;
}

int StreamManagement::lastAckElapsed() const
{
	if (sm_timeout_data.point_time.isNull())
		return 0;
	return sm_timeout_data.point_time.secsTo(QDateTime::currentDateTime());
}

int StreamManagement::takeAckedCount()
{
	int cnt = sm_stanzas_notify;
	sm_stanzas_notify = 0;
	return cnt;
}

QDomElement StreamManagement::getUnacknowledgedStanza()
{
	if (sm_resend_pos < state_.send_queue.size())
		return state_.send_queue.at(sm_resend_pos++);
	return QDomElement();
}

int StreamManagement::addUnacknowledgedStanza(const QDomElement &e)
{
	state_.send_queue.enqueue(e);
	int len = state_.send_queue.length();
#ifdef IRIS_SM_DEBUG
	qDebug() << "Stream Management: [INF] Send queue length is changed: " << len;
#endif
	return len;
}

void StreamManagement::processAcknowledgement(quint32 last_handled)
{
	sm_timeout_data.pause_mode = true;
	sm_timeout_data.point_time = QDateTime::currentDateTime();
#ifdef IRIS_SM_DEBUG
	bool f = false;
#endif
	while (!state_.send_queue.isEmpty() && state_.server_last_handled != last_handled) {
		state_.send_queue.dequeue();
		++state_.server_last_handled;
		++sm_stanzas_notify;
#ifdef IRIS_SM_DEBUG
		f = true;
#endif
	}
#ifdef IRIS_SM_DEBUG
	if (f) {
		qDebug() << "Stream Management: [INF] Send queue length is changed: " << state_.send_queue.length();
		if (state_.send_queue.isEmpty() && last_handled != state_.server_last_handled)
			qDebug() << "Stream Management: [ERR] Send queue is empty but last_handled != server_last_handled " << last_handled << state_.server_last_handled;
	}
#endif
}

bool StreamManagement::processNormalStanza(const QDomElement &e)
{
	Q_UNUSED(e)
	if (!sm_timeout_data.pause_mode) {
		sm_timeout_data.point_time = QDateTime::currentDateTime();
		return true;
	}
	return false;
}

void StreamManagement::markStanzaHandled()
{
	++state_.received_count;
#ifdef IRIS_SM_DEBUG
	qDebug() << "Stream Management: [INF] current received id: " << state_.received_count;
#endif
}

QDomElement StreamManagement::generateRequestStanza(QDomDocument &doc)
{
	if (sm_timeout_data.pause_mode) {
#ifdef IRIS_SM_DEBUG
		qDebug() << "Stream Management: [?->] Sending request of acknowledgment to server";
#endif
		sm_timeout_data.pause_mode = false;
		sm_timeout_data.point_time = QDateTime::currentDateTime();
		return doc.createElementNS(NS_STREAM_MANAGEMENT, "r");
	}
	return QDomElement();
}

QDomElement StreamManagement::makeResponseStanza(QDomDocument &doc)
{
#ifdef IRIS_SM_DEBUG
	qDebug() << "Stream Management: [-->] Sending acknowledgment with h =" << state_.received_count;
#endif
	QDomElement e = doc.createElementNS(NS_STREAM_MANAGEMENT, "a");
	e.setAttribute("h", state_.received_count);
	return e;
}
