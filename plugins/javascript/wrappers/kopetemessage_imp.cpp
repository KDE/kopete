// -*- c++ -*-

/*
 *  Copyright (C) 2003, Ian Reinhart Geiser <geiseri@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#include <kdebug.h>

#include <kjsembed/jsopaqueproxy.h>
#include <kjsembed/jsbinding.h>
#include <kjsembed/jsfactory.h>
#include <kjsembed/jsfactory_imp.h>
#include <kjsembed/kjsembedpart.h>
#include <kjsembed/customobject_imp.h>

#include "kopetemessage_imp.h"

namespace KJSEmbed {
namespace Bindings {

KopeteMessageLoader::KopeteMessageLoader( QObject *parent, const char *name, const QStringList &args ) :
	JSBindingPlugin(parent, name, args)
{
}

KJS::Object KopeteMessageLoader::createBinding(KJSEmbedPart *, KJS::ExecState *, const KJS::List &) const
{
	KJS::Object tmp;
	return tmp;
}

KopeteMessageImp::KopeteMessageImp( KJS::ExecState *exec, int id ) : JSProxyImp(exec), mid(id)
{
}

KopeteMessageImp::~KopeteMessageImp()
{
}

void KopeteMessageImp::addBindings( KJS::ExecState *exec, KJS::Object &object )
{
	Kopete::Message *msg = message(object);
	if( msg )
	{
		JSProxy::MethodTable methods[] =
		{
			{ bgColor, "bgColor"},
			{ setBgColor, "setbgColor"},
			{ fgColor,  "fgColor"},
			{ setFgColor, "setFgColor"},
			{ font,  "font"},
			{ setFont, "setFont"},
			{ plainBody,  "plainBody"},
			{ richBody, "richBody"},
			{ setPlainBody,  "setPlainBody"},
			{ setRichBody, "setRichBody"},
			{ importance, "importance"},
			{ setImportance, "setImportance"},
			{ type, "type"},
			{ asXML, "asXML"},
			{ 0, 0 }
		};

		int idx = 0;
		do
		{
			KopeteMessageImp *m = new KopeteMessageImp( exec, methods[idx].id );
			object.put( exec , methods[idx].name, KJS::Object(m) );
			++idx;
		}
		while( methods[idx].id );
	}
}

Kopete::Message *KopeteMessageImp::message( KJS::Object &object )
{
	JSOpaqueProxy *op = JSProxy::toOpaqueProxy( object.imp() );
	if ( !op )
	{
		kdWarning() << "MyCustomObjectImp::call() failed, not a JSOpaqueProxy" << endl;
		return 0;
	}

	if ( op->typeName() != "MyCustomObject" )
	{
		kdWarning() << "MyCustomObjectImp::call() failed, type is " << op->typeName() << endl;
		return 0;
	}

	return (Kopete::Message*)op->toVoidStar();
}

KJS::Value KopeteMessageImp::call( KJS::ExecState *exec, KJS::Object &self, const KJS::List &args )
{
	Kopete::Message *msg = message( self );
	if( msg )
	{
		switch( mid )
		{
			case bgColor:
				return KJS::String( msg->bg().name() );

			case setBgColor:
				msg->setBg( QColor( args[0].toString(exec).qstring() ) );

			case fgColor:
				return KJS::String( msg->bg().name() );

			case setFgColor:
				msg->setFg( QColor( args[0].toString(exec).qstring() ) );

			case font:
				return KJS::String( msg->font().toString() );

			case setFont:
				msg->setFont( QFont( args[0].toString(exec).qstring() ) );

			case plainBody:
				return KJS::String( msg->plainBody() );

			case setPlainBody:
				msg->setBody( args[0].toString(exec).qstring(), Kopete::Message::PlainText );

			case richBody:
				return KJS::String( msg->parsedBody() );

			case setRichBody:
				msg->setBody( args[0].toString(exec).qstring(), Kopete::Message::RichText );

			case importance:
				return KJS::Number( msg->importance() );

			case setImportance:
				msg->setImportance( Kopete::Message::MessageImportance(args[0].toInteger(exec)) );

			case type:
				return KJS::Number( msg->type() );

			case asXML:
				return KJS::String( msg->asXML().toString() );
			//case from:
			//return KJS::Object( new JSContact( const_cast<Kopete::Contact*>( msg->from() ) ) );
		}
	}

	return KJS::Value();
}

int KopeteMessageImp::extractInt( KJS::ExecState *exec, const KJS::List &args, int idx)
{
     return (args.size() > idx) ? args[idx].toInteger(exec) : 0;
}
QString KopeteMessageImp::extractString(KJS::ExecState *exec, const KJS::List &args, int idx)
{
     return (args.size() > idx) ? args[idx].toString(exec).qstring() : QString::null;
}

} // namespace KJSEmbed::Bindings
} // namespace KJSEmbed

#include <kgenericfactory.h>
typedef KGenericFactory<KJSEmbed::Bindings::KopeteMessageLoader> KopeteMessageLoaderFactory;
K_EXPORT_COMPONENT_FACTORY( libkopetemessageplugin, KopeteMessageLoaderFactory( "KopeteMessageLoader" ) );

