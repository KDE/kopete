// filetransfertask.h

// Copyright (C)  2006

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

#ifndef FILETRANSFERTASK_H
#define FILETRANSFERTASK_H

#include <qfile.h>
#include "task.h"

class Transfer;

class FileTransferTask : public Task
{
Q_OBJECT
public:
	FileTransferTask( Task* parent );
	/** create an outgoing filetransfer */
	FileTransferTask( Task* parent, const QString& contact, const QString &fileName );

	//! Task implementation
	void onGo();
	bool forMe( const Transfer* transfer );
	bool take( Transfer* transfer );

private:
	void sendFile();
	TLV makeRendezvousRequest( QByteArray cookie );
	enum Action { Send, Receive };
	Action m_action;
	QFile m_localFile;
	QString m_contact;
	DWORD m_seq;
};

#endif
