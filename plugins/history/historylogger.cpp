/*
    historylogger.cpp

    Copyright (c) 2003-2004 by Olivier Goffart        <ogoffart @ kde.org>

    Kopete    (c) 2003-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "historylogger.h"
#include "historyconfig.h"

#include <qregexp.h>
#include <qfile.h>
#include <qdir.h>
#include <qdatetime.h>
#include <qdom.h>
#include <qtimer.h>

#include <kdebug.h>
#include <kstandarddirs.h>
#include <ksavefile.h>

#include "kopeteglobal.h"
#include "kopetecontact.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopetemetacontact.h"
#include "kopetechatsession.h"

// -----------------------------------------------------------------------------
HistoryLogger::HistoryLogger( Kopete::MetaContact *m,  QObject *parent, const char *name )
 : QObject(parent, name)
{
	m_saveTimer=0L;
	m_saveTimerTime=0;
	m_metaContact=m;
	m_hideOutgoing=false;
	m_cachedMonth=-1;
	m_realMonth=QDate::currentDate().month();
	m_oldSens=Default;

	//the contact may be destroyed, for example, if the contact changes its metacontact
	connect(m_metaContact , SIGNAL(destroyed(QObject *)) , this , SLOT(slotMCDeleted()));

	setPositionToLast();
}


HistoryLogger::HistoryLogger( Kopete::Contact *c,  QObject *parent, const char *name )
 : QObject(parent, name)
{
	m_saveTimer=0L;
	m_saveTimerTime=0;
	m_cachedMonth=-1;
	m_metaContact=c->metaContact();
	m_hideOutgoing=false;
	m_realMonth=QDate::currentDate().month();
	m_oldSens=Default;

	//the contact may be destroyed, for example, if the contact changes its metacontact
	connect(m_metaContact , SIGNAL(destroyed(QObject *)) , this , SLOT(slotMCDeleted()));

	setPositionToLast();
}


HistoryLogger::~HistoryLogger()
{
	if(m_saveTimer && m_saveTimer->isActive())
		saveToDisk();
}


void HistoryLogger::setPositionToLast()
{
	setCurrentMonth(0);
	m_oldSens = AntiChronological;
	m_oldMonth=0;
	m_oldElements.clear();
}


void HistoryLogger::setPositionToFirst()
{
	setCurrentMonth( getFirstMonth() );
	m_oldSens = Chronological;
	m_oldMonth=m_currentMonth;
	m_oldElements.clear();
}


void HistoryLogger::setCurrentMonth(int month)
{
	m_currentMonth = month;
	m_currentElements.clear();
}


QDomDocument HistoryLogger::getDocument(const Kopete::Contact *c, unsigned int month , bool canLoad , bool* contain)
{
	if(m_realMonth!=QDate::currentDate().month())
	{ //We changed month, our indice are not correct anymore, clean memory.
	  // or we will see what i called "the 31 midnight bug"(TM) :-)  -Olivier
		m_documents.clear();
		m_cachedMonth=-1;
		m_currentMonth++; //Not usre it's ok, but should work;
		m_oldMonth++;     // idem
		m_realMonth=QDate::currentDate().month();
	}

	if(!m_metaContact)
	{ //this may happen if the contact has been moved, and the MC deleted
		if(c && c->metaContact())
			m_metaContact=c->metaContact();
		else
			return QDomDocument();
	}

	if(!m_metaContact->contacts().contains(c))
	{
		if(contain)
			*contain=false;
		return QDomDocument();
	}

	QMap<unsigned int , QDomDocument> documents = m_documents[c];
	if (documents.contains(month))
		return documents[month];


	QDomDocument doc =  getDocument(c, QDate::currentDate().addMonths(0-month), canLoad, contain);

	documents.insert(month, doc);
	m_documents[c]=documents;

	return doc;

}

QDomDocument HistoryLogger::getDocument(const Kopete::Contact *c, const QDate date , bool canLoad , bool* contain)
{
	if(!m_metaContact)
	{ //this may happen if the contact has been moved, and the MC deleted
		if(c && c->metaContact())
			m_metaContact=c->metaContact();
		else
			return QDomDocument();
	}

	if(!m_metaContact->contacts().contains(c))
	{
		if(contain)
			*contain=false;
		return QDomDocument();
	}

	if(!canLoad)
	{
		if(contain)
			*contain=false;
		return QDomDocument();
	}

	QString	FileName = getFileName(c, date);

	QDomDocument doc( "Kopete-History" );

	QFile file( FileName );
	if ( !file.open( IO_ReadOnly ) )
	{
		if(contain)
			*contain=false;
		return doc;
	}
	if ( !doc.setContent( &file ) )
	{
		file.close();
		if(contain)
			*contain=false;
		return doc;
	}
	file.close();

	if(contain)
		*contain=true;

	return doc;
}


void HistoryLogger::appendMessage( const Kopete::Message &msg , const Kopete::Contact *ct )
{
	if(!msg.from())
		return;

	// If no contact are given: If the manager is availiable, use the manager's
	// first contact (the channel on irc, or the other contact for others protocols
	const Kopete::Contact *c = ct;
	if(!c && msg.manager() )
	{
		QPtrList<Kopete::Contact> mb=msg.manager()->members() ;
		c = mb.first();
	}
	if(!c)  //If the contact is still not initialized, use the message author.
		c =   msg.direction()==Kopete::Message::Outbound ? msg.to().first() : msg.from()  ;


	if(!m_metaContact)
	{ //this may happen if the contact has been moved, and the MC deleted
		if(c && c->metaContact())
			m_metaContact=c->metaContact();
		else
			return;
	}


	if(!c || !m_metaContact->contacts().contains(c) )
	{
		/*QPtrList<Kopete::Contact> contacts= m_metaContact->contacts();
		QPtrListIterator<Kopete::Contact> it( contacts );
		for( ; it.current(); ++it )
		{
			if( (*it)->protocol()->pluginId() == msg.from()->protocol()->pluginId() )
			{
				c=*it;
				break;
			}
		}*/
		//if(!c)

		kdWarning(14310) << k_funcinfo << "No contact found in this metacontact to" <<
			" append in the history" << endl;
		return;
	}

	QDomDocument doc=getDocument(c,0);
	QDomElement docElem = doc.documentElement();

	if(docElem.isNull())
	{
		docElem= doc.createElement( "kopete-history" );
		docElem.setAttribute ( "version" , "0.9" );
		doc.appendChild( docElem );
		QDomElement headElem = doc.createElement( "head" );
		docElem.appendChild( headElem );
		QDomElement dateElem = doc.createElement( "date" );
		dateElem.setAttribute( "year",  QString::number(QDate::currentDate().year()) );
		dateElem.setAttribute( "month", QString::number(QDate::currentDate().month()) );
		headElem.appendChild(dateElem);
		QDomElement myselfElem = doc.createElement( "contact" );
		myselfElem.setAttribute( "type",  "myself" );
		myselfElem.setAttribute( "contactId", c->account()->myself()->contactId() );
		headElem.appendChild(myselfElem);
		QDomElement contactElem = doc.createElement( "contact" );
		contactElem.setAttribute( "contactId", c->contactId() );
		headElem.appendChild(contactElem);
	}

	QDomElement msgElem = doc.createElement( "msg" );
	msgElem.setAttribute( "in",  msg.direction()==Kopete::Message::Outbound ? "0" : "1" );
	msgElem.setAttribute( "from",  msg.from()->contactId() );
	msgElem.setAttribute( "nick",  msg.from()->property( Kopete::Global::Properties::self()->nickName() ).value().toString() ); //do we have to set this?
	msgElem.setAttribute( "time", msg.timestamp().toString("d h:m:s") );

	QDomText msgNode = doc.createTextNode( msg.plainBody() );
	docElem.appendChild( msgElem );
	msgElem.appendChild( msgNode );


	// I'm temporizing the save.
	// On hight-traffic channel, saving can take lots of CPU. (because the file is big)
	// So i wait a time proportional to the time needed to save..

	const QString filename=getFileName(c,QDate::currentDate());
	if(!m_toSaveFileName.isEmpty() && m_toSaveFileName != filename)
	{ //that mean the contact or the month has changed, save it now.
		saveToDisk();
	}

	m_toSaveFileName=filename;
	m_toSaveDocument=doc;

	if(!m_saveTimer)
	{
		m_saveTimer=new QTimer(this);
		connect( m_saveTimer, SIGNAL( timeout() ) , this, SLOT(saveToDisk()) );
	}
	if(!m_saveTimer->isActive())
		m_saveTimer->start( m_saveTimerTime, true /*singleshot*/ );
}

