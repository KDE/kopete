#include <kdebug.h>
#include <kgenericfactory.h>
#include <kstandarddirs.h>

#include "kopetemessage.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetecontact.h"
#include "kopetemetacontact.h"

#include "msginfoplugin.h"

#include <qcolor.h>

K_EXPORT_COMPONENT_FACTORY( kopete_msginfo, KGenericFactory<MsgInfoPlugin>( "kopete_msginfo" )  )

MsgInfoPlugin::MsgInfoPlugin( QObject *parent, const char *name, const QStringList &/*args*/ )
: Kopete::Plugin( KGlobal::instance(), parent, name )
{
	connect( Kopete::MessageManagerFactory::self(),
		SIGNAL( aboutToDisplay( Kopete::Message & ) ),
		SLOT( slotProcessDisplay( Kopete::Message & ) ) );
	connect( Kopete::MessageManagerFactory::self(),
		SIGNAL( aboutToSend( Kopete::Message & ) ),
		SLOT( slotProcessSend( Kopete::Message & ) ) );
}

MsgInfoPlugin::~MsgInfoPlugin()
{
}

bool
MsgInfoPlugin::serialize( Kopete::MetaContact *metaContact,
			  QStringList &strList  ) const
{
	if ( mMsgCountMap.contains( metaContact ) )
		strList<< QString::number( mMsgCountMap[ metaContact ] );
	else
		strList<< "0";
	return true;
}

void
MsgInfoPlugin::deserialize( Kopete::MetaContact *metaContact, const QStringList& data )
{
	mMsgCountMap[ metaContact ] = data.first().toUInt();
}

void
MsgInfoPlugin::slotProcessDisplay( Kopete::Message& msg )
{
	//we got a message
	if ( msg.direction() == Kopete::Message::Inbound ) {
		Kopete::MetaContact *meta = msg.from()->metaContact();
		++(mMsgCountMap[ meta ]);
	}
	changeMessage( msg );
}

void
MsgInfoPlugin::slotProcessSend( Kopete::Message& msg )
{
	changeMessage( msg );
}

void
MsgInfoPlugin::changeMessage( Kopete::Message& msg )
{
	msg.setBody( msg.body().replace( "%K%", "Kopete - The best IM client") );
	msg.setBody( msg.body().replace( "%U%", "http://kopete.kde.org") );
	if ( msg.direction() == Kopete::Message::Inbound ) {
		int num = mMsgCountMap[ msg.from()->metaContact() ];
		msg.setBody( msg.body().replace( "%#%", QString::number(num) ) );
	} else {
		Kopete::MetaContact *meta = msg.to().first()->metaContact();
		int num = mMsgCountMap[ meta ];
		msg.setBody( msg.body().replace( "%#%", QString::number(num) ) );
	}
}

#include "msginfoplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:

