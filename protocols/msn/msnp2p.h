/*
    msnp2p.h - msn p2p protocol

    Copyright (c) 2003 by Olivier Goffart        <ogoffart@tiscalinet.be>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef MSNP2P_H
#define MSNP2P_H

#include <qobject.h>
#include <qstrlist.h>


class KopeteMessage;
class MSNAccount;

class KTempFile;

/**
 * @author Olivier Goffart
 */
class MSNP2P : public QObject
{
	Q_OBJECT

public:
	/**
	 * Contructor: id is the KopeteMessageMangager's id
	 */
	MSNP2P(  QObject *parent=0L , const char *name=0L);
	~MSNP2P();

public slots:
	void slotReadMessage( const QByteArray &msg );

	void requestDisplayPicture( const QString &myHandle, const QString &msgHandle, QString msnObject);

signals:
	void sendCommand( const QString &cmd, const QString &args = QString::null,
		bool addId = true, const QByteArray &body = QByteArray() , bool binary=false );

private:
	KTempFile *m_file;
	unsigned int m_Tsize;

	QString m_myHandle;
	QString m_msgHandle;


};

#endif