void HistoryLogger::saveToDisk()
{
	if(m_saveTimer)
		m_saveTimer->stop();
	if(m_toSaveFileName.isEmpty() || m_toSaveDocument.isNull())
		return;

	QTime t;
	t.start(); //mesure the time needed to save.

	KSaveFile file( m_toSaveFileName );
	if( file.status() == 0 )
	{
		QTextStream *stream = file.textStream();
		//stream->setEncoding( QTextStream::UnicodeUTF8 ); //???? oui ou non?
		m_toSaveDocument.save( *stream, 1 );
		file.close();

		m_saveTimerTime=QMIN(t.elapsed()*1000, 300000);
		    //a time 1000 times supperior to the time needed to save.  but with a upper limit of 5 minutes
		//on a my machine, (2.4Ghz, but old HD) it should take about 10 ms to save the file.
		// So that would mean save every 10 seconds, which seems to be ok.
		// But it may take 500 ms if the file to save becomes too big (1Mb).
		kdDebug(14310) << k_funcinfo << m_toSaveFileName << " saved in " << t.elapsed() << " ms " <<endl ;

		m_toSaveFileName=QString::null;
		m_toSaveDocument=QDomDocument();
	}
	else
		kdError(14310) << k_funcinfo << "impossible to save the history file " << m_toSaveFileName << endl;

}

