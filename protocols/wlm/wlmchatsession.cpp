/*
    wlmchatsession.cpp - MSN Message Manager

    Copyright (c) 2008      by Tiago Salem Herrmann <tiagosh@gmail.com>
    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "wlmchatsession.h"

#include <QLabel>
#include <QImage>
#include <QToolTip>
#include <QFile>
#include <QIcon>
#include <QTextCodec>
#include <QRegExp>
#include <QDomDocument>
#include <QFileInfo>
#include <QBuffer>
#include <QPainter>
#include <QStringList>

#include <kconfig.h>
#include <kdebug.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <kaction.h>
#include <kactionmenu.h>
#include <kactioncollection.h>
#include <kmainwindow.h>
#include <ktoolbar.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>
#include <kemoticons.h>
#include <kcodecs.h>
#include <KTemporaryFile>

#include "kopetecontactaction.h"
#include "kopeteonlinestatus.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetechatsessionmanager.h"
#include "kopeteuiglobal.h"
#include "kopeteglobal.h"
#include "kopeteview.h"
#include "kopeteutils.h"
#include "private/kopeteemoticons.h"
#include "kopetetransfermanager.h"

#include "wlmcontact.h"
#include "wlmprotocol.h"
#include "wlmaccount.h"
#include "wlmchatsessioninkaction.h"
#ifdef HAVE_GIFLIB
#include <gif_lib.h>
/* old giflib has no GIFLIB_MAJOR, define to avoid cpp warnings */
#ifndef GIFLIB_MAJOR
#define GIFLIB_MAJOR 4
#endif
#endif

WlmChatSession::WlmChatSession (Kopete::Protocol * protocol,
                                const Kopete::Contact * user,
                                Kopete::ContactPtrList others,
                                MSN::SwitchboardServerConnection * conn):
Kopete::ChatSession (user, others, protocol),
m_chatService (conn),
m_downloadDisplayPicture (false),
m_sendNudge (false),
m_chatServiceRequested (false),
m_tries (0),
m_oimid (1),
m_sessionID(1)
{
#ifdef HAVE_MEDIASTREAMER
    m_voiceFilter = NULL;
    m_voiceCardCapture = NULL;
    m_voiceTicker = NULL;
    m_voiceRecorder = NULL;
    m_voiceTimer = NULL;
#endif

    Kopete::ChatSessionManager::self ()->registerChatSession (this);

    setComponentData (protocol->componentData ());

    connect (this, SIGNAL (messageSent (Kopete::Message &,
                                        Kopete::ChatSession *)),
             this, SLOT (slotMessageSent (Kopete::Message &,
                                          Kopete::ChatSession *)));

    connect (this, SIGNAL (myselfTyping(bool)),
             this, SLOT (sendTypingMsg(bool)));

    m_keepalivetimer = new QTimer (this);
    connect (m_keepalivetimer, SIGNAL (timeout()), SLOT (sendKeepAlive()));

    if (getChatService ()
        && getChatService ()->connectionState () ==
        MSN::SwitchboardServerConnection::SB_READY)
    {
        setReady (true);
    }

    m_actionNudge = new KAction (KIcon ("preferences-desktop-notification-bell"), i18n ("Send Nudge"), this);
    actionCollection ()->addAction ("wlmSendNudge", m_actionNudge);
    connect (m_actionNudge, SIGNAL (triggered(bool)), this,
             SLOT (sendNudge()));


    m_actionInvite =
        new KActionMenu (KIcon ("system-users"), i18n ("&Invite"), this);
    actionCollection ()->addAction ("wlmInvite", m_actionInvite);
    m_actionInvite->setDelayed(false);
    connect (m_actionInvite->menu (), SIGNAL (aboutToShow()), this,
             SLOT (slotActionInviteAboutToShow()));

    unsigned int userCaps = qobject_cast<WlmContact*>(members ().first ())->property(WlmProtocol::protocol()->contactCapabilities).value().toString().toUInt();

    // if the official client supports Isf, for some reason it won't accept gif's.
    if(userCaps & MSN::InkGifSupport &&
       !(userCaps & MSN::InkIsfSupport))
    {
        m_actionInk = new WlmChatSessionInkAction;
#ifdef HAVE_GIFLIB
        actionCollection ()->addAction ("wlmSendInk", m_actionInk);
#endif
        m_actionInk->setDelayed(false);
        connect(m_actionInk, SIGNAL(sendInk(QPixmap)), this, SLOT(slotSendInk(QPixmap)));
    }

#ifdef HAVE_MEDIASTREAMER
    if(userCaps & MSN::VoiceClips)
    {
        m_actionVoice = new KActionMenu (KIcon ("preferences-desktop-sound"), i18n ("Send &Voice"), this);
        actionCollection ()->addAction ("wlmSendVoice", m_actionVoice);
        ms_init();
        m_voiceCardCapture = ms_snd_card_manager_get_default_capture_card(ms_snd_card_manager_get());
        if (!m_voiceCardCapture)
        {
            actionCollection()->action("wlmSendVoice")->setEnabled(false);
            actionCollection()->action("wlmSendVoice")->setToolTip(i18n("Sound card not detected"));
        }
        connect (m_actionVoice->menu(), SIGNAL (aboutToShow()), this,
                 SLOT (slotSendVoiceStartRec()));

        connect (m_actionVoice->menu(), SIGNAL (aboutToHide()), this,
                 SLOT (slotSendVoiceStopRec()));

        KAction *stopRec = new  KAction(KIcon("wlm_fakefriend"), i18n("Stop &recording"), actionCollection ());
        m_actionVoice->addAction (stopRec);
        m_actionVoice->setDelayed(false);
    }
#endif

    setXMLFile ("wlmchatui.rc");
    setMayInvite (true);
}

