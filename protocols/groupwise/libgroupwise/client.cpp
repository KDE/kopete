#include <qapplication.h>

#include "gwclientstream.h"
#include "requestfactory.h"
#include "task.h"
#include "tasks/conferencetask.h"
#include "tasks/getdetailstask.h"
#include "tasks/getstatustask.h"
#include "tasks/logintask.h"
#include "tasks/setstatustask.h"
#include "tasks/statustask.h"

#include "client.h"

class Client::ClientPrivate
{
public:
	ClientPrivate() {}

	ClientStream *stream;
	int id_seed;
	Task *root;
	QString host, user, pass, resource;
	QString osname, tzname, clientName, clientVersion;
	int tzoffset;
	bool active;
	RequestFactory * requestFactory;
/*	LiveRoster roster;
	ResourceList resourceList;
	QValueList<GroupChat> groupChatList;*/
};

Client::Client(QObject *par)
:QObject(par, "groupwiseclient" )
{
	d = new ClientPrivate;
	d->tzoffset = 0;
	d->active = false;
	d->osname = "N/A";
	d->clientName = "N/A";
	d->clientVersion = "0.0";

	d->root = new Task(this, true);
	d->requestFactory = new RequestFactory;
	d->stream = 0;
}

Client::~Client()
{
	close();
	delete d->root;
	delete d->requestFactory;
	delete d;
}

void Client::connectToServer( ClientStream *s, const NovellDN &server, bool auth )
{
	d->stream = s;
	//connect(d->stream, SIGNAL(connected()), SLOT(streamConnected()));
	//connect(d->stream, SIGNAL(handshaken()), SLOT(streamHandshaken()));
	connect(d->stream, SIGNAL(error(int)), SLOT(streamError(int)));
	//connect(d->stream, SIGNAL(sslCertificateReady(const QSSLCert &)), SLOT(streamSSLCertificateReady(const QSSLCert &)));
	connect(d->stream, SIGNAL(readyRead()), SLOT(streamReadyRead()));
	//connect(d->stream, SIGNAL(closeFinished()), SLOT(streamCloseFinished()));

	d->stream->connectToServer(server, auth);
}

void Client::start( const QString &host, const QString &user, const QString &pass )
{
	d->host = host;
	d->user = user;
	d->pass = pass;

	initialiseEventTasks();
	
	LoginTask * login = new LoginTask( d->root );
	
	connect( login, SIGNAL( gotMyself( const ContactItem &  ) ), 
			this, SIGNAL( accountDataReceived( const ContactItem & ) ) );
			
	connect( login, SIGNAL( gotFolder( const FolderItem & ) ), 
			this, SIGNAL( folderReceived( const FolderItem & ) ) );
			
	connect( login, SIGNAL( gotContact( const ContactItem &  ) ), 
			this, SIGNAL( contactReceived( const ContactItem &  ) ) );
			
	connect( login, SIGNAL( gotContactUserDetails( const ContactDetails & ) ), 
			this, SIGNAL( contactUserDetailsReceived( const ContactDetails & ) ) ) ;
			
	connect( login, SIGNAL( finished() ), this, SLOT( lt_loginFinished() ) );
	
	login->initialise();
	login->go( true );
	
// 	Status stat;
// 	stat.setIsAvailable(false);
// 
// 	JT_PushPresence *pp = new JT_PushPresence(rootTask());
// 	connect(pp, SIGNAL(subscription(const Jid &, const QString &)), SLOT(ppSubscription(const Jid &, const QString &)));
// 	connect(pp, SIGNAL(presence(const Jid &, const Status &)), SLOT(ppPresence(const Jid &, const Status &)));
// 
// 	JT_PushMessage *pm = new JT_PushMessage(rootTask());
// 	connect(pm, SIGNAL(message(const Message &)), SLOT(pmMessage(const Message &)));
// 
// 	JT_PushRoster *pr = new JT_PushRoster(rootTask());
// 	connect(pr, SIGNAL(roster(const Roster &)), SLOT(prRoster(const Roster &)));
// 
// 	new JT_ServInfo(rootTask());

	d->active = true;

}

void Client::close()
{
	qDebug( "TODO: close()" );
	if(d->stream) {
		if(d->active) {
/*			for(QValueList<GroupChat>::Iterator it = d->groupChatList.begin(); it != d->groupChatList.end(); it++) {
				GroupChat &i = *it;
				i.status = GroupChat::Closing;

				JT_Presence *j = new JT_Presence(rootTask());
				Status s;
				s.setIsAvailable(false);
				j->pres(i.j, s);
				j->go(true);
			}*/
		}

		d->stream->disconnect(this);
		d->stream->close();
		d->stream = 0;
	}
// 	disconnected();
// 	cleanup();
}

