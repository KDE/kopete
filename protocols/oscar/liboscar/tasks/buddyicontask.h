// buddyicontask.h

// Copyright (C)  2005  Matt Rogers <mattr@kde.org>

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without fdeven the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301  USA

#ifndef BUDDYICONTASK_H
#define BUDDYICONTASK_H

#include "task.h"

class Transfer;

class BuddyIconTask : public Task
{
Q_OBJECT
public:
	BuddyIconTask( Task* parent );

	void uploadIcon( Oscar::WORD length, const QByteArray& data );
	void setReferenceNum( Oscar::WORD num );

	void requestIconFor( const QString& user );
	void setHash( const QByteArray& md5Hash );
	void setIconType( Oscar::WORD iconType );
	void setHashType( Oscar::BYTE type );

	//! Task implementation
	void onGo();
	bool forMe( const Transfer* transfer ) const;
	bool take( Transfer* transfer );

signals:
	void haveIcon( const QString&, QByteArray );

private:
	void sendIcon();
	void handleUploadResponse();
	void sendAIMBuddyIconRequest();
	void handleAIMBuddyIconResponse();
	void sendICQBuddyIconRequest();
	void handleICQBuddyIconResponse();

private:
	enum Action { Send, Receive };
	Action m_action;
	Oscar::WORD m_iconLength;
	int m_refNum;
	QByteArray m_icon;
	QString m_user;
	QByteArray m_hash;
	Oscar::WORD m_iconType;
	Oscar::BYTE m_hashType;
	Oscar::DWORD m_seq;
};

#endif