QValueList<Kopete::Message> HistoryLogger::readMessages(QDate date)
{
	QRegExp rxTime("(\\d+) (\\d+):(\\d+)($|:)(\\d*)"); //(with a 0.7.x compatibility)
	QValueList<Kopete::Message> messages;


	QPtrList<Kopete::Contact> ct=m_metaContact->contacts();
	QPtrListIterator<Kopete::Contact> it( ct );

	for( ; it.current(); ++it )
	{
		QDomDocument doc=getDocument(*it,date, true, 0L);
		QDomElement docElem = doc.documentElement();
		QDomNode n = docElem.firstChild();

		while(!n.isNull())
		{
			QDomElement  msgElem2 = n.toElement();
			if( !msgElem2.isNull() && msgElem2.tagName()=="msg")
			{
				rxTime.search(msgElem2.attribute("time"));
				QDateTime dt( QDate(date.year() , date.month() , rxTime.cap(1).toUInt()), QTime( rxTime.cap(2).toUInt() , rxTime.cap(3).toUInt(), rxTime.cap(5).toUInt()  ) );

				if (dt.date() != date)
				{
					n = n.nextSibling();
					continue;
				}

				Kopete::Message::MessageDirection dir = (msgElem2.attribute("in") == "1") ?
						Kopete::Message::Inbound : Kopete::Message::Outbound;

				if(!m_hideOutgoing || dir != Kopete::Message::Outbound)
				{ //parse only if we don't hide it

					QString f=msgElem2.attribute("from" );
					const Kopete::Contact *from=f.isNull()? 0L : (*it)->account()->contacts()[f];

					if(!from)
						from= dir==Kopete::Message::Inbound ? (*it) : (*it)->account()->myself();

					Kopete::ContactPtrList to;
					to.append( dir==Kopete::Message::Inbound ? (*it)->account()->myself() : *it );

					Kopete::Message msg(dt, from, to, msgElem2.text(), dir);
					msg.setBody( QString::fromLatin1("<span title=\"%1\">%2</span>")
							.arg( dt.toString(Qt::LocalDate), msg.escapedBody() ),
							Kopete::Message::RichText);
				

					// We insert it at the good place, given its date
 					QValueListIterator<Kopete::Message> msgIt;
					
					for (msgIt = messages.begin(); msgIt != messages.end(); ++msgIt)
					{
						if ((*msgIt).timestamp() > msg.timestamp())
							break;
					}
					messages.insert(msgIt, msg);
				}
			}

			n = n.nextSibling();
		} // end while on messages
		
	}
	return messages;
}

