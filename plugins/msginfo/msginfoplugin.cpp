#include <kdebug.h>
#include <kgenericfactory.h>
#include <kstandarddirs.h>

#include "kopete.h"
#include "kopetemessage.h"
#include "kopetecontact.h"

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
MsgInfoPlugin::serialize()
{
	return false;
}

void
MsgInfoPlugin::deserialize()
{
}

void
MsgInfoPlugin::slotProcessDisplay( KopeteMessage& msg )
{
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
}





