#include <kdebug.h>
#include <kgenericfactory.h>
#include <kstandarddirs.h>

#include "kopete.h"
#include "kopetemessage.h"
#include "kopetecontact.h"
#include "kopetemetacontact.h"

#include "msginfoplugin.h"

#include <qregexp.h>
#include <qcolor.h>

K_EXPORT_COMPONENT_FACTORY( kopete_msginfo, KGenericFactory<MsgInfoPlugin> );

MsgInfoPlugin::MsgInfoPlugin( QObject *parent, const char *name,
			      const QStringList &/*args*/ )
	: Plugin( parent, name )
{
	connect( kopeteapp, SIGNAL(aboutToDisplay(KopeteMessage&)),
		 SLOT(slotProcessDisplay(KopeteMessage&)) );
	connect( kopeteapp, SIGNAL(aboutToSend(KopeteMessage&)),
		 SLOT(slotProcessSend(KopeteMessage&)) );
}

MsgInfoPlugin::~MsgInfoPlugin()
{
}

void
MsgInfoPlugin::init()
{
}

bool
MsgInfoPlugin::unload()
{
	return true;
}

bool
MsgInfoPlugin::serialize( KopeteMetaContact *metaContact,
			  QStringList &strList  ) const
{
	if ( mMsgCountMap.contains( metaContact ) )
		strList<< QString::number( mMsgCountMap[ metaContact ] );
	else
		strList<< "0";
	return true;
}

void
MsgInfoPlugin::deserialize( KopeteMetaContact *metaContact, const QStringList& data )
{
	mMsgCountMap[ metaContact ] = data.first().toUInt();
}

void
MsgInfoPlugin::slotProcessDisplay( KopeteMessage& msg )
{
	//we got a message
	if ( msg.direction() == KopeteMessage::Inbound ) {
		KopeteMetaContact *meta = msg.from()->metaContact();
		++(mMsgCountMap[ meta ]);
	}
	changeMessage( msg );
}

void
MsgInfoPlugin::slotProcessSend( KopeteMessage& msg )
{
	changeMessage( msg );
}

void
MsgInfoPlugin::changeMessage( KopeteMessage& msg )
{
	msg.setBody( msg.body().replace( QRegExp( "%K%" ), "Kopete - The best IM client") );
	msg.setBody( msg.body().replace( QRegExp( "%U%" ), "http://kopete.kde.org") );
	if ( msg.direction() == KopeteMessage::Inbound ) {
		int num = mMsgCountMap[ msg.from()->metaContact() ];
		msg.setBody( msg.body().replace( QRegExp( "%#%" ), QString::number(num) ) );
	} else {
		KopeteMetaContact *meta = msg.to().first()->metaContact();
		int num = mMsgCountMap[ meta ];
		msg.setBody( msg.body().replace( QRegExp( "%#%" ), QString::number(num) ) );
	}
}





#include "msginfoplugin.moc"