void
WlmChatSession::sendKeepAlive ()
{
    if(isReady ())
        getChatService ()->sendKeepAlive ();
}

void
WlmChatSession::inviteContact (const QString & passport)
{
    if (!isReady () && !isConnecting ())
    {
        m_pendingInvitations.append (passport);
        requestChatService ();
        return;
    }
	WlmContact * c = qobject_cast<WlmContact*>(account ()->contacts().value(passport));
    if (c)
        slotInviteContact (c);
}

unsigned int
WlmChatSession::generateSessionID()
{
    m_sessionID++;
    QTime midnight(0, 0, 0);
    qsrand(midnight.secsTo(QTime::currentTime()));
    return (unsigned int) (qrand() + m_sessionID) & 0xFFFFFFFF;
}

void
WlmChatSession::sendFile (const QString & fileLocation, long unsigned int fileSize)
{
    Q_UNUSED( fileSize );

    QFileInfo fileInfo(fileLocation);

    MSN::fileTransferInvite ft;
    ft.type = MSN::FILE_TRANSFER_WITHOUT_PREVIEW;
    ft.sessionId = generateSessionID();
    ft.filename = QFile::encodeName(fileLocation).constData();
    ft.friendlyname = fileInfo.fileName().toUtf8().constData();
    ft.filesize = fileInfo.size ();
    ft.userPassport = members ().first ()->contactId ().toLatin1 ().constData ();

    // do not generate preview for big pictures
    if(ft.filesize < 2097152)
    {   
        QImage tryImage( fileLocation );
        if(tryImage.format() != QImage::Format_Invalid)
        {
            ft.type = MSN::FILE_TRANSFER_WITH_PREVIEW;
            QByteArray ba;
            QBuffer buffer(&ba);
            buffer.open(QIODevice::WriteOnly);
            tryImage = tryImage.scaled(64,64, Qt::KeepAspectRatio);
            // some clients are stretching the image, so make sure it is really 64x64
            if (tryImage.size() != QSize(64,64))
            {
                QImage temp(64,64, QImage::Format_ARGB32_Premultiplied);
                temp.fill(Qt::transparent);
                QRect r = tryImage.rect();
                r.moveCenter(temp.rect().center());
                QPainter p(&temp);
                p.drawImage(r.topLeft(), tryImage);
                tryImage = temp;
            }
            tryImage.save(&buffer, "PNG"); 
            ft.preview = KCodecs::base64Encode(ba).constData();
        }
    }

    // TODO create a switchboard to send the file is one if not available.
    if (isReady ())
    {
        if (!account ())
            return;
        WlmAccount *acc = qobject_cast < WlmAccount * >(account ());
        if (!acc)
            return;
        Kopete::Transfer * transf =
            Kopete::TransferManager::transferManager ()->
            addTransfer (members ().first (), fileLocation,
                         QFile (fileLocation).size (),
                         members ().first ()->contactId (),
                         Kopete::FileTransferInfo::Outgoing);

        connect (transf, SIGNAL (transferCanceled()),
                 acc->transferManager (), SLOT (slotCanceled()));
        acc->transferManager ()->addTransferSession (ft.sessionId, transf,
                                                     account ()->myself ()->
                                                     contactId (),
                                                     members ().first ()->
                                                     contactId ());

        setCanBeDeleted (false);
        getChatService ()->sendFile (ft);
    }
    else
    {
        m_pendingFiles.append (fileLocation);
        if (!isConnecting ())
            requestChatService ();
    }
}

void
WlmChatSession::slotSendVoiceStartRec ()
{
#ifdef HAVE_MEDIASTREAMER
    if (members ().count () < 0)
        return;

    if(members ().first ()->onlineStatus () ==
        WlmProtocol::protocol ()->wlmOffline ||
        members ().first ()->onlineStatus () ==
        WlmProtocol::protocol ()->wlmUnknown)
    {
        Kopete::Message msg = Kopete::Message ();
        msg.setPlainBody (i18n ("The other contact needs to be online to receive voice clips."));
        msg.setDirection (Kopete::Message::Internal);
        appendMessage (msg);
        // we cannot call hide() directly because the menu is not shown yet.
        QTimer::singleShot(0, m_actionVoice->menu(), SLOT(hide()));
        return;
    }

    if(myself()->onlineStatus () ==
        WlmProtocol::protocol ()->wlmInvisible)
    {
        Kopete::Message msg = Kopete::Message ();
        msg.setPlainBody (i18n ("You cannot send voice clips in invisible status"));
        msg.setDirection (Kopete::Message::Internal);
        appendMessage (msg);
        // we cannot call hide() directly because the menu is not shown yet.
        QTimer::singleShot(0, m_actionVoice->menu(), SLOT(hide()));
        return;
    }

    KTemporaryFile voiceClip;
    voiceClip.setPrefix("kopete_voiceClip-");
    voiceClip.setSuffix(".wav");
    voiceClip.open();
    voiceClip.setAutoRemove(false);
    m_currentVoiceClipName = voiceClip.fileName();
    addFileToRemove(m_currentVoiceClipName);

    int rate = 16000;
    
    m_voiceFilter=ms_snd_card_create_reader(m_voiceCardCapture);

    ms_filter_call_method (m_voiceFilter, MS_FILTER_SET_SAMPLE_RATE, &rate);
    m_voiceTicker=ms_ticker_new();

    m_voiceRecorder = ms_filter_new(MS_FILE_REC_ID);
    ms_filter_call_method(m_voiceRecorder,MS_FILE_REC_OPEN, QFile::encodeName(m_currentVoiceClipName).data ());
    ms_filter_call_method_noarg(m_voiceRecorder,MS_FILE_REC_START);
    ms_filter_call_method (m_voiceRecorder, MS_FILTER_SET_SAMPLE_RATE, &rate);

    ms_filter_link(m_voiceFilter,0,m_voiceRecorder,0);

    ms_ticker_attach(m_voiceTicker,m_voiceFilter);
    if(!m_voiceTimer)
    {
        m_voiceTimer = new QTimer(this);
        connect(m_voiceTimer, SIGNAL(timeout()), this, SLOT(slotSendVoiceStopRecTimeout()));
        m_voiceTimer->start (15 * 1000);
    }
#endif
}

void
WlmChatSession::slotSendVoiceStopRecTimeout()
{
#ifdef HAVE_MEDIASTREAMER
    if(m_voiceTimer)
    {
        Kopete::Message msg = Kopete::Message ();
        msg.setPlainBody (i18n ("The maximum recording time is 15 seconds"));
        msg.setDirection (Kopete::Message::Internal);
        appendMessage (msg);
        slotSendVoiceStopRec();
    }
#endif
}

void
WlmChatSession::slotSendVoiceStopRec()
{
#ifdef HAVE_MEDIASTREAMER
    if(m_actionVoice)
        m_actionVoice->menu()->hide();
    if(m_voiceTimer)
    {
        m_voiceTimer->stop();
        m_voiceTimer->deleteLater();
        m_voiceTimer = NULL;
    }
    if(m_voiceRecorder)
        ms_filter_call_method_noarg(m_voiceRecorder,MS_FILE_REC_CLOSE);
    if(m_voiceTicker && m_voiceFilter)
        ms_ticker_detach(m_voiceTicker,m_voiceFilter);
    if(m_voiceFilter && m_voiceRecorder)
        ms_filter_unlink(m_voiceFilter,0,m_voiceRecorder,0);
    if(m_voiceFilter)
        ms_filter_destroy(m_voiceFilter);
    if(m_voiceTicker)
       ms_ticker_destroy(m_voiceTicker);
    if(m_voiceRecorder)
        ms_filter_destroy(m_voiceRecorder);

    m_voiceRecorder=NULL;
    m_voiceTicker=NULL;
    m_voiceFilter=NULL;

    if(m_currentVoiceClipName.isEmpty())
        return;
    
    // average size of a 0.5 second voice clip
    if(QFile(m_currentVoiceClipName).size() < 15000)
    {
        Kopete::Message msg = Kopete::Message ();
        msg.setPlainBody (i18n ("The voice clip must be longer"));
        msg.setDirection (Kopete::Message::Internal);
        appendMessage (msg);
        m_currentVoiceClipName = QString();
        return;
    }

    if(getChatService () && isReady())
    {
        std::string obj;

        // keep a local file without the conversion to siren.
        // we use KTemporaryFile to generate a random file name
        KTemporaryFile voiceClip;
        voiceClip.setPrefix("kopete_voiceClip-");
        voiceClip.setSuffix(".wav");
        voiceClip.setAutoRemove(false);
        voiceClip.open();
        QString localVoice = voiceClip.fileName();
        addFileToRemove(voiceClip.fileName());
        voiceClip.close();
        // copy will not overwrite the file, so we need to delete it
        voiceClip.remove();
        QFile::copy(m_currentVoiceClipName, localVoice);

        QByteArray localFileName = QFile::encodeName(m_currentVoiceClipName);
        getChatService ()->myNotificationServer()->msnobj.addMSNObject(localFileName.constData (),11);
        getChatService ()->myNotificationServer()->msnobj.getMSNObjectXML(localFileName.constData (), 11, obj);
        getChatService ()->sendVoiceClip(obj);

        Kopete::Message kmsg( myself(), members() );
        kmsg.setType(Kopete::Message::TypeVoiceClipRequest);
        kmsg.setDirection( Kopete::Message::Outbound );
        kmsg.setFileName(localVoice);
        appendMessage ( kmsg );
    }
    else if (!isReady () && !isConnecting ())
    {
        m_pendingVoices.append (m_currentVoiceClipName);
        requestChatService ();
    }
    else if(isConnecting ())
    {
        m_pendingVoices.append (m_currentVoiceClipName);
    }
    m_currentVoiceClipName = QString();
#endif
}

