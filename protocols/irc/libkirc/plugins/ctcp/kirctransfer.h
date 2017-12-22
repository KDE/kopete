/*
    kirctransfer.h - DCC Handler

    Copyright (c) 2003-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2003-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRCTRANSFER_H
#define KIRCTRANSFER_H

#include <qdatastream.h>
#include <qfile.h>
#include <qhostaddress.h>
#include <qobject.h>
#include <q3textstream.h>

class QFile;
class QTextCodec;

namespace KIrc {
class Engine;

class Transfer : public QObject
{
    Q_OBJECT

public:
    enum Type {
        Unknown,
        Chat,
        FileOutgoing,
        FileIncoming
    };

    enum Status {
        Error_NoSocket = -2,
        Error = -1,
        Idle = 0,
        HostLookup,
        Connecting,
        Connected,
        Closed
    };
public:
    Transfer(KIrc::Engine *engine, QString nick,   // QString nick_peer_adress
             Type type = Unknown, QObject *parent = 0L, const char *name = 0L);

    Transfer(KIrc::Engine *engine, QString nick,   // QString nick_peer_adress,
             QHostAddress peer_address, quint16 peer_port, Transfer::Type type, QObject *parent = 0L, const char *name = 0L);

    Transfer(KIrc::Engine *engine, QString nick,   // QString nick_peer_adress,
             Transfer::Type type, QString fileName, quint32 fileSize, QObject *parent = 0L, const char *name = 0L);

    Transfer(KIrc::Engine *engine, QString nick,   // QString nick_peer_adress,
             QHostAddress peer_address, quint16 peer_port, Transfer::Type type, QString fileName, quint32 fileSize, QObject *parent = 0L, const char *name = 0L);
/*
    For a file transfer properties are:

        KExntendedSocket	*socket
    or
        QHostAddress		peerAddress
        quint16		peerPort
    for determining the socket.

        QString			fileName
        quint32		fileSize
    for detemining the file propeties.
*//*
    Transfer(	KIrc *engine, QString nick,// QString nick_peer_adress,
            Transfer::Type type, QVariant properties,
            QObject *parent = 0L, const char *name = 0L );
*/
    ~Transfer();

    KIrc::Engine *engine() const
    {
        return m_engine;
    }

    QString nick() const
    {
        return m_nick;
    }

    Type type() const
    {
        return m_type;
    }

    Status status() const;

    /* Start the transfer.
     * If not connected connect to client.
     * Allow receiving/emitting data.
     */
    bool initiate();

    QString fileName() const
    {
        return m_fileName;
    }

    /* Change the file name.
     */
    void setFileName(QString fileName)
    {
        m_fileName = fileName;
    }

    unsigned long fileSize() const
    {
        return m_fileSize;
    }

public slots:
//	bool setSocket( KExtendedSocket *socket );
    void closeSocket();

    void setCodec(QTextCodec *codec);
    void writeLine(const QString &msg);

    void flush();

    void userAbort(QString);

signals:
    void readLine(const QString &msg);

    void fileSizeCurrent(unsigned int);
    void fileSizeAcknowledge(unsigned int);

//	void received(quint32);
//	void sent(quint32);

    void abort(QString);

    /* Emitted when the transfer is complete.
     * Usually it means that the file transfer has successfully finished.
     */
    void complete();

protected slots:
    void slotError(int);

    void readyReadLine();

    void readyReadFileIncoming();

    void writeFileOutgoing();
    void readyReadFileOutgoing();

protected:
//	void emitSignals();
    void checkFileTransferEnd(quint32 fileSizeAck);

    KIrc::Engine *m_engine;
    QString m_nick;

    Type m_type;
//	KExtendedSocket *m_socket;
    bool m_initiated;

    // Text member data
    Q3TextStream m_socket_textStream;
//	QTextCodec *	m_socket_codec;

    // File member data
    QFile m_file;
    QString m_fileName;
    quint32 m_fileSize;
    quint32 /*usize_t*/ m_fileSizeCur;
    quint32 /*usize_t*/ m_fileSizeAck;
    QDataStream m_socketDataStream;
    char m_buffer[1024];
    int m_bufferLength;

    // Data transfer measures
    quint32 m_receivedBytes;
    quint32 m_receivedBytesLimit;

    quint32 m_sentBytes;
    quint32 m_sentBytesLimit;
};
}

#endif
