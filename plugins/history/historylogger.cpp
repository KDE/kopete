/*
    historylogger.cpp

    Copyright (c) 2003 by Olivier Goffart        <ogoffart@tiscalinet.be>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

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

#include <qregexp.h>
#include <qfile.h>
#include <qdir.h>
#include <qdatetime.h>
#include <qdom.h>


#include <kdebug.h>
//#include <kiconloader.h>
#include <kstandarddirs.h>
#include <ksavefile.h>
#include <kconfig.h>


#include "kopetecontact.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopetemetacontact.h"

HistoryLogger::HistoryLogger( KopeteMetaContact *m ,  QObject *parent , const char *name )
 : QObject(parent, name)
{
	m_metaContact=m;
	m_hideOutgoing=false;
	m_cachedMonth=-1;
	m_oldSens=Default;

	//the contact may be destroyed, for example, if the contact chanfe metacontact
	connect(m_metaContact , SIGNAL(destroyed(QObject *)) , this , SLOT(slotMCDeleted()));

	setPositionToLast();
}


HistoryLogger::HistoryLogger( KopeteContact *c ,  QObject *parent , const char *name )
 : QObject(parent, name)
{
	m_cachedMonth=-1;
	m_metaContact=c->metaContact();
	m_hideOutgoing=false;
	m_oldSens=Default;

	//the contact may be destroyed, for example, if the contact chanfe metacontact
	connect(m_metaContact , SIGNAL(destroyed(QObject *)) , this , SLOT(slotMCDeleted()));

	setPositionToLast();
}

HistoryLogger::~HistoryLogger()
{
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
	setCurrentMonth( getFistMonth() );
	m_oldSens = Chronological;
	m_oldMonth=m_currentMonth;
	m_oldElements.clear();
}

void HistoryLogger::setCurrentMonth(int month)
{
	m_currentMonth=month;
	m_currentElements.clear();
}



QDomDocument HistoryLogger::getDocument(const KopeteContact *c, unsigned int month , bool canLoad , bool* contain)
{
	if(!m_metaContact)
	{ //this may happends if the contact has been moved, and the MC deleted
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
	if(documents.contains(month))
		return documents[month];

	if(!canLoad)
	{
		if(contain)
			*contain=false;
		return QDomDocument();
	}

	QString FileName = getFileName( c , month);

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

	documents.insert(month, doc);

	return doc;
}


void HistoryLogger::appendMessage( const KopeteMessage &msg , const KopeteContact *ct )
{
	if(!msg.from())
		return;

	const KopeteContact *c=ct?ct:(  msg.direction()==KopeteMessage::Outbound?msg.to().first():msg.from()  );

	if(!m_metaContact)
	{ //this may happends if the contact has been moved, and the MC deleted
		if(c && c->metaContact())
			m_metaContact=c->metaContact();
		else
			return;
	}


	if(!c || !m_metaContact->contacts().contains(c) )
	{
		QPtrList<KopeteContact> contacts= m_metaContact->contacts();
		QPtrListIterator<KopeteContact> it( contacts );
		for( ; it.current(); ++it )
		{
			if( (*it)->protocol()->pluginId() == msg.from()->protocol()->pluginId() )
			{
				c=*it;
				break;
			}
		}
		if(!c)
			return; //TODO: show a warning
	}
	QDomDocument doc=getDocument(c,0);
	QDomElement docElem = doc.documentElement();
	if(docElem.isNull())
	{
		docElem= doc.createElement( "kopete-history" );
		docElem.setAttribute ( "version" , "0.7" );
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
	msgElem.setAttribute( "in",  msg.direction()==KopeteMessage::Outbound ? "0" : "1" );
	msgElem.setAttribute( "from",  msg.from()->contactId() );
	msgElem.setAttribute( "nick",  msg.from()->displayName() ); //do we have to set this?
	msgElem.setAttribute( "time",  QString::number(msg.timestamp().date().day()) + " " +  QString::number(msg.timestamp().time().hour()) + ":" + QString::number(msg.timestamp().time().minute())  );
	QDomText msgNode = doc.createTextNode( msg.plainBody() );
	docElem.appendChild( msgElem );
	msgElem.appendChild( msgNode );


	//kdDebug() << "HistoryLogger::appendMessage: "  <<  doc.toString() << endl;
	//SAVE!
	//is that a good thing to save every time?
	KSaveFile file( getFileName(c,0) );
	if( file.status() == 0 )
	{
		QTextStream *stream = file.textStream();
		//stream->setEncoding( QTextStream::UnicodeUTF8 ); //???? oui ou non?
		doc.save( *stream ,1 );
		if ( !file.close() )
		{
			//kdDebug() << k_funcinfo << "can't close the file" << endl;
		}
	}
	else
	{
		//kdWarning() << k_funcinfo << "ERROR: Couldn't open accounts file " << fileName << " accounts not saved." << endl;
	}
}

QValueList<KopeteMessage> HistoryLogger::readMessages( unsigned int nb , const KopeteContact *c , Sens sens, bool reverseOrder )
{
	QValueList<KopeteMessage> messages;

	if(!m_metaContact)
	{ //this may happends if the contact has been moved, and the MC deleted
		if(c && c->metaContact())
			m_metaContact=c->metaContact();
		else
			return messages;
	}


	if(c && !m_metaContact->contacts().contains(c) ) return messages;

	if(sens ==0 )  //if no sens are selected, just continue in the previous sens
		sens = m_oldSens ;
	if( m_oldSens != 0 && sens != m_oldSens )
	{ //we changed our sens! so retreive the old position to fly in the other way
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
	KGlobal::config()->setGroup("History Plugin");
	QColor FGcolor=KGlobal::config()->readColorEntry( "History Color");

	//Hello guest!

	//there are two algoritms:
	// - if a contact is given, or the metacontact contain only one contact,  just read the history.
	// - else, merge the history

	//the merging algoritm is the following:
	// we see what contact we have to read first, and we look at the firt date before another contact
	// has a message with a bigger date.

	QDateTime timeLimit;
	const KopeteContact *currentContact=c;
	if(!c && m_metaContact->contacts().count()==1)
		currentContact=m_metaContact->contacts().first();

	while(messages.count() < nb )
	{
		timeLimit=QDateTime();
		QDomElement msgElem; //here is the message element
		QDateTime timestamp; //and the timestamp of this message

		if(!c && m_metaContact->contacts().count()>1)
		{ //we have to merge the differents subcontact history
			QPtrList<KopeteContact> ct=m_metaContact->contacts();
			QPtrListIterator<KopeteContact> it( ct );
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
						QRegExp rx("(\\d+) (\\d+):(\\d+)");
						rx.search(msgElem2.attribute("time"));
						QDate d=QDate::currentDate().addMonths(0-m_currentMonth);
						QDateTime dt( QDate(d.year() , d.month() , rx.cap(1).toUInt()), QTime( rx.cap(2).toUInt() , rx.cap(3).toUInt() ) );
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
//kdDebug() << "HistoryLogger::readMessages: changiong the month (debut) " << m_currentMonth << endl;
			if(sens==Chronological)
			{
				if(m_currentMonth <= 0)
					break; //there are no other messages to show. break even if we don't have nb messages
				setCurrentMonth(m_currentMonth-1);
			}
			else
			{
				if(m_currentMonth >= getFistMonth(c))
					break; //we don't have any other messages to show
				setCurrentMonth(m_currentMonth+1);
			}
//kdDebug() << "HistoryLogger::readMessages: changiong the month " << m_currentMonth << endl;
			continue; //begin the loop from the bottom, and find currentContact and timeLimit again
		}

		while(messages.count() < nb && !msgElem.isNull()  && ( !timestamp.isValid() || !timeLimit.isValid() || ((sens==Chronological) ? timestamp <= timeLimit : timestamp>= timeLimit) ))
		{   //break this loop, if we have reached the correct number of messages, if there are not more messages for this contact, or if we reached the timeLimit
		    //msgElem is the next message, still not parsed, so we parse it now

			KopeteMessage::MessageDirection dir= msgElem.attribute("in" ) == "1" ? KopeteMessage::Inbound : KopeteMessage::Outbound ;

			if(!m_hideOutgoing || dir != KopeteMessage::Outbound)
			{ //parse only if we don't hide it

				if( m_filter.isNull() || ( m_filterRegExp? msgElem.text().contains(QRegExp(m_filter,m_filterCaseSensitive)) : msgElem.text().contains(m_filter,m_filterCaseSensitive) ))
				{
					QString f=msgElem.attribute("from" );
					const KopeteContact *from=f.isNull()? 0L : currentContact->account()->contacts()[f];
					if(!from)
						from= dir==KopeteMessage::Inbound ? currentContact : currentContact->account()->myself();

					KopeteContactPtrList to;
					to.append( dir==KopeteMessage::Inbound ? currentContact->account()->myself() : currentContact );

					if(!timestamp.isValid())
					{ //parse timestamp only if it was not already parsed
						QRegExp rx("(\\d+) (\\d+):(\\d+)");
						rx.search(msgElem.attribute("time"));
						QDate d=QDate::currentDate().addMonths(0-m_currentMonth);
						timestamp=QDateTime( QDate(d.year() , d.month() , rx.cap(1).toUInt()), QTime( rx.cap(2).toUInt() , rx.cap(3).toUInt() ) );
					}

					KopeteMessage msg(timestamp,from,to, msgElem.text() , dir);
					msg.setFg(FGcolor);
	//kdDebug() << "HistoryLogger::readMessages: message: " << msg.plainBody() << endl;
					if(reverseOrder)
						messages.prepend(msg);
					else
						messages.append(msg);
				}
			}

			//here is the point of workaround.  if i drop the root element, this crashes
			QDomNode n=   ( (sens==Chronological) ? msgElem.nextSibling() : msgElem.previousSibling() ) ; //get the next massage
			msgElem=QDomElement(); //n.toElement();
			while(!n.isNull() && msgElem.isNull())
			{
//				kdDebug() << "HistoryLogger::readMessages: n " << n.isNull() << " " << n.nodeType() <<  " " <<n.nodeName() <<  endl;
				msgElem=n.toElement();
				if( !msgElem.isNull() )
				{
					if(msgElem.tagName()=="msg")
					{
						if(!c && m_metaContact->contacts().count()>1)
						{ //in case of hideoutgoing messages, it is faster to do this, so we don't parse the date if it is not needed
							QRegExp rx("(\\d+) (\\d+):(\\d+)");
							rx.search(msgElem.attribute("time"));
							QDate d=QDate::currentDate().addMonths(0-m_currentMonth);
							timestamp=QDateTime( QDate(d.year() , d.month() , rx.cap(1).toUInt()), QTime( rx.cap(2).toUInt() , rx.cap(3).toUInt() ) );
						}
						else
							timestamp=QDateTime(); //invalid
					}
					else
						msgElem=QDomElement();
				}
				n=(sens==Chronological)? n.nextSibling() : n.previousSibling();
			}
			m_currentElements[currentContact]=msgElem;  //this is the next message
		}
	}

	if(messages.count() < nb)
	{
		m_currentElements.clear(); //currents element are null this can't be allowed
	}

	return messages;
}

QString HistoryLogger::getFileName(const KopeteContact* c, unsigned int month)
{
	QDate d=QDate::currentDate().addMonths(0-month);

	QString name = c->protocol()->pluginId().replace( QRegExp( QString::fromLatin1( "[./~?*]" ) ), QString::fromLatin1( "-" ) ) +
			QString::fromLatin1( "/" ) +
			c->contactId().replace( QRegExp( QString::fromLatin1( "[./~?*]" ) ), QString::fromLatin1( "-" ) ) +
		d.toString(".yyyyMM");

	return locateLocal( "data", QString::fromLatin1( "kopete/logs/" ) + name+ QString::fromLatin1( ".xml" ) ) ;

}

unsigned int HistoryLogger::getFistMonth(const KopeteContact *c)
{
	if(!c)
		return getFistMonth();


	QDir d(locateLocal("data",QString("kopete/logs/%1").arg(c->protocol()->pluginId().replace(
		QRegExp(QString::fromLatin1("[./~?*]")),QString::fromLatin1("-"))))
		);

	d.setFilter( QDir::Files | QDir::NoSymLinks );
	d.setSorting( QDir::Name );

	const QFileInfoList *list = d.entryInfoList();
	QFileInfoListIterator it( *list );
	QFileInfo *fi;

	QRegExp rx( "\\.(\\d\\d\\d\\d)(\\d\\d)" );

	while ( (fi = it.current()) != 0 )
	{
		if(fi->fileName().contains(c->contactId().replace( QRegExp( QString::fromLatin1( "[./~?*]" ) ), QString::fromLatin1( "-" ) )))
		{
			rx.search(fi->fileName());
			int result = 12*(QDate::currentDate().year() - rx.cap(1).toUInt()) +QDate::currentDate().month() - rx.cap(2).toUInt();
			return result;
		}
		++it;
	}
	return 0;
}

unsigned int HistoryLogger::getFistMonth()
{
	if(m_cachedMonth!=-1)
		return m_cachedMonth;


	if(!m_metaContact)
		return 0;


	int m=0;
	QPtrList<KopeteContact> contacts=m_metaContact->contacts();
	QPtrListIterator<KopeteContact> it( contacts );
	for( ; it.current(); ++it )
	{
		int m2=getFistMonth(*it);
		if(m2>m) m=m2;
	}
	m_cachedMonth=m;
	return m;

}

void HistoryLogger::setHideOutgoing(bool b)
{
	m_hideOutgoing=b;
}

void HistoryLogger::slotMCDeleted()
{
	m_metaContact=0L;
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


#include "historylogger.moc"