void
WlmChatSession::slotSendFile ()
{
   qobject_cast < WlmContact * >(members ().first ())->sendFile ();
}

void
WlmChatSession::slotInviteContact (Kopete::Contact * contact)
{
    // if we have a session, just invite the new contact
    if (isReady ())
    {
        getChatService ()->inviteUser (contact->contactId ().toLatin1 ().constData ());
        return;
    }
    // if we are not in a session or connecting, add this contact to be invited later
    if (!isReady () && !isConnecting ())
    {
        m_pendingInvitations.append (contact->contactId ());
        requestChatService ();
        return;
    }
    // finally if we have a connection in progress, only add this user to be invited later
    if (isConnecting ())
        m_pendingInvitations.append (contact->contactId ());
}

static void
printGifErrorMessage()
{
#ifdef HAVE_GIFLIB
#ifdef HAVE_GIF_ERROR_STRING // giflib 4.2.0+
#if GIFLIB_MAJOR >= 5
        fprintf(stderr, "GIF-LIB error (exact reporting not implemented)\n");
#else
        const char * errorString = GifErrorString();
        if (errorString)
            fprintf(stderr, "GIF-LIB error: %s\n", errorString);
        else
            fprintf(stderr, "GIF-LIB undefined error: %d\n", GifError());
#endif
#else // older giflib versions, libungif
        PrintGifError();
#endif // HAVE_GIF_ERROR_STRING
#endif // HAVE_GIFLIB
}

/* stolen from kpaint write_to_gif() */
void
WlmChatSession::convertToGif( const QPixmap & ink, QString filename)
{
#ifdef HAVE_GIFLIB
#if GIFLIB_MAJOR >= 5
#define FreeMapObject  GifFreeMapObject
#define MakeMapObject  GifMakeMapObject
#endif
    int i, status;
    GifFileType *GifFile;
    ColorMapObject *screenColourmap;
    ColorMapObject *imageColourmap;
    QImage img = ink.toImage().convertToFormat(QImage::Format_Indexed8);

    imageColourmap= MakeMapObject(256, NULL);
    if (!imageColourmap) {
        return;
    }

    screenColourmap= MakeMapObject(256, NULL);
    if (!screenColourmap) {
        return;
    }

    for (i= 0; i < 256; i++) {
        if (i <img.numColors()) {
            imageColourmap->Colors[i].Red= qRed(img.color(i));
            imageColourmap->Colors[i].Green= qGreen(img.color(i));
            imageColourmap->Colors[i].Blue= qBlue(img.color(i));
        }
        else {
            imageColourmap->Colors[i].Red= 0;
            imageColourmap->Colors[i].Green= 0;
            imageColourmap->Colors[i].Blue= 0;
        }
    }

    for (i= 0; i < 256; i++) {
        if (i <img.numColors()) {
            screenColourmap->Colors[i].Red= qRed(img.color(i));
            screenColourmap->Colors[i].Green= qGreen(img.color(i));
            screenColourmap->Colors[i].Blue= qBlue(img.color(i));
        }
        else {
            screenColourmap->Colors[i].Red= 0;
            screenColourmap->Colors[i].Green= 0;
            screenColourmap->Colors[i].Blue= 0;
        }
    }

#if GIFLIB_MAJOR >= 5
    GifFile= EGifOpenFileName(QFile::encodeName(filename).constData(), 0, NULL);
#else
    GifFile= EGifOpenFileName(QFile::encodeName(filename).constData(), 0);
#endif
    if (!GifFile) {
        FreeMapObject(imageColourmap);
        FreeMapObject(screenColourmap);
        return;
    }

    status= EGifPutScreenDesc(GifFile,
                img.width(),
                img.height(),
                256,
                0,
                screenColourmap);

    if (status != GIF_OK) {
        EGifCloseFile(GifFile);
        return;
    }

    status= EGifPutImageDesc(GifFile,
               0, 0,
               img.width(),
               img.height(),
               0,
               imageColourmap);

    if (status != GIF_OK) {
        return;
    }

    for (i = 0; status && (i < img.height()); i++) {
        status= EGifPutLine(GifFile, 
            img.scanLine(i),
            img.width());
    }

    if (status != GIF_OK) {
        printGifErrorMessage();
        EGifCloseFile(GifFile);
        return;
    }

    if (EGifCloseFile(GifFile) != GIF_OK) {
        printGifErrorMessage();
        return;
    }
    return;
#endif
}