void Client::initialiseEventTasks()
{
	StatusTask * st = new StatusTask( d->root ); // FIXME - add an additional EventRoot?
	connect( st, SIGNAL( gotStatus( const QString &, Q_UINT16, const QString & ) ), SIGNAL( statusReceived( const QString &, Q_UINT16, const QString & ) ) );
	ConferenceTask * ct = new ConferenceTask( d->root ); 
	connect( ct, SIGNAL( message( const ConferenceEvent & ) ), SIGNAL( messageReceived( const ConferenceEvent & ) ) );
	
}

void Client::setStatus( GroupWise::Status status, const QString & reason )
{
	SetStatusTask * sst = new SetStatusTask( d->root );
	sst->status( status, reason, QString::null );
	connect( sst, SIGNAL( finished() ), this, SLOT( sst_statusChanged() ) );
	sst->go( true );
	// TODO: set status change in progress flag
	
}

void Client::requestStatus( const QString & userDN )
{
	GetStatusTask * gst = new GetStatusTask( d->root );
	gst->userDN( userDN );
	connect( gst, SIGNAL( gotStatus( const QString &, Q_UINT16, const QString & ) ), SIGNAL( statusReceived( const QString &, Q_UINT16, const QString & ) ) );
	gst->go( true );
}

void Client::sendMessage( const Message &message )
{
	//TODO Implement sendMessage
	qDebug( "TODO: sendMessage" );
}

void Client::sendTyping( /*Conference &conference ,*/ bool typing )
{
	//TODO Implement sendTyping
	qDebug( "TODO: sendTyping" );
}

void Client::requestDetails( const QStringList & userDNs )
{
	GetDetailsTask * gdt = new GetDetailsTask( d->root );
	gdt->userDNs( userDNs );
	connect( gdt, SIGNAL( gotContactUserDetails( const ContactDetails & ) ), 
			this, SIGNAL( contactUserDetailsReceived( const ContactDetails & ) ) );
	gdt->go( true );
}

// SLOTS //
void Client::streamError( int error )
{
	qDebug( "CLIENT ERROR (Error %i)", error );
}

void Client::streamReadyRead()
{
	qDebug( "CLIENT STREAM READY READ" );
	// take the incoming transfer and distribute it to the task tree
	Transfer * transfer = d->stream->read();
	distribute( transfer );
}

void Client::lt_loginFinished()
{
	qDebug( "got login finished" );
	const LoginTask * lt = (LoginTask *)sender();
	if ( lt->success() )
	{
		qDebug( "LOGIN SUCCEEDED" );
		SetStatusTask * sst = new SetStatusTask( d->root );
		sst->status( GroupWise::Available, QString::null, QString::null );
		sst->go( true );
		emit loggedIn();
	}
	// otherwise client should disconnect and signal failure that way??
}

void Client::sst_statusChanged()
{
	const SetStatusTask * sst = (SetStatusTask *)sender();
	if ( sst->success() )
	{
		qDebug( "status change succeeded" );
		emit ourStatusChanged( sst->requestedStatus(), sst->awayMessage(), sst->autoReply() );
	}
}

// INTERNALS //

QString Client::userId()
{
	return "maeuschen";
}

QString Client::userDN()
{
	return "cn=maeuschen,o=suse";
}

QString Client::password()
{
	return "maeuschen";
}

QString Client::userAgent()
{
	return "libgroupwise/0.1 (Linux, 2.6.5-7.97-smp)";
}

QCString Client::ipAddress()
{
	return "10.10.11.103";
}

void Client::distribute( Transfer * transfer )
{
	if( !rootTask()->take( transfer ) )
		qDebug( "CLIENT: root task refused transfer" );
}

void Client::send( Request * request )
{
	qDebug( "CLIENT::send()" );
	if( !d->stream )
	{	
		qDebug( "CLIENT - NO STREAM TO SEND ON!");
		return;
	}
// 	QString out = request.toString();
// 	debug(QString("Client: outgoing: [\n%1]\n").arg(out));
// 	xmlOutgoing(out);

	d->stream->write( request );
}

void Client::debug(const QString &str)
{
	qDebug( "CLIENT: %s", str.ascii() );
}

QString Client::genUniqueId()
{
	QString s;
	s.sprintf("a%x", d->id_seed);
	d->id_seed += 0x10;
	return s;
}

RequestFactory * Client::requestFactory()
{
	return d->requestFactory;
}

Task * Client::rootTask()
{
	return d->root;
}

#include "client.moc"
