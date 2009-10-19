/*
 * This file is part of Kopete
 *
 * Copyright (C) 2009 Collabora Ltd. <info@collabora.co.uk>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef LIB_KOPETE_TELEPATHY_TELEPATHYFILETRANSFER_H
#define LIB_KOPETE_TELEPATHY_TELEPATHYFILETRANSFER_H

#include <KopeteTelepathy/telepathycontact.h>
#include <TelepathyQt4/Connection>

namespace Tp
{
class PendingOperation;
}

namespace Kopete
{
class Transfer;
}

class TelepathyFileTransfer : public QObject
{
    Q_OBJECT

public:
    TelepathyFileTransfer(Tp::ConnectionPtr connection,
                          TelepathyContact *contact,
                          const QString &fileName);
    virtual ~TelepathyFileTransfer();

private slots:
    void onConnectionReady(Tp::PendingOperation *);
    void onInvalidated();
    void onFileTransferChannelCreated(Tp::PendingOperation*);
    void onFileTransferChannelReady(Tp::PendingOperation*);
    void onFileTransferChannelStateChanged(Tp::FileTransferState,
                                           Tp::FileTransferStateChangeReason);
    void onFileTransferChannelTransferredBytesChanged(qulonglong);
    void onTransferCancelled();
    void onTransferResult();


private:
    Tp::ConnectionPtr m_conn;
    Tp::OutgoingFileTransferChannelPtr m_channel;
    QFile m_localFile;
    Kopete::Transfer *m_transfer;
    TelepathyContact *m_contact;
};

#endif // Header guard