QValueList<Kopete::Message> HistoryLogger::readMessages(unsigned int lines,
	const Kopete::Contact *c, Sens sens, bool reverseOrder, bool colorize)
{
	//QDate dd =  QDate::currentDate().addMonths(0-m_currentMonth);

	QValueList<Kopete::Message> messages;

	// A regexp useful for this function
	QRegExp rxTime("(\\d+) (\\d+):(\\d+)($|:)(\\d*)"); //(with a 0.7.x compatibility)

	if(!m_metaContact)
	{ //this may happen if the contact has been moved, and the MC deleted
		if(c && c->metaContact())
			m_metaContact=c->metaContact();
		else
			return messages;
	}

	if(c && !m_metaContact->contacts().contains(c) )
		return messages;

	if(sens ==0 )  //if no sens are selected, just continue in the previous sens
		sens = m_oldSens ;
	if( m_oldSens != 0 && sens != m_oldSens )
	{ //we changed our sens! so retrieve the old position to fly in the other way
		m_currentElements= m_oldElements;
		m_currentMonth=m_oldMonth;
	}
	else
	{
		m_oldElements=m_currentElements;
		m_oldMonth=m_currentMonth;
	}
	m_oldSens=sens;

	//getting the color for messages:
	QColor fgColor = HistoryConfig::history_color();

	//Hello guest!

	//there are two algoritms:
	// - if a contact is given, or the metacontact contain only one contact,  just read the history.
	// - else, merge the history

	//the merging algoritm is the following:
	// we see what contact we have to read first, and we look at the firt date before another contact
	// has a message with a bigger date.

	QDateTime timeLimit;
	const Kopete::Contact *currentContact=c;
	if(!c && m_metaContact->contacts().count()==1)
		currentContact=m_metaContact->contacts().first();
	else if(!c && m_metaContact->contacts().count()== 0)
	{
		return messages;
	}
	
	while(messages.count() < lines)
	{
		timeLimit=QDateTime();
		QDomElement msgElem; //here is the message element
		QDateTime timestamp; //and the timestamp of this message

		if(!c && m_metaContact->contacts().count()>1)
		{ //we have to merge the differents subcontact history
			QPtrList<Kopete::Contact> ct=m_metaContact->contacts();
			QPtrListIterator<Kopete::Contact> it( ct );
			for( ; it.current(); ++it )
			{ //we loop over each contact. we are searching the contact with the next message with the smallest date,
			  // it will becomes our current contact, and the contact with the mext message with the second smallest
			  // date, this date will bocomes the limit.

				QDomNode n;
				if(m_currentElements.contains(*it))
					n=m_currentElements[*it];
				else  //there is not yet "next message" register, so we will take the first  (for the current month)
				{
					QDomDocument doc=getDocument(*it,m_currentMonth);
					QDomElement docElem = doc.documentElement();
					n= (sens==Chronological)?docElem.firstChild() : docElem.lastChild();

					//i can't drop the root element
					workaround.append(docElem);
				}
				while(!n.isNull())
				{
					QDomElement  msgElem2 = n.toElement();
					if( !msgElem2.isNull() && msgElem2.tagName()=="msg")
					{
						rxTime.search(msgElem2.attribute("time"));
						QDate d=QDate::currentDate().addMonths(0-m_currentMonth);
						QDateTime dt( QDate(d.year() , d.month() , rxTime.cap(1).toUInt()), QTime( rxTime.cap(2).toUInt() , rxTime.cap(3).toUInt(), rxTime.cap(5).toUInt()  ) );
						if(!timestamp.isValid() || ((sens==Chronological )? dt < timestamp : dt > timestamp) )
						{
							timeLimit=timestamp;
							timestamp=dt;
							msgElem=msgElem2;
							currentContact=*it;

						}
						else if(!timeLimit.isValid() || ((sens==Chronological) ? timeLimit > dt : timeLimit < dt) )
						{
							timeLimit=dt;
						}
						break;
					}
					n=(sens==Chronological)? n.nextSibling() : n.previousSibling();
				}
			}
		}
		else  //we don't have to merge the history. just take the next item in the contact
		{
			if(m_currentElements.contains(currentContact))
				msgElem=m_currentElements[currentContact];
			else
			{
				QDomDocument doc=getDocument(currentContact,m_currentMonth);
				QDomElement docElem = doc.documentElement();
				QDomNode n= (sens==Chronological)?docElem.firstChild() : docElem.lastChild();
				msgElem=QDomElement();
				while(!n.isNull()) //continue until we get a msg
				{
					msgElem=n.toElement();
					if( !msgElem.isNull() && msgElem.tagName()=="msg")
					{
						m_currentElements[currentContact]=msgElem;
						break;
					}
					n=(sens==Chronological)? n.nextSibling() : n.previousSibling();
				}

				//i can't drop the root element
				workaround.append(docElem);
			}
		}


		if(msgElem.isNull()) //we don't find ANY messages in any contact for this month. so we change the month
		{
			if(sens==Chronological)
			{
				if(m_currentMonth <= 0)
					break; //there are no other messages to show. break even if we don't have nb messages
				setCurrentMonth(m_currentMonth-1);
			}
			else
			{
				if(m_currentMonth >= getFirstMonth(c))
					break; //we don't have any other messages to show
				setCurrentMonth(m_currentMonth+1);
			}
			continue; //begin the loop from the bottom, and find currentContact and timeLimit again
		}

		while(
			(messages.count() < lines) &&
			!msgElem.isNull() &&
			(!timestamp.isValid() || !timeLimit.isValid() ||
				((sens==Chronological) ? timestamp <= timeLimit : timestamp >= timeLimit)
			))
		{
			// break this loop, if we have reached the correct number of messages,
			// if there are no more messages for this contact, or if we reached
			// the timeLimit msgElem is the next message, still not parsed, so
			// we parse it now

			Kopete::Message::MessageDirection dir = (msgElem.attribute("in") == "1") ?
				Kopete::Message::Inbound : Kopete::Message::Outbound;

			if(!m_hideOutgoing || dir != Kopete::Message::Outbound)
			{ //parse only if we don't hide it

				if( m_filter.isNull() || ( m_filterRegExp? msgElem.text().contains(QRegExp(m_filter,m_filterCaseSensitive)) : msgElem.text().contains(m_filter,m_filterCaseSensitive) ))
				{
					QString f=msgElem.attribute("from" );
					const Kopete::Contact *from=(f.isNull() || !currentContact) ? 0L : currentContact->account()->contacts()[f];

					if(!from)
						from= dir==Kopete::Message::Inbound ? currentContact : currentContact->account()->myself();

					Kopete::ContactPtrList to;
					to.append( dir==Kopete::Message::Inbound ? currentContact->account()->myself() : currentContact );

					if(!timestamp.isValid())
					{
						//parse timestamp only if it was not already parsed
						rxTime.search(msgElem.attribute("time"));
						QDate d=QDate::currentDate().addMonths(0-m_currentMonth);
						timestamp=QDateTime( QDate(d.year() , d.month() , rxTime.cap(1).toUInt()), QTime( rxTime.cap(2).toUInt() , rxTime.cap(3).toUInt() , rxTime.cap(5).toUInt() ) );
					}

					Kopete::Message msg(timestamp, from, to, msgElem.text(), dir);
					if (colorize)
					{
						msg.setBody( QString::fromLatin1("<span style=\"color:%1\" title=\"%2\">%3</span>")
							.arg( fgColor.name(), timestamp.toString(Qt::LocalDate), msg.escapedBody() ),
							Kopete::Message::RichText
						);
						msg.setFg( fgColor );
					}
					else
					{
						msg.setBody( QString::fromLatin1("<span title=\"%1\">%2</span>")
							.arg( timestamp.toString(Qt::LocalDate), msg.escapedBody() ),
							Kopete::Message::RichText
						);
					}

					if(reverseOrder)
						messages.prepend(msg);
					else
						messages.append(msg);
				}
			}

			//here is the point of workaround. If i drop the root element, this crashes
			//get the next message
			QDomNode node = ( (sens==Chronological) ? msgElem.nextSibling() :
				msgElem.previousSibling() );

			msgElem = QDomElement(); //n.toElement();
			while (!node.isNull() && msgElem.isNull())
			{
				msgElem = node.toElement();
				if (!msgElem.isNull())
				{
					if (msgElem.tagName() == "msg")
					{
						if (!c && (m_metaContact->contacts().count() > 1))
						{
							// In case of hideoutgoing messages, it is faster to do
							// this, so we don't parse the date if it is not needed
							QRegExp rx("(\\d+) (\\d+):(\\d+):(\\d+)");
							rx.search(msgElem.attribute("time"));

							QDate d = QDate::currentDate().addMonths(0-m_currentMonth);
							timestamp = QDateTime(
								QDate(d.year(), d.month(), rx.cap(1).toUInt()),
								QTime( rx.cap(2).toUInt(), rx.cap(3).toUInt() ) );
						}
						else
							timestamp = QDateTime(); //invalid
					}
					else
						msgElem = QDomElement();
				}

				node = (sens == Chronological) ? node.nextSibling() :
					node.previousSibling();
			}
			m_currentElements[currentContact]=msgElem;  //this is the next message
		}
	}

	if(messages.count() < lines)
		m_currentElements.clear(); //current elements are null this can't be allowed

	return messages;
}

