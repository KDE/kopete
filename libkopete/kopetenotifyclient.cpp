/* This file is part of the KDE libraries
   Copyright (C) 2000 Charles Samuels <charles@altair.dhs.org>
                 2000 Malte Starostik <starosti@zedat.fu-berlin.de>
		 2000,2003 Carsten Pfeiffer <pfeiffer@kde.org>
		 2003 Olivier Goffart <ogoffart@tiscalinet.be>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kopetenotifyclient.h"

// QT headers
#include <qfile.h>
#include <qvbox.h>
#include <qsignal.h>
#include <qfileinfo.h>
#include <qptrstack.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qdatastream.h>


// KDE headers
#include <kwin.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kprocess.h>
#include <dcopclient.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kactivelabel.h>
#include <kpassivepopup.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>



static const char daemonName[] = "knotify";

static int notifyBySound(const QString &filename , const QString &appname, int uniqueId)
{
  if (!kapp) return 0;

  DCOPClient *client=kapp->dcopClient();
  if (!client->isAttached())
  {
    client->attach();
    if (!client->isAttached())
      return 0;
  }

  if ( !KopeteNotifyClient::startDaemon() )
      return 0;


  QByteArray data;
  QDataStream ds(data, IO_WriteOnly);


#if 0   //TODO: when knotify support it, use this method insteads of the following
  ds << filename << appname << uniqueId;
  if ( client->send(daemonName, "Notify", "notifyBySound(QString,QString,int)", data) )
#elif KDE_IS_VERSION( 3, 1, 90 )
  ds << QString::null << appname << QString::null << filename << QString::null << 1 << KopeteNotifyClient::Default << 0 << uniqueId;
  if ( client->send(daemonName, "Notify", "notify(QString,QString,QString,QString,QString,int,int,int,int)", data) )
      return uniqueId;
#else
  ds << QString::null << appname << QString::null << filename << QString::null << 1 << KopeteNotifyClient::Default << 0 ;
  if ( client->send(daemonName, "Notify", "notify(QString,QString,QString,QString,QString,int,int,int)", data) )
      return uniqueId;
#endif

  return 0;
}


static bool notifyByMessagebox(const QString &text, int level, const KGuiItem &action , QObject* receiver , const char* slot)
{
    // ignore empty messages
    if ( text.isEmpty() )
        return false;


    if(!receiver || !slot) {
	// display message box for specified event level
		switch( level ) {
		default:
		case KopeteNotifyClient::Notification:
			KMessageBox::information( 0, text, i18n( "Notification" ) );
			break;
		case KopeteNotifyClient::Warning:
			KMessageBox::sorry( 0, text, i18n( "Warning" ) );
			break;
		case KopeteNotifyClient::Error:
			KMessageBox::error( 0, text, i18n( "Error" ) );
			break;
		case KopeteNotifyClient::Catastrophe:
			KMessageBox::error( 0, text, i18n( "Fatal" ) );
			break;
	}
    } else { //we may show the specific action button
		int result=0;
		QSignal signal;
		signal.connect(receiver, slot);
		switch( level ) {
		default:
		case KopeteNotifyClient::Notification:
			result=KMessageBox::questionYesNo(0, text, i18n("Notification"), action, KStdGuiItem::cancel() , QString::null, false );
			break;
		case KopeteNotifyClient::Warning:
			result=KMessageBox::warningYesNo( 0, text, i18n("Warning"), action, KStdGuiItem::cancel() , QString::null, false );
			break;
		case KopeteNotifyClient::Error:
			result=KMessageBox::warningYesNo( 0, text, i18n("Error"), action, KStdGuiItem::cancel() , QString::null, false );
			break;
		case KopeteNotifyClient::Catastrophe:
			result=KMessageBox::warningYesNo( 0, text, i18n("Catastrophe!"), action, KStdGuiItem::cancel() , QString::null, false );
			break;
	}
	if(result==KMessageBox::Yes)
		signal.activate();

    }


    return true;
}

static bool notifyByPassivePopup( const QString &text, const QString &appName,WId senderWinId,
                        const KGuiItem &action , QObject* receiver , const char* slot )
{
    KIconLoader iconLoader( appName );
    KConfig eventsFile( KopeteNotifyClient::instance()->instanceName()+"/eventsrc", true, false, "data");
    KConfigGroup config( &eventsFile, "!Global!" );
    QString iconName = config.readEntry( "IconName", appName );
    QPixmap icon = iconLoader.loadIcon( iconName, KIcon::Small );
    QString title = config.readEntry( "Comment", appName );
    //KPassivePopup::message(title, text, icon, senderWinId);

  	KPassivePopup *pop = new KPassivePopup( senderWinId );

	QVBox *vb = pop->standardView( title, text, icon );

	if ( receiver && slot )
	{
		KActiveLabel *link = new KActiveLabel( QString::fromLatin1( "<p align=\"right\"><a href=\" \">" ) +
            action.plainText() + QString::fromLatin1( "</a></p>" ), vb, "msg_label" );
		//link->setAlignment( AlignRight );
		QObject::disconnect(link, SIGNAL(linkClicked(const QString &)), link, SLOT(openLink(const QString &)));
		QObject::connect(link, SIGNAL(linkClicked(const QString &)), receiver, slot);
		QObject::connect(link, SIGNAL(linkClicked(const QString &)), pop, SLOT(hide()));
	}

	pop->setAutoDelete( true );
	pop->setTimeout(-1);

	pop->setView( vb );
	pop->show();

    return true;
}

static bool notifyByExecute(const QString &command) {
    if (!command.isEmpty()) {
	// kdDebug() << "executing command '" << command << "'" << endl;
	KProcess p;
	p.setUseShell(true);
	p << command;
	p.start(KProcess::DontCare);
	return true;
    }
    return false;
}


static bool notifyByLogfile(const QString &text, const QString &file)
{
    // ignore empty messages
    if ( text.isEmpty() )
        return true;

    // open file in append mode
    QFile logFile(file);
    if ( !logFile.open(IO_WriteOnly | IO_Append) )
        return false;

    // append msg
    QTextStream strm( &logFile );
    strm << "- KNotify " << QDateTime::currentDateTime().toString() << ": ";
    strm << text << endl;

    // close file
    logFile.close();
    return true;
}

static bool notifyByStderr(const QString &text)
{
    // ignore empty messages
    if ( text.isEmpty() )
        return true;

    // open stderr for output
    QTextStream strm( stderr, IO_WriteOnly );

    // output msg
    strm << "KNotify " << QDateTime::currentDateTime().toString() << ": ";
    strm << text << endl;

    return true;
}

static bool notifyByTaskbar( WId win )
{
    if( win == 0 )
        return false;

#if KDE_IS_VERSION( 3, 1, 90 )
    KWin::demandAttention( win );
    return true;
#else
    return false;
#endif
}


int KopeteNotifyClient::event( StandardEvent type, const QString& text )
{
    return event( 0, type, text );
}

int KopeteNotifyClient::event(const QString &message, const QString &text)
{
    return event(0, message, text);
}

int KopeteNotifyClient::userEvent(const QString &text, int present, int level,
                              const QString &sound, const QString &file)
{
    return userEvent( 0, text, present, level, sound, file );
}



int KopeteNotifyClient::event( int winId, StandardEvent type, const QString& text )
{
    QString message;
    switch ( type ) {
    case cannotOpenFile:
	message = QString::fromLatin1("cannotopenfile");
	break;
    case warning:
	message = QString::fromLatin1("warning");
	break;
    case fatalError:
	message = QString::fromLatin1("fatalerror");
	break;
    case catastrophe:
	message = QString::fromLatin1("catastrophe");
	break;
    case notification: // fall through
    default:
	message = QString::fromLatin1("notification");
	break;
    }

    return event( winId, message,  text  );
}


int KopeteNotifyClient::event(int winId, const QString &message,
                          const QString &text)
{
    return event( winId , message, text, KGuiItem() , 0L , 0L)  ;
}

int KopeteNotifyClient::userEvent(int winId, const QString &text, int present,
                              int level,
                              const QString &sound, const QString &file)
{
    return userEvent(winId, text,  present , level, sound, file, QString::null , KGuiItem() , 0L , 0L);
}



int KopeteNotifyClient::event(int winId, const QString &message, const QString &text,
                    const KGuiItem &action , QObject* receiver , const char* slot)
{
   if (message.isEmpty()) return 0;

    int level=Default;
    QString sound;
    QString file;
    QString commandline;

    // get config file
    KConfig eventsFile( KopeteNotifyClient::instance()->instanceName()+"/eventsrc", true, false, "data");
    eventsFile.setGroup(message);

    KConfig configFile( KopeteNotifyClient::instance()->instanceName()+".eventsrc", true, false);
    configFile.setGroup(message);

	int present=getPresentation(message);
	if(present==-1)
		present=getDefaultPresentation(message);
	if(present==-1)
		present=0;

    // get sound file name
    if( present & KopeteNotifyClient::Sound ) {
        sound = configFile.readPathEntry( "soundfile" );
        if ( sound.length()==0 )
            sound = eventsFile.readPathEntry( "default_sound" );
    }

    // get log file name
    if( present & KopeteNotifyClient::Logfile ) {
        file = configFile.readPathEntry( "logfile" );
        if ( file.length()==0 )
            file = eventsFile.readPathEntry( "default_logfile" );
    }

    // get default event level
    if( present & KopeteNotifyClient::Messagebox )
        level = eventsFile.readNumEntry( "level", 0 );

     // get command line
    if (present & KopeteNotifyClient::Execute ) {
        commandline = configFile.readPathEntry( "commandline" );
        if ( commandline.length()==0 )
            commandline = eventsFile.readPathEntry( "default_commandline" );
    }


    return userEvent(winId, text,  present , level, sound, file, commandline, action, receiver, slot);
}

int KopeteNotifyClient::userEvent(int winId, const QString &text, int present, int level,
                              const QString &sound, const QString &file, const QString& commandline,
                              const KGuiItem &action , QObject* receiver , const char* slot)
{
    int uniqueId = kMax( 1, kapp->random() ); // must not be 0 -- means failure!

    QString appname = KopeteNotifyClient::instance()->instanceName();

    if(winId==0   && kapp->mainWidget())
    {
	winId=kapp->mainWidget()->winId();
    }

    // emit event
    if ( present & KopeteNotifyClient::Sound ) // && QFile(sound).isReadable()
        notifyBySound( sound , appname , uniqueId ) ;

    if ( present & KopeteNotifyClient::PassivePopup )
        notifyByPassivePopup( text, appname, winId, action, receiver, slot );

    else if ( present & KopeteNotifyClient::Messagebox )
        notifyByMessagebox( text, level, action, receiver, slot );

    if ( present & KopeteNotifyClient::Logfile ) // && QFile(file).isWritable()
        notifyByLogfile( text, file );

    if ( present & KopeteNotifyClient::Stderr )
        notifyByStderr( text );

    if ( present & KopeteNotifyClient::Execute )
        notifyByExecute( commandline );

    if ( present & KopeteNotifyClient::Taskbar )
        notifyByTaskbar( winId );

    return uniqueId;
}

int KopeteNotifyClient::getPresentation(const QString &eventname)
{
	int present;
	if (eventname.isEmpty()) return Default;

	KConfig eventsfile( KopeteNotifyClient::instance()->instanceName()+".eventsrc", true, false);
	eventsfile.setGroup(eventname);

	present=eventsfile.readNumEntry("presentation", -1);

	return present;
}

QString KopeteNotifyClient::getFile(const QString &eventname, int present)
{
	if (eventname.isEmpty()) return QString::null;

	KConfig eventsfile( KopeteNotifyClient::instance()->instanceName()+".eventsrc", true, false);
	eventsfile.setGroup(eventname);

	switch (present)
	{
	case (Sound):
		return eventsfile.readPathEntry("soundfile");
	case (Logfile):
		return eventsfile.readPathEntry("logfile");
	}

	return QString::null;
}

int KopeteNotifyClient::getDefaultPresentation(const QString &eventname)
{
	int present;
	if (eventname.isEmpty()) return Default;

	KConfig eventsfile( KopeteNotifyClient::instance()->instanceName()+"/eventsrc", true, false, "data");
	eventsfile.setGroup(eventname);

	present=eventsfile.readNumEntry("default_presentation", -1);

	return present;

}

QString KopeteNotifyClient::getDefaultFile(const QString &eventname, int present)
{
	if (eventname.isEmpty()) return QString::null;

	KConfig eventsfile( KopeteNotifyClient::instance()->instanceName()+"/eventsrc", true, false, "data");
	eventsfile.setGroup(eventname);

	switch (present)
	{
	case (Sound):
		return eventsfile.readPathEntry("default_sound");
	case (Logfile):
		return eventsfile.readPathEntry("default_logfile");
	}

	return QString::null;
}

bool KopeteNotifyClient::startDaemon()
{
  static bool firstTry = true;
  if (firstTry && !kapp->dcopClient()->isApplicationRegistered(daemonName)) {
    firstTry = false;
    return KApplication::startServiceByDesktopName( QString::fromLatin1( daemonName ) ) == 0;
  }
  return true;
}


void KopeteNotifyClient::beep(const QString& reason)
{
  if ( !kapp || KopeteNotifyClient::Instance::currentInstance()->useSystemBell() ) {
    QApplication::beep();
    return;
  }

  DCOPClient *client=kapp->dcopClient();
  if (!client->isAttached())
  {
    client->attach();
    if (!client->isAttached() || !client->isApplicationRegistered(daemonName))
    {
      QApplication::beep();
      return;
    }
  }
  // The kaccess daemon handles visual and other audible beeps
  if ( client->isApplicationRegistered( "kaccess" ) )
  {
      QApplication::beep();
      return;
  }

  KopeteNotifyClient::event(KopeteNotifyClient::notification, reason);
}


KInstance * KopeteNotifyClient::instance() {
    return KopeteNotifyClient::Instance::current();
}


class KopeteNotifyClient::InstanceStack
{
public:
	InstanceStack() { m_defaultInstance = 0; }
	virtual ~InstanceStack() { delete m_defaultInstance; }
	void push(Instance *instance) { m_instances.push(instance); }

	void pop(Instance *instance)
	{
		if (m_instances.top() == instance)
			m_instances.pop();
		else if (!m_instances.isEmpty())
		{
			kdWarning(160) << "Tried to remove an Instance that is not the current," << endl;
			kdWarning(160) << "Resetting to the main KApplication." << endl;
			m_instances.clear();
		}
		else
			kdWarning(160) << "Tried to remove an Instance, but the stack was empty." << endl;
    }

	Instance *currentInstance()
	{
		if (m_instances.isEmpty())
		{
			m_defaultInstance = new Instance(kapp);
		}
		return m_instances.top();
	}

private:
	QPtrStack<Instance> m_instances;
	Instance *m_defaultInstance;
};

KopeteNotifyClient::InstanceStack * KopeteNotifyClient::Instance::s_instances = 0L;
static KStaticDeleter<KopeteNotifyClient::InstanceStack > instancesDeleter;

struct KopeteNotifyClient::InstancePrivate
{
    KInstance *instance;
    bool useSystemBell;
};

KopeteNotifyClient::Instance::Instance(KInstance *instance)
{
    d = new InstancePrivate;
    d->instance = instance;
    instances()->push(this);

    KConfig *config = instance->config();
    KConfigGroupSaver cs( config, "General" );
    d->useSystemBell = config->readBoolEntry( "UseSystemBell", false );
}

KopeteNotifyClient::Instance::~Instance()
{
	if (s_instances)
		s_instances->pop(this);
	delete d;
}

KopeteNotifyClient::InstanceStack *KopeteNotifyClient::Instance::instances()
{
	if (!s_instances)
		instancesDeleter.setObject(s_instances, new InstanceStack);
	return s_instances;
}

bool KopeteNotifyClient::Instance::useSystemBell() const
{
    return d->useSystemBell;
}


// static methods

// We always return a valid KopeteNotifyClient::Instance here. If no special one
// is available, we have a default-instance with kapp as KInstance.
// We make sure to always have that default-instance in the stack, because
// the stack might have gotten cleared in the destructor.
// We can't use QStack::setAutoDelete( true ), because no instance besides
// our default instance is owned by us.
KopeteNotifyClient::Instance * KopeteNotifyClient::Instance::currentInstance()
{
	return instances()->currentInstance();
}

KInstance *KopeteNotifyClient::Instance::current()
{
    return currentInstance()->d->instance;
}
