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

#include <kopetetransfermanager.h>

#include <KopeteTelepathy/telepathyfiletransfer.h>

#include <KopeteTelepathy/telepathyaccount.h>
#include <KopeteTelepathy/telepathychatsession.h>
#include <KopeteTelepathy/telepathyprotocolinternal.h>

#include <KDebug>

#include <TelepathyQt4/Constants>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/PendingOperation>
#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/PendingChannel>
#include <TelepathyQt4/FileTransferChannel>
#include <TelepathyQt4/OutgoingFileTransferChannel>


TelepathyFileTransfer::TelepathyFileTransfer(Tp::ConnectionPtr connection,
                                             TelepathyContact *contact,
                                             const QString &fileName)
    : QObject(contact),
      m_conn(connection),
      m_localFile(fileName),
      m_transfer(0),
      m_contact(contact)
{
    kDebug() << "Starting new outgoing file transfer:"
             << contact->contactId() << fileName;

    QObject::connect(m_conn.data(),
                     SIGNAL(invalidated(Tp::DBusProxy *,
                            const QString &, const QString &)),
                     SLOT(onInvalidated()));

    QObject::connect(m_conn->becomeReady(),
                     SIGNAL(finished(Tp::PendingOperation *)),
                     SLOT(onConnectionReady(Tp::PendingOperation *)));
}

TelepathyFileTransfer::~TelepathyFileTransfer()
{
    kDebug() << "Destroying file transfer";

    m_localFile.close();
}

void TelepathyFileTransfer::onConnectionReady(Tp::PendingOperation *op)
{
    if (op->isError()) {
        kWarning() << "Readying connection failed:"
                   << op->errorName()
                   << op->errorMessage();
        /* FIXME - We can't just return, so deleting */
        deleteLater();
        return;
    }

    kDebug() << "Telepathy connection is ready for file transfer";

    if (!m_localFile.open(QIODevice::ReadOnly)) {
        kWarning() << "Unable to open file for reading";
        deleteLater();
        return;
    }

    QFileInfo fileInfo(m_localFile);

    kDebug() << "Creating file transfer channel...";

    QVariantMap req;
    req.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".ChannelType"),
               TELEPATHY_INTERFACE_CHANNEL_TYPE_FILE_TRANSFER);
    req.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType"),
               (uint) Tp::HandleTypeContact);
    req.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandle"),
               m_contact->internalContact()->handle()[0]);
    req.insert(QLatin1String(
                TELEPATHY_INTERFACE_CHANNEL_TYPE_FILE_TRANSFER ".Filename"),
               fileInfo.fileName());
    req.insert(QLatin1String(
                TELEPATHY_INTERFACE_CHANNEL_TYPE_FILE_TRANSFER ".Size"),
               (qulonglong) fileInfo.size());
    req.insert(QLatin1String(
                TELEPATHY_INTERFACE_CHANNEL_TYPE_FILE_TRANSFER ".ContentType"),
               "application/octet-stream");

    kDebug() << "Request:" << req;

    connect(m_conn->createChannel(req),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onFileTransferChannelCreated(Tp::PendingOperation*)));
}

void TelepathyFileTransfer::onInvalidated()
{
    kDebug() << "Invalidated";

    deleteLater();
}

void TelepathyFileTransfer::onFileTransferChannelCreated(Tp::PendingOperation *op)
{
    if (op->isError()) {
        kWarning() << "Creating file transfer channel failed:"
                   << op->errorName()
                   << op->errorMessage();
        /* FIXME - We can't just return, so deleting */
        deleteLater();
        return;
    }

    kDebug() << "Telepathy file transfer channel created";

    Tp::PendingChannel *pc = qobject_cast<Tp::PendingChannel*>(op);
    m_channel = Tp::OutgoingFileTransferChannelPtr::dynamicCast(pc->channel());

    m_transfer = Kopete::TransferManager::transferManager()->
        addTransfer(m_contact,
                    m_localFile.fileName(),
                    m_localFile.size(),
                    m_contact->contactId(),
                    Kopete::FileTransferInfo::Outgoing);

    QObject::connect(m_transfer,
                     SIGNAL(transferCancelled()),
                     SLOT(onTransferCancelled()));

    QObject::connect(m_transfer,
                     SIGNAL(result(KJob *)),
                     SLOT(onTransferResult()));

    QObject::connect(m_channel.data(),
                     SIGNAL(invalidated(Tp::DBusProxy *,
                            const QString &, const QString &)),
                     SLOT(onInvalidated()));

    QObject::connect(m_channel->becomeReady(Tp::FileTransferChannel::FeatureCore),
                     SIGNAL(finished(Tp::PendingOperation *)),
                     SLOT(onFileTransferChannelReady(Tp::PendingOperation *)));
}

void TelepathyFileTransfer::onFileTransferChannelReady(Tp::PendingOperation *op)
{
    if (op->isError()) {
        kWarning() << "Readying file transfer channel failed:"
                   << op->errorName()
                   << op->errorMessage();
        /* FIXME - We can't just return, so deleting */
        deleteLater();
        return;
    }

    kDebug() << "Telepathy file transfer channel ready";

    QObject::connect(m_channel.data(),
                     SIGNAL(stateChanged(Tp::FileTransferState,
                                         Tp::FileTransferStateChangeReason)),
                     SLOT(onFileTransferChannelStateChanged(Tp::FileTransferState,
                                                            Tp::FileTransferStateChangeReason)));
    QObject::connect(m_channel.data(),
                     SIGNAL(transferredBytesChanged(qulonglong)),
                     SLOT(onFileTransferChannelTransferredBytesChanged(qulonglong)));
}

void TelepathyFileTransfer::onFileTransferChannelStateChanged(Tp::FileTransferState state,
                                                              Tp::FileTransferStateChangeReason reason)
{
    kDebug() << "File transfer state changed:" << state << reason;

    switch (state) {
        case Tp::FileTransferStatePending:
            break;
        case Tp::FileTransferStateAccepted:
            m_channel->provideFile(&m_localFile);
            break;
        case Tp::FileTransferStateOpen:
            break;
        case Tp::FileTransferStateCompleted:
            m_transfer->slotComplete();
            break;
        case Tp::FileTransferStateCancelled:
            m_transfer->slotCancelled();
            break;
        case Tp::FileTransferStateNone:
            break;
        default:
            break;
    }
}

void TelepathyFileTransfer::onFileTransferChannelTransferredBytesChanged(qulonglong count)
{
    qDebug().nospace() << "Tranferred bytes " << count << " - " <<
            ((int) (((double) count / m_channel->size()) * 100)) << "% done";

    m_transfer->slotProcessed(count);
}

void TelepathyFileTransfer::onTransferCancelled()
{
    kDebug() << "File transfer cancelled";

//    deleteLater();
}

void TelepathyFileTransfer::onTransferResult()
{
    kDebug() << "File transfer result";
}