QString HistoryLogger::getFileName(const Kopete::Contact* c, QDate date)
{
	
	QString name = c->protocol()->pluginId().replace( QRegExp( QString::fromLatin1( "[./~?*]" ) ), QString::fromLatin1( "-" ) ) +
		QString::fromLatin1( "/" ) +
		c->account()->accountId().replace( QRegExp( QString::fromLatin1( "[./~?*]" ) ), QString::fromLatin1( "-" ) ) +
		QString::fromLatin1( "/" ) +
	c->contactId().replace( QRegExp( QString::fromLatin1( "[./~?*]" ) ), QString::fromLatin1( "-" ) ) +
		date.toString(".yyyyMM");

	QString filename=locateLocal( "data", QString::fromLatin1( "kopete/logs/" ) + name+ QString::fromLatin1( ".xml" ) ) ;

	//Check if there is a kopete 0.7.x file
	QFileInfo fi(filename);
	if(!fi.exists())
	{
		name = c->protocol()->pluginId().replace( QRegExp( QString::fromLatin1( "[./~?*]" ) ), QString::fromLatin1( "-" ) ) +
			QString::fromLatin1( "/" ) +
			c->contactId().replace( QRegExp( QString::fromLatin1( "[./~?*]" ) ), QString::fromLatin1( "-" ) ) +
			date.toString(".yyyyMM");

		QString filename2=locateLocal( "data", QString::fromLatin1( "kopete/logs/" ) + name+ QString::fromLatin1( ".xml" ) ) ;

		QFileInfo fi2(filename2);
		if(fi2.exists())
			return filename2;
	}

	return filename;

}

