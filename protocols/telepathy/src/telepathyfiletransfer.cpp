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

#include <telepathyfiletransfer.h>

#include <telepathyaccount.h>
#include <telepathychatsession.h>
#include <telepathyprotocolinternal.h>

#include <KDebug>

#include <TelepathyQt4/Constants>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/PendingOperation>
#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/PendingChannel>
#include <TelepathyQt4/FileTransferChannel>
#include <TelepathyQt4/OutgoingFileTransferChannel>
#include <TelepathyQt4/IncomingFileTransferChannel>


TelepathyFileTransfer::TelepathyFileTransfer(Tp::ChannelPtr channel,
                                             TelepathyContact *contact,
                                             const QString &fileName)
    : QObject(contact),
      m_localFile(fileName),
      m_transfer(0),
      m_contact(contact),
      m_direction(Outgoing)
{
    kDebug() << "Starting new outgoing file transfer:"
             << contact->contactId() << fileName;

    if (!m_localFile.open(QIODevice::ReadOnly)) {
        kWarning() << "Unable to open file for reading";
        deleteLater();
        return;
    }

    m_transfer = Kopete::TransferManager::transferManager()->
        addTransfer(m_contact,
                    m_localFile.fileName(),
                    m_localFile.size(),
                    m_contact->contactId(),
                    Kopete::FileTransferInfo::Outgoing);

    m_channel = Tp::FileTransferChannelPtr::dynamicCast(channel);

    QObject::connect(m_transfer,
                     SIGNAL(slotCancelled()),
                     SLOT(onTransferCancelled()));

    QObject::connect(m_transfer,
                     SIGNAL(result(KJob *)),
                     SLOT(onTransferResult()));

    QObject::connect(m_channel.data(),
                     SIGNAL(invalidated(Tp::DBusProxy *,
                            const QString &, const QString &)),
                     SLOT(onInvalidated()));

    QObject::connect(m_channel->becomeReady(
                Tp::FileTransferChannel::FeatureCore),
                     SIGNAL(finished(Tp::PendingOperation *)),
                     SLOT(onFileTransferChannelReady(Tp::PendingOperation *)));
}

TelepathyFileTransfer::TelepathyFileTransfer(Tp::ChannelPtr channel,
                                             TelepathyContact *contact)
    : QObject(contact),
      m_localFile(""),
      m_transfer(0),
      m_contact(contact),
      m_direction(Incoming),
      m_transferId(0)
{
    kDebug() << "Starting new incoming file transfer:"
             << contact->contactId();

    QObject::connect(Kopete::TransferManager::transferManager(),
                     SIGNAL(accepted(Kopete::Transfer *, const QString &)),
                     SLOT(onIncomingTransferAccepted(Kopete::Transfer *,
                                                     const QString &)));
    QObject::connect(Kopete::TransferManager::transferManager(),
                     SIGNAL(refused(const Kopete::FileTransferInfo &)),
                     SLOT(onIncomingTransferRefused(
                             const Kopete::FileTransferInfo &)));

    m_channel = Tp::FileTransferChannelPtr::dynamicCast(channel);

    QVariantMap properties = m_channel->immutableProperties();

    m_transferId = Kopete::TransferManager::transferManager()->
        askIncomingTransfer(contact,
                            properties[TELEPATHY_INTERFACE_CHANNEL_TYPE_FILE_TRANSFER ".Filename"].toString(),
                            properties[TELEPATHY_INTERFACE_CHANNEL_TYPE_FILE_TRANSFER ".Size"].toULongLong(),
                            properties[TELEPATHY_INTERFACE_CHANNEL_TYPE_FILE_TRANSFER ".Description"].toString(),
                            QString());
}

TelepathyFileTransfer::~TelepathyFileTransfer()
{
    kDebug() << "Destroying file transfer";

    m_localFile.close();
    m_channel->requestClose();
}

void TelepathyFileTransfer::onInvalidated()
{
    kDebug() << "Invalidated";

    m_transfer->slotError(KIO::ERR_INTERNAL, "Channel invalidated");
}

void TelepathyFileTransfer::onFileTransferChannelReady(Tp::PendingOperation *op)
{
    if (op->isError()) {
        kWarning() << "Readying file transfer channel failed:"
                   << op->errorName()
                   << op->errorMessage();
        m_transfer->slotError(KIO::ERR_COULD_NOT_CONNECT, op->errorMessage());
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

    if (m_direction == Incoming) {
        Tp::IncomingFileTransferChannelPtr inChannel =
            Tp::IncomingFileTransferChannelPtr::dynamicCast(m_channel);
        inChannel->acceptFile(0, &m_localFile);
    }
}

void TelepathyFileTransfer::onFileTransferChannelStateChanged(Tp::FileTransferState state,
                                                              Tp::FileTransferStateChangeReason reason)
{
    kDebug() << "File transfer state changed:" << state << reason;

    switch (state) {
        case Tp::FileTransferStatePending:
            break;
        case Tp::FileTransferStateAccepted:
            if (m_direction == Outgoing) {
                Tp::OutgoingFileTransferChannelPtr outChannel =
                    Tp::OutgoingFileTransferChannelPtr::dynamicCast(m_channel);
                outChannel->provideFile(&m_localFile);
            }
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

    m_channel->cancel();
}

void TelepathyFileTransfer::onTransferResult()
{
    kDebug() << "File transfer result";

    deleteLater();
}

void TelepathyFileTransfer::onIncomingTransferAccepted(Kopete::Transfer *trans,
                                                       const QString &fileName)
{
    kDebug() << "User has accepted the incoming file transfer";

    if ((long)trans->info().transferId() != m_transferId)
        return;

    m_transfer = trans;
    m_localFile.setFileName(fileName);

    if (!m_localFile.open(QIODevice::WriteOnly)) {
        kWarning() << "Unable to open file for reading";
        m_transfer->slotError(KIO::ERR_COULD_NOT_WRITE, fileName);
        return;
    }

    QObject::connect(m_transfer,
                     SIGNAL(slotCancelled()),
                     SLOT(onTransferCancelled()));

    QObject::connect(m_transfer,
                     SIGNAL(result(KJob *)),
                     SLOT(onTransferResult()));

    QObject::connect(m_channel.data(),
                     SIGNAL(invalidated(Tp::DBusProxy *,
                            const QString &, const QString &)),
                     SLOT(onInvalidated()));

    QObject::connect(m_channel->becomeReady(
                Tp::FileTransferChannel::FeatureCore),
                     SIGNAL(finished(Tp::PendingOperation *)),
                     SLOT(onFileTransferChannelReady(Tp::PendingOperation *)));
}

void TelepathyFileTransfer::onIncomingTransferRefused(
        const Kopete::FileTransferInfo &info)
{
    kDebug() << "User has refused the incoming file transfer:"
             << info.file();

    if ((long)info.transferId() != m_transferId)
        return;

    deleteLater();
}

