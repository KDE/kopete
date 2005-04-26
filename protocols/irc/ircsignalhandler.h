
/*
    ircsignalhandler.h - Maps signals from the IRC engine to contacts

    Copyright (c) 2004      by Jason Keirstead <jason@keirstead.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef _IRC_SIGNAL_HANDLER_H
#define _IRC_SIGNAL_HANDLER_H

#include <qobject.h>
#include <qstringlist.h>
#include <qdatetime.h>

#include <kdebug.h>

#include "ircaccount.h"
#include "irccontactmanager.h"

/***
* IRC Signal handler. Mapps a KIRC engine signal to the right contact. Avoids
* Having a signal connected to 500+ slots where only one is valid, instead
* uses the contact dictionary.
*
* Warning: This file has a lot of black magic in it. Avoid it if
* you don't want your eyes to bleed. More below...
*
* Define some templated classes and methods to map a KIRC signal to the
* right contact. Having these templates greatly cuts down *A LOT* on the amount of
* code that would need to be in the signal mapper, at the expense of some readability.
*
* There are four IRCSignalMapping classes, one each for signals with 0, 1, 2,
* and 3 arguments ( plus the contact ID ). The classes take the signal, look
* up the contact it is for, and call the function passed into the class by the
* mapping function.
*
* Since QObjects cannot be inside templates, the QMember classes that connect
* to the slots are seperate.
*/

/*** Pre-declare mapping types for the QObjects **/
struct IRCSignalMappingBase{};

struct IRCSignalMappingT : IRCSignalMappingBase
{
	virtual void exec( const QString & ) = 0;
	virtual ~IRCSignalMappingT() {};
};

struct IRCSignalMappingSingleT : IRCSignalMappingBase
{
	virtual void exec( const QString &, const QString & ) = 0;
	virtual ~IRCSignalMappingSingleT() {};
};

struct IRCSignalMappingDoubleT : IRCSignalMappingBase
{
	virtual void exec( const QString &, const QString &, const QString & ) = 0;
	virtual ~IRCSignalMappingDoubleT() {};
};

struct IRCSignalMappingTripleT : IRCSignalMappingBase
{
	virtual void exec( const QString &, const QString &, const QString &, const QString & ) = 0;
	virtual ~IRCSignalMappingTripleT() {};
};

/***
QObject members, these connect to the KIRC signals and call
the Mapping functions when they emit.
**/

class QMember : public QObject
{
	Q_OBJECT

	public:
		QMember( IRCSignalMappingT *m, QObject *p ) : QObject( p ), mapping( m ){};

	public slots:
		void slotEmit( const QString &id )
		{
			//kdDebug(14120) << k_funcinfo << id << endl;
			mapping->exec(id);
		}

	private:
		IRCSignalMappingT *mapping;
};

class QMemberSingle : public QObject
{
	Q_OBJECT

	public:
		QMemberSingle( IRCSignalMappingSingleT *m, QObject *p ) : QObject( p ), mapping( m ){}

	public slots:
		void slotEmit( const QString &id, const QString &arg )
		{
			//kdDebug(14120) << k_funcinfo << id << " : " << arg  << endl;
			mapping->exec(id,arg);
		}

	private:
		IRCSignalMappingSingleT *mapping;
};

class QMemberDouble : public QObject
{
	Q_OBJECT

	public:
		QMemberDouble( IRCSignalMappingDoubleT *m, QObject *p ) : QObject( p ), mapping( m ){}

	public slots:
		void slotEmit( const QString &id, const QString &arg, const QString &arg2 )
		{
			//kdDebug(14120) << k_funcinfo << id << " : " << arg << " : " << arg2 << endl;
			mapping->exec(id,arg,arg2);
		}

	private:
		IRCSignalMappingDoubleT *mapping;
};

class QMemberTriple : public QObject
{
	Q_OBJECT

	public:
		QMemberTriple( IRCSignalMappingTripleT *m, QObject *p ) : QObject( p ), mapping( m ){}

	public slots:
		void slotEmit( const QString &id, const QString &arg, const QString &arg2, const QString &arg3 )
		{
			//kdDebug(14120) << k_funcinfo << id << " : " << arg << " : " << arg2 << " : " << arg3 << endl;
			mapping->exec(id,arg,arg2,arg3);
		}

	private:
		IRCSignalMappingTripleT *mapping;
};

/***
Mapping classes. These contain pointers to the functions to call. We first
look up the right contact in the contact manager's dictionary, and then
call the method
**/

template <class TClass>
class IRCSignalMapping : public IRCSignalMappingT
{
	public:
		IRCSignalMapping( IRCContactManager *mgr, const char * /*signal*/,
			void (TClass::*m)() ) : manager(mgr), method(m){}

		void exec( const QString &id )
		{
			TClass *c = (TClass*)manager->findContact( id );
			if( c )
			{
				void (TClass::*tmp)() = (void (TClass::*)())method;
				(*c.*tmp)();
			}
		}

	private:
		IRCContactManager *manager;
		void (TClass::*method)();
};