unsigned int HistoryLogger::getFirstMonth(const Kopete::Contact *c)
{
	if(!c)
		return getFirstMonth();

	QRegExp rx( "\\.(\\d\\d\\d\\d)(\\d\\d)" );
	QFileInfo *fi;

	// BEGIN check if there are Kopete 0.7.x
	QDir d1(locateLocal("data",QString("kopete/logs/")+
		c->protocol()->pluginId().replace( QRegExp(QString::fromLatin1("[./~?*]")),QString::fromLatin1("-"))
		));
	d1.setFilter( QDir::Files | QDir::NoSymLinks );
	d1.setSorting( QDir::Name );

	const QFileInfoList *list1 = d1.entryInfoList();
	QFileInfoListIterator it1( *list1 );

	while ( (fi = it1.current()) != 0 )
	{
		if(fi->fileName().contains(c->contactId().replace( QRegExp( QString::fromLatin1( "[./~?*]" ) ), QString::fromLatin1( "-" ) )))
		{
			rx.search(fi->fileName());
			int result = 12*(QDate::currentDate().year() - rx.cap(1).toUInt()) +QDate::currentDate().month() - rx.cap(2).toUInt();

			if(result < 0)
			{
				kdWarning(14310) << k_funcinfo << "Kopete only found log file from Kopete 0.7.x made in the future. Check your date!" << endl;
				break;
			}
			return result;
		}
		++it1;
	}
	// END of kopete 0.7.x check


	QDir d(locateLocal("data",QString("kopete/logs/")+
		c->protocol()->pluginId().replace( QRegExp(QString::fromLatin1("[./~?*]")),QString::fromLatin1("-")) +
		QString::fromLatin1( "/" ) +
		c->account()->accountId().replace( QRegExp( QString::fromLatin1( "[./~?*]" ) ), QString::fromLatin1( "-" ) )
		));

	d.setFilter( QDir::Files | QDir::NoSymLinks );
	d.setSorting( QDir::Name );

	const QFileInfoList *list = d.entryInfoList();
	QFileInfoListIterator it( *list );
	while ( (fi = it.current()) != 0 )
	{
		if(fi->fileName().contains(c->contactId().replace( QRegExp( QString::fromLatin1( "[./~?*]" ) ), QString::fromLatin1( "-" ) )))
		{
			rx.search(fi->fileName());
			int result = 12*(QDate::currentDate().year() - rx.cap(1).toUInt()) +QDate::currentDate().month() - rx.cap(2).toUInt();
			if(result < 0)
			{
				kdWarning(14310) << k_funcinfo << "Kopete only found log file made in the future. Check your date!" << endl;
				break;
			}
			return result;
		}
		++it;
	}
	return 0;
}