void
WlmChatSession::slotSendInk ( const QPixmap & ink)
{
    KTemporaryFile inkImage;
    inkImage.setPrefix("inkformatgif-");
    inkImage.setSuffix(".gif");
    inkImage.open();
    // if we autoremove the image, it will be deleted before
    // khtml have the chance to show it on the screen.
    inkImage.setAutoRemove(false);
    QString name = inkImage.fileName();
    addFileToRemove(name);
    convertToGif(ink, name);

    // encode to base64 and send it to libmsn
    QByteArray draw = KCodecs::base64Encode(inkImage.readAll());
    if(!isReady() && !isConnecting())
    {
        m_pendingInks << draw;
        requestChatService ();
    }
    else if (isConnecting ())
        m_pendingInks << draw;
    else
        getChatService ()->sendInk(draw.constData());

    QString msg=QString ("<img src=\"%1\" />").arg ( name );

    Kopete::Message kmsg( myself(), members() );
    kmsg.setHtmlBody( msg );
    kmsg.setDirection( Kopete::Message::Outbound );
    appendMessage ( kmsg );

    inkImage.deleteLater();
}

void
WlmChatSession::slotActionInviteAboutToShow ()
{
    // We can't simply insert  KAction in this menu bebause we don't know when to delete them.
    //  items inserted with insert items are automatically deleted when we call clear

    qDeleteAll (m_inviteactions);
    m_inviteactions.clear ();

    m_actionInvite->menu ()->clear ();


    QHash < QString, Kopete::Contact * >contactList = account ()->contacts();
    QHash < QString, Kopete::Contact * >::Iterator it, itEnd =
        contactList.end ();
    for (it = contactList.begin (); it != itEnd; ++it)
    {
        if (!members ().contains (it.value ()) && it.value ()->isOnline ())
        {
            KAction *a =
                new Kopete::UI::ContactAction (it.value (),
                                               actionCollection ());
			connect( a, SIGNAL(triggered(Kopete::Contact*,bool)),
					this, SLOT(slotInviteContact(Kopete::Contact*)) );

            m_actionInvite->addAction (a);
            m_inviteactions.append (a);
        }
    }

    // We can't simply insert  KAction in this menu bebause we don't know when to delete them.
    //  items inserted with insert items are automatically deleted when we call clear
/*
    m_inviteactions.setAutoDelete(true);
    m_inviteactions.clear();

    m_actionInvite->popupMenu()->clear();

    QListIterator<Kopete::Contact> it( account()->contacts() );
    for( ; it.current(); ++it )
    {
        if( !members().contains( it.current() ) && it.current()->isOnline() && it.current() != myself() )
        {
            KAction *a=new KopeteContactAction( it.current(), this,
                SLOT(slotInviteContact(Kopete::Contact*)), m_actionInvite );
            m_actionInvite->insert( a );
            m_inviteactions.append( a ) ;
        }
    }
//    KAction *b=new KAction( i18n ("Other..."), 0, this, SLOT(slotInviteOtherContact()), m_actionInvite, "actionOther" );
//    m_actionInvite->insert( b );
//    m_inviteactions.append( b ) ;
*/
}


void
WlmChatSession::sendNudge ()
{
    if (isReady ())
    {
        getChatService ()->sendNudge ();
        Kopete::Message msg = Kopete::Message (myself (), members ());
        msg.setDirection (Kopete::Message::Outbound);
        msg.setType (Kopete::Message::TypeAction);
        msg.setPlainBody (i18n ("has sent a nudge"));

        appendMessage (msg);
        return;
    }

    if (!isConnecting ())
    {
        setSendNudge (true);
        requestChatService ();
    }
}