template <class TClass>
class IRCSignalMappingSingle : public IRCSignalMappingSingleT
{
	public:
		IRCSignalMappingSingle<TClass>( IRCContactManager *mgr, const char * /*signal*/,
			void (TClass::*m)(const QString&) ) : manager(mgr), method(m){}

		void exec( const QString &id, const QString &arg )
		{
			TClass *c = (TClass*)manager->findContact( id );
			if( c )
			{
				void (TClass::*tmp)(const QString&) = (void (TClass::*)(const QString&))method;
				(*c.*tmp)( arg );
			}
		}

	private:
		IRCContactManager *manager;
		void (TClass::*method)(const QString &);
};

template <class TClass>
class IRCSignalMappingDouble : public IRCSignalMappingDoubleT
{
	public:
		IRCSignalMappingDouble<TClass>( IRCContactManager *mgr, const char * /*signal*/,
			void (TClass::*m)(const QString&,const QString&) ) : manager(mgr), method(m){}

		void exec( const QString &id,const QString &arg, const QString &arg2 )
		{
			TClass *c = (TClass*)manager->findContact( id );
			if( c )
			{
				void (TClass::*tmp)(const QString&,const QString&) =
					(void (TClass::*)(const QString&,const QString&))method;
				(*c.*tmp)(arg,arg2);
			}
		}

	private:
		IRCContactManager *manager;
		void (TClass::*method)(const QString &,const QString &);
};

template <class TClass>
class IRCSignalMappingTriple : public IRCSignalMappingTripleT
{
	public:
		IRCSignalMappingTriple<TClass>( IRCContactManager *mgr, const char * /*signal*/,
			void (TClass::*m)(const QString&,const QString&,const QString&) )
			: manager(mgr), method(m){}

		void exec( const QString &id,const QString&arg, const QString &arg2, const QString &arg3 )
		{
			TClass *c = (TClass*)manager->findContact( id );
			if( c )
			{
				void (TClass::*tmp)(const QString&,const QString&,const QString&) =
					(void (TClass::*)(const QString&,const QString&,const QString&))method;
				(*c.*tmp)(arg,arg2,arg3);
			}
		}

	private:
		IRCContactManager *manager;
		void (TClass::*method)(const QString &,const QString &,const QString &);
};

class IRCSignalHandler : public QObject
{
	Q_OBJECT

	public:
		IRCSignalHandler( IRCContactManager *manager );
		~IRCSignalHandler();

	private slots:

		/****
		Slots for signals with non-QString types
		*/

		//Channel contact slots
		void slotNamesList( const QString &, const QStringList & );
		void slotEndOfNames( const QString & );
		void slotTopicUser( const QString &, const QString&, const QDateTime &);

		//User contact slots
		void slotNewWhoIsIdle(const QString &, unsigned long  );
		void slotNewWhoReply(const QString &, const QString &, const QString &, const QString &,
			const QString &, bool , const QString &, uint , const QString & );

	private:
		IRCContactManager *manager;
		QValueList<IRCSignalMappingBase*> mappings;

		/****
		Signal mapping functions
		*/

		template <class TClass>
		inline void map( IRCContactManager *m, const char* signal, void (TClass::*method)() )
		{
			IRCSignalMappingT *mapping = new IRCSignalMapping<TClass>( m, signal, method );
			mappings.append(mapping);
			QObject::connect( static_cast<IRCAccount*>( m->mySelf()->account() )->engine(), signal,
				new QMember( mapping, this),
				SLOT( slotEmit( const QString &) )
			);
		}

		template <class TClass>
		inline void mapSingle( IRCContactManager *m,
			const char* signal, void (TClass::*method)(const QString&) )
		{
			IRCSignalMappingSingleT *mapping = new IRCSignalMappingSingle<TClass>( m, signal, method );
			mappings.append(mapping);
			QObject::connect( static_cast<IRCAccount*>( m->mySelf()->account() )->engine(), signal,
				new QMemberSingle( mapping, this),
				SLOT( slotEmit( const QString &, const QString &) )
			);
		}

		template <class TClass>
		inline void mapDouble( IRCContactManager *m,
			const char* signal, void (TClass::*method)(const QString&,const QString&) )
		{
			IRCSignalMappingDoubleT *mapping = new IRCSignalMappingDouble<TClass>( m, signal, method );
			mappings.append(mapping);
			QObject::connect( static_cast<IRCAccount*>( m->mySelf()->account() )->engine(), signal,
				new QMemberDouble( mapping, this),
				SLOT( slotEmit( const QString &, const QString &,const QString &) )
			);
		}

		template <class TClass>
		inline void mapTriple( IRCContactManager *m,
			const char* signal,
			void (TClass::*method)(const QString&,const QString &, const QString &) )
		{
			IRCSignalMappingTripleT *mapping = new IRCSignalMappingTriple<TClass>( m, signal, method );
			mappings.append(mapping);
			QObject::connect( static_cast<IRCAccount*>( m->mySelf()->account() )->engine(), signal,
				new QMemberTriple( mapping, this),
				SLOT( slotEmit( const QString &, const QString &,const QString &,const QString &) )
			);
		}
};

#endif