unsigned int HistoryLogger::getFirstMonth()
{
	if(m_cachedMonth!=-1)
		return m_cachedMonth;

	if(!m_metaContact)
		return 0;

	int m=0;
	QPtrList<Kopete::Contact> contacts=m_metaContact->contacts();
	QPtrListIterator<Kopete::Contact> it( contacts );
	for( ; it.current(); ++it )
	{
		int m2=getFirstMonth(*it);
		if(m2>m) m=m2;
	}
	m_cachedMonth=m;
	return m;
}

void HistoryLogger::setHideOutgoing(bool b)
{
	m_hideOutgoing = b;
}

void HistoryLogger::slotMCDeleted()
{
	m_metaContact = 0;
}

void HistoryLogger::setFilter(const QString& filter, bool caseSensitive , bool isRegExp)
{
	m_filter=filter;
	m_filterCaseSensitive=caseSensitive;
	m_filterRegExp=isRegExp;
}

QString HistoryLogger::filter() const
{
	return m_filter;
}

bool HistoryLogger::filterCaseSensitive() const
{
	return m_filterCaseSensitive;
}

bool HistoryLogger::filterRegExp() const
{
	return m_filterRegExp;
}

QValueList<int> HistoryLogger::getDaysForMonth(QDate date)
{
	QRegExp rxTime("time=\"(\\d+) \\d+:\\d+(:\\d+)?\""); //(with a 0.7.x compatibility)

	QValueList<int> dayList;

	QPtrList<Kopete::Contact> contacts = m_metaContact->contacts();
	QPtrListIterator<Kopete::Contact> it(contacts);

	int lastDay=0;
	for(; it.current(); ++it)
	{
//		kdDebug() << getFileName(*it, date) << endl;
		QFile file(getFileName(*it, date));
		if(!file.open(IO_ReadOnly))
		{
			continue;
		}
		QTextStream stream(&file);
		QString fullText = stream.read();
		file.close();

		int pos = 0;
		while( (pos = rxTime.search(fullText, pos)) != -1)
		{
			pos += rxTime.matchedLength();
			int day=rxTime.capturedTexts()[1].toInt();
			
			if ( day !=lastDay && dayList.find(day) == dayList.end()) // avoid duplicates
			{
				dayList.append(rxTime.capturedTexts()[1].toInt());
				lastDay=day;
			}
		}
	}
	return dayList;
}

#include "historylogger.moc"