WlmChatSession::~WlmChatSession ()
{
    if (!account ())
        return;

    WlmAccount *acc = qobject_cast < WlmAccount * >(account ());

    if (!acc)
        return;

    WlmChatManager *manager = acc->chatManager ();

    if (manager && getChatService ())
        manager->chatSessions.remove (getChatService ());

    stopSendKeepAlive();

    if (isReady () && getChatService ())
    {
        delete getChatService ();
        setChatService(NULL);
    }
    for (int i = 0; i < m_filesToRemove.size(); ++i)
        QFile::remove(m_filesToRemove.at(i));
}

void
WlmChatSession::addFileToRemove (QString path)
{
    m_filesToRemove << path;
}

bool
WlmChatSession::isConnecting()
{
    if(!getChatService ())
        return false;

    if(getChatService()->connectionState () !=
        MSN::SwitchboardServerConnection::SB_READY &&
            getChatService ()->connectionState () !=
             MSN::SwitchboardServerConnection::SB_DISCONNECTED)
        return true;
    return false;
}

void
WlmChatSession::setChatService (MSN::SwitchboardServerConnection * conn)
{
    m_chatService = conn;
    if (!getChatService())
    {
        setReady (false);
        return;
    }
    if (getChatService ()
        && getChatService ()->connectionState () ==
            MSN::SwitchboardServerConnection::SB_READY)
    {
        setReady (true);
    }
}

MSN::Message WlmChatSession::parseMessage(Kopete::Message & msg)
{
	// send the message and wait for the ACK
	int fontEffects = 0;
    MSN::Message mmsg(msg.plainBody().toUtf8().constData());

	// FIXME: Can we add FontFamily FF_DONTCARE ?
	if (msg.format() == Qt::RichText)
	{
    	mmsg.setFontName(msg.font().family().toLatin1().constData());
		if (msg.font().bold())
			fontEffects |= MSN::Message::BOLD_FONT;
		if (msg.font().italic())
			fontEffects |= MSN::Message::ITALIC_FONT;
		if (msg.font().underline())
			fontEffects |= MSN::Message::UNDERLINE_FONT;
		if (msg.font().strikeOut())
			fontEffects |= MSN::Message::STRIKETHROUGH_FONT;

		mmsg.setFontEffects(fontEffects);
		QColor color = msg.foregroundColor();
		mmsg.setColor(color.red(), color.green(), color.blue());
	}

    WlmAccount *acc = qobject_cast < WlmAccount * >(account ());
    if(!acc)
        return mmsg;

    if(!acc->doNotSendEmoticons())
    {
        // stolen from msn plugin
        const QHash<QString, QStringList> emap = Kopete::Emoticons::self()->theme().emoticonsMap();

        // Check the list for any custom emoticons
        for (QHash<QString, QStringList>::const_iterator itr = emap.begin(); itr != emap.end(); ++itr)
        {
            for (QStringList::const_iterator itr2 = itr.value().constBegin(); itr2 != itr.value().constEnd(); ++itr2)
            {
                if (msg.plainBody().contains(*itr2))
                {
                    getChatService()->sendEmoticon((*itr2).toUtf8().constData(), QFile::encodeName(itr.key()).constData());
                }
            }
        }
    }
	return mmsg;
}

