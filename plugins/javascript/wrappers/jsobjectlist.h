
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KOPETE_JSOBJECTLIST_H
#define KOPETE_JSOBJECTLIST_H

class JavaScriptIdentifier
{
public:
	static QString id( Kopete::Contact *c )
	{
		return c->contactId();
	}

	static QString id( Kopete::MetaContact *c )
	{
		return c->displayName();
	}

	static QString id( Kopete::Group *c )
	{
		return c->displayName();
	}
};

template< class ObjectT >
class ObjectList
{
public:
       	ObjectList( KJSEmbed::KJSEmbedPart *engine ) : exec( engine->globalExec() ), jsEngine( engine )
	{
		init();
	}

	ObjectList( KJSEmbed::KJSEmbedPart *engine, const QPtrList<ObjectT> &list )
		: exec( engine->globalExec() ), jsEngine( engine )
	{
		init();

		for( QPtrListIterator<ObjectT> it( list ); it.current(); ++it )
		{
			addObject( JavaScriptIdentifier::id( it.current() ), it.current() );
		}
	}

	ObjectList( KJSEmbed::KJSEmbedPart *engine, const QDict<ObjectT> &list )
		: exec( engine->globalExec() ), jsEngine( engine )
	{
		init();

		for( QDictIterator<ObjectT> it( list ); it.current(); ++it )
		{
			addObject( JavaScriptIdentifier::id( it.current() ), it.current() );
		}
	}

	void addObject( const QString &name, ObjectT *object )
	{
		//kDebug() << k_funcinfo << "Adding object " << name << endl;
		if( !objectList[name] )
		{
			KJS::List arg;

			KJS::Object jsObject = jsEngine->factory()->createProxy( exec, object );
			addChildObjects( jsObject, object );
			arg.append( jsObject );
			pushMethod.call( exec, jsArray, arg );
			objectList.insert( name, object );
		}
	}

	void addChildObjects( KJS::Object &jsObject, Kopete::MetaContact *object )
	{
		ObjectList<Kopete::Contact> *contacts = new ObjectList<Kopete::Contact>(
			jsEngine, object->contacts()
		);

		wrapperList.insert( JavaScriptIdentifier::id( object ), contacts );
		jsObject.put( exec, "contacts", contacts->array() );

		/*ObjectList<Kopete::Group> *groups = new ObjectList<Kopete::Group>(
			jsEngine, object->groups()
		);

		wrapperList.insert( JavaScriptIdentifier::id( object ), groups );
		jsObject.put( exec, "groups", groups->array() );*/
	}

	void addChildObjects( KJS::Object &jsObject, Kopete::Contact *object )
	{
		JSStatus* status = new JSStatus( object->onlineStatus() );
		wrapperList.insert( JavaScriptIdentifier::id( object ), status );
		jsObject.put( exec, "onlineStatus", KJS::Object( status ) );
	}

	void addChildObjects( KJS::Object &, Kopete::Group * ){}

	void removeObject( const QString &name )
	{
		KJS::Object jsObject = object( name );
		byNameObject.deleteProperty( exec, name );
		objectList.remove( name );
		wrapperList.remove( name );
	}

	KJS::Object &array()
	{
		return jsArray;
	}

	ObjectT *objectPointer( const QString &name ) const
	{
		return objectList[ name ];
	}

private:
	void init()
	{
		KJS::List arg;
		jsArray = jsEngine->interpreter()->builtinArray().construct( exec, arg );
		pushMethod = jsArray.get( exec, "push" ).toObject( exec );

		wrapperList.setAutoDelete(true);
	}

	KJS::Object jsArray;
	KJS::Object pushMethod;
	KJS::ExecState *exec;
	KJSEmbed::KJSEmbedPart *jsEngine;
	QDict<ObjectT>	objectList;
	QDict<void>	wrapperList;
};

#endif