void
WlmChatSession::setReady (bool value)
{
    Q_UNUSED( value );

    if (isReady ())
    {
        m_tries = 0;
        if (isDownloadDisplayPicture ())
        {
            setDownloadDisplayPicture (false);
            requestDisplayPicture ();
        }
        if (isSendNudge ())
        {
            sendNudge ();
            setSendNudge (false);
        }

        // invite pending contacts
        QLinkedList < QString >::iterator it;
        for (it = m_pendingInvitations.begin ();
             it != m_pendingInvitations.end (); ++it)
        {
	        WlmContact * c = qobject_cast<WlmContact*>(account ()->contacts().value(*it));
            if (c)
                slotInviteContact (c);
        }
        m_pendingInvitations.clear ();

        // send queued messages first
        QLinkedList < Kopete::Message >::iterator it2;
        for (it2 = m_messagesQueue.begin (); it2 != m_messagesQueue.end ();
             ++it2)
        {
            MSN::Message mmsg = parseMessage(*it2);

            int trid = getChatService ()->sendMessage (&mmsg);

            m_messagesSentQueue[trid] = (*it2);
        }
        m_messagesQueue.clear ();

        // send pending files
        QLinkedList < QString >::iterator it3;
        for (it3 = m_pendingFiles.begin (); it3 != m_pendingFiles.end ();
             ++it3)
        {
            sendFile ((*it3), 0);
        }
        m_pendingFiles.clear ();

        QLinkedList < QByteArray >::iterator it4;
        for (it4 = m_pendingInks.begin (); it4 != m_pendingInks.end (); ++it4)
        {
            getChatService ()->sendInk((*it4).constData());
        }
        m_pendingInks.clear ();

#ifdef HAVE_MEDIASTREAMER
        QLinkedList < QString >::iterator it5;
        for (it5 = m_pendingVoices.begin (); it5 != m_pendingVoices.end ();
             ++it5)
        {
            std::string obj;

            // keep a local file without the conversion to siren.
            KTemporaryFile voiceClip;
            voiceClip.setPrefix("kopete_voiceClip-");
            voiceClip.setSuffix(".wav");
            voiceClip.setAutoRemove(false);
            voiceClip.open();
            addFileToRemove(voiceClip.fileName());
            QString localVoice = voiceClip.fileName();
            voiceClip.close();
            // copy will not overwrite the file, so we need to delete it
            voiceClip.remove();
            QFile::copy((*it5), localVoice);

            QByteArray localFileName = QFile::encodeName( (*it5) );
            getChatService ()->myNotificationServer()->msnobj.addMSNObject(localFileName.constData (),11);
            getChatService ()->myNotificationServer()->msnobj.getMSNObjectXML(localFileName.constData (), 11, obj);
            getChatService ()->sendVoiceClip(obj);

            Kopete::Message kmsg( myself(), members() );
            kmsg.setType(Kopete::Message::TypeVoiceClipRequest);
            kmsg.setDirection( Kopete::Message::Outbound );
            kmsg.setFileName(localVoice);
            appendMessage ( kmsg );
        }
        m_pendingVoices.clear ();
        m_currentVoiceClipName = QString();
#endif
    }
    else
    {
        stopSendKeepAlive();
    }
}

bool
WlmChatSession::requestChatService ()
{
    // check if the other contact is offline
    if (members ().count () > 0 &&
        members ().first ()->onlineStatus () ==
        WlmProtocol::protocol ()->wlmOffline)
        return false;

    if (!isReady () && account ()->isConnected () && !isConnecting () && !m_chatServiceRequested)
    {
        const std::string rcpt_ = members().first()->contactId().toLatin1().constData();
        const std::string msg_ = "";
        const std::pair<std::string, std::string> *ctx = new std::pair<std::string, std::string>(rcpt_, msg_);
        // request a new switchboard connection
        static_cast <WlmAccount *>(account ())->server ()->cb.mainConnection->requestSwitchboardConnection (ctx);
        QTimer::singleShot (30 * 1000, this, SLOT (switchboardConnectionTimeout()));
        // if the user attempts to send more than one message a new sb
        // is requested every time unless we received one in the mean
        // time and the other client has already joined
        m_chatServiceRequested = true;
        return true;
    }
    // probably we are about to connect
    return true;
}

void
WlmChatSession::switchboardConnectionTimeout ()
{
    if (!isReady ())
    {
        // allow a new chat service request
        m_chatServiceRequested = false;
        // try 3 times
        if (m_tries < 3)
        {
            m_tries++;
            requestChatService ();
            return;
        }
        Kopete::Utils::notifyCannotConnect(account(), "Could not open switchboard connection");

        QMap<unsigned int, Kopete::Message>::const_iterator i;
        for (i = m_messagesSentQueue.constBegin(); i != m_messagesSentQueue.constEnd(); ++i)
            this->receivedMessageState(i->id(), Kopete::Message::StateError );

        messageSucceeded ();
    }
}

void
WlmChatSession::messageTimeout ()
{
    int trid = m_messagesTimeoutQueue.takeFirst();
	if(m_messagesSentQueue.contains(trid))
		this->receivedMessageState(m_messagesSentQueue[trid].id(), Kopete::Message::StateError );
}

void
WlmChatSession::slotMessageSent (Kopete::Message & msg,
                                 Kopete::ChatSession * chat)
{
    Q_UNUSED( chat );
    if (!account ()->isConnected ())
    {
        KMessageBox::queuedMessageBox (Kopete::UI::Global::mainWidget (),
                                       KMessageBox::Information,
                                       "You cannot send a message while in offline status",
                                       "Information");
        messageSucceeded ();
        return;
    }

    if (isReady ())
    {
        MSN::Message mmsg = parseMessage(msg);

        int trid = getChatService ()->sendMessage (&mmsg);

        // Show the message we just sent in the chat window as sending
        msg.setState( Kopete::Message::StateSending );
        this->appendMessage(msg);
        this->messageSucceeded();

        m_messagesSentQueue[trid] = msg;
        m_messagesTimeoutQueue.append(trid);
        QTimer::singleShot (60 * 1000, this,
                            SLOT (messageTimeout()));
        return;
    }

    if (!isConnecting () && !isReady ())
    {
        // request switchboard
        if (!requestChatService ())
        {
            MSN::Soap::OIM oim;
            oim.myFname = myself ()->displayName ().toUtf8().constData();
            oim.toUsername = members ().first ()->contactId ().toLatin1 ().constData();
            oim.message = msg.plainBody ().toUtf8 ().constData();
            oim.myUsername = myself ()->contactId ().toLatin1 ().constData();
            oim.id = m_oimid++;

            static_cast <WlmAccount *>(account ())->server ()->cb.mainConnection->send_oim (oim);
            appendMessage (msg);
            messageSucceeded ();
            return;
        }

        // Show the message we just sent in the chat window as sending
        msg.setState( Kopete::Message::StateSending );
        this->appendMessage(msg);
        this->messageSucceeded();

        // put the message in a queue
        m_messagesQueue.append (msg);
        return;
    }

    if (isConnecting ())
    {
        // Show the message we just sent in the chat window as sending
        msg.setState( Kopete::Message::StateSending );
        this->appendMessage(msg);
        this->messageSucceeded();

        // put the message in the queue, we are trying to connect to the
        // switchboard server
        m_messagesQueue.append (msg);
        return;
    }

}
bool
WlmChatSession::isReady ()
{
    if(!getChatService ())
        return false;

    // check in libmsn if we are really ready
    if(getChatService ()->connectionState () ==
            MSN::SwitchboardServerConnection::SB_READY)
        return true;
    else
        return false;
}

void
WlmChatSession::sendTypingMsg (bool istyping)
{
    if (!istyping || isConnecting ())
        return;

    // do not send notification if we 
    // are alone in the session
    if (!isReady ())
        return;

    getChatService ()->sendTypingNotification ();

    startSendKeepAlive();
}

void
WlmChatSession::messageSentACK (unsigned int trID)
{
    this->receivedMessageState(m_messagesSentQueue[trID].id(), Kopete::Message::StateSent );

    m_messagesSentQueue.remove (trID);
    // remove the blinking icon when there are no messages
    // waiting for delivery
    if (m_messagesSentQueue.empty ())
        messageSucceeded ();
}

void 
WlmChatSession::startSendKeepAlive()
{
    // send keepalive each 50 seconds.
    if(m_keepalivetimer && isReady())
        m_keepalivetimer->start (50 * 1000);
}

void 
WlmChatSession::stopSendKeepAlive()
{
    if(m_keepalivetimer)
        m_keepalivetimer->stop ();
}

void
WlmChatSession::receivedNudge (QString passport)
{
	WlmContact * c = qobject_cast<WlmContact*>(account ()->contacts().value(passport));
    if (!c)
        c = qobject_cast<WlmContact*>(members ().first ());

    Kopete::Message msg = Kopete::Message (c, myself ());
    msg.setPlainBody (i18n ("has sent you a nudge"));
    msg.setDirection (Kopete::Message::Inbound);
    msg.setType (Kopete::Message::TypeAction);
    appendMessage (msg);
    emitNudgeNotification ();   // notifies with system message close to tray icon
    startSendKeepAlive();

}

void
WlmChatSession::requestDisplayPicture ()
{
    // only request picture in a 2 session people only
    if (members ().count () != 1)
        return;

    WlmContact *contact = qobject_cast < WlmContact * >(members ().first ());

    if (!contact)
        return;

    if (contact->getMsnObj ().isEmpty () || contact->getMsnObj () == "0")
        return;

    QString msnobj = contact->getMsnObj ();

    QDomDocument xmlobj;
    xmlobj.setContent (msnobj);

    // track display pictures by SHA1D field
    QString SHA1D = xmlobj.documentElement ().attribute ("SHA1D");

    if (SHA1D.isEmpty ())
        return;

    QString currentSHA1D = contact->property(WlmProtocol::protocol()->displayPhotoSHA1).value().toString();
    QString photoPath = contact->property(Kopete::Global::Properties::self()->photo().key()).value().toString();
    if (SHA1D == currentSHA1D && QFileInfo(photoPath).size() > 0)
        return;

    // request switchboard connection
    // and ask for the display picture
    if (!isReady () && !isConnecting ())
    {
        requestChatService ();
        setDownloadDisplayPicture (true);
        return;                 // TODO - schedule this action
    }
    if (isReady ())
    {
        QString newlocation = KGlobal::dirs()->locateLocal("appdata", "wlmpictures/" + QString(SHA1D.replace ('/', '_')));
        getChatService()->requestDisplayPicture(generateSessionID(), QFile::encodeName(newlocation).constData(),
                                                contact->getMsnObj().toUtf8().constData());
        setDownloadDisplayPicture (false);
    }
}

#include "wlmchatsession.moc"
