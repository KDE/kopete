//Olivier Goffart <ogoffart @ kde.org>
// 2003 06 26

#include "historyplugin.h" //just needed because we are a member of this class
                           // we don't use any history function here

/**-----------------------------------------------------------
 * CONVERTER from the old kopete history.
 * it port history from kopete 0.6, 0.5 and above the actual
 * this should be placed in a perl script handled by KConf_update
 * but i need to acess to some info i don't have with perl, like
 * the accountId, to know each protocol id, and more
 *-----------------------------------------------------------*/

#include "kopetepluginmanager.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopetecontact.h"
#include "kopetemessage.h"
#include "kopeteprotocol.h"
#include "kopeteuiglobal.h"

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kprogress.h>
#include <kapplication.h>
#include <ksavefile.h>
#include <qdir.h>
#include <qdom.h>
#include <qregexp.h>

#define CBUFLENGTH 512 // buffer length for fgets()

void HistoryPlugin::convertOldHistory()
{
	bool deleteFiles=  KMessageBox::questionYesNo( Kopete::UI::Global::mainWidget(),
		i18n( "Would you like to remove old history files?" ) , i18n( "History Converter" ), KStdGuiItem::del(), i18n("Keep") ) == KMessageBox::Yes;

	KProgressDialog *progressDlg=new KProgressDialog(Kopete::UI::Global::mainWidget() , "history_progress_dlg" , i18n( "History converter" ) ,
		 QString::null , true); //modal  to  make sure the user will not doing stupid things (we have a kapp->processEvents())
	progressDlg->setAllowCancel(false); //because i am too lazy to allow to cancel


	QString kopetedir=locateLocal( "data", QString::fromLatin1( "kopete"));
	QDir d( kopetedir ); //d should point to ~/.kde/share/apps/kopete/

	d.setFilter( QDir::Dirs  );

	const QFileInfoList *list = d.entryInfoList();
	QFileInfoListIterator it( *list );
	QFileInfo *fi;
	while ( (fi = it.current()) != 0 )
	{
		QString protocolId;
		QString accountId;

		if( Kopete::Protocol *p = dynamic_cast<Kopete::Protocol *>( Kopete::PluginManager::self()->plugin( fi->fileName() ) ) )
		{
			protocolId=p->pluginId();
			QDictIterator<Kopete::Account> it(Kopete::AccountManager::self()->accounts(p));
			Kopete::Account *a = it.current();
			if(a)
				accountId=a->accountId();
		}

		if(accountId.isNull() || protocolId.isNull())
		{
			if(fi->fileName() == "MSNProtocol" || fi->fileName() == "msn_logs" )
			{
				protocolId="MSNProtocol";
				KGlobal::config()->setGroup("MSN");
				accountId=KGlobal::config()->readEntry( "UserID" );
			}
			else if(fi->fileName() == "ICQProtocol" || fi->fileName() == "icq_logs" )
			{
				protocolId="ICQProtocol";
				KGlobal::config()->setGroup("ICQ");
				accountId=KGlobal::config()->readEntry( "UIN" );
			}
			else if(fi->fileName() == "AIMProtocol" || fi->fileName() == "aim_logs" )
			{
				protocolId="AIMProtocol";
				KGlobal::config()->setGroup("AIM");
				accountId=KGlobal::config()->readEntry( "UserID" );
			}
			else if(fi->fileName() == "OscarProtocol" )
			{
				protocolId="AIMProtocol";
				KGlobal::config()->setGroup("OSCAR");
				accountId=KGlobal::config()->readEntry( "UserID" );
			}
			else if(fi->fileName() == "JabberProtocol" || fi->fileName() == "jabber_logs")
			{
				protocolId="JabberProtocol";
				KGlobal::config()->setGroup("Jabber");
				accountId=KGlobal::config()->readEntry( "UserID" );
			}
			//TODO: gadu, wp
		}

		if(!protocolId.isEmpty() || !accountId.isEmpty())
		{
			QDir d2( fi->absFilePath() );
			d2.setFilter( QDir::Files  );
			d2.setNameFilter("*.log");
			const QFileInfoList *list = d2.entryInfoList();
			QFileInfoListIterator it2( *list );
			QFileInfo *fi2;

			progressDlg->progressBar()->reset();
			progressDlg->progressBar()->setTotalSteps(d2.count());
			progressDlg->setLabel(i18n("Parsing old history in %1").arg(fi->fileName()));
			progressDlg->show(); //if it was not already showed...

			while ( (fi2 = it2.current()) != 0 )
			{
				//we assume that all "-" are dots.  (like in hotmail.com)
				QString contactId=fi2->fileName().replace(".log" , QString::null).replace("-" , ".");

				if(!contactId.isEmpty() )
				{
					progressDlg->setLabel(i18n("Parsing old history in %1:\n%2").arg(fi->fileName()).arg(contactId));
					kapp->processEvents(0); //make sure the text is updated in the progressDlg

					int month=0;
					int year=0;
					QDomDocument doc;
					QDomElement docElem;

					QDomElement msgelement;
					QDomNode node;
					QDomDocument xmllist;
					Kopete::Message::MessageDirection dir;
					QString body, date, nick;
					QString buffer, msgBlock;
					char cbuf[CBUFLENGTH]; // buffer for the log file

					QString logFileName = fi2->absFilePath();

					// open the file
					FILE *f = fopen(QFile::encodeName(logFileName), "r");

					// create a new <message> block
					while ( ! feof( f ) )
					{
						fgets(cbuf, CBUFLENGTH, f);
						buffer = QString::fromUtf8(cbuf);

						while ( strchr(cbuf, '\n') == NULL && !feof(f) )
						{
							fgets( cbuf, CBUFLENGTH, f );
							buffer += QString::fromUtf8(cbuf);
						}

						if( buffer.startsWith( QString::fromLatin1( "<message " ) ) )
						{
							msgBlock = buffer;

							// find the end of the message block
							while( !feof( f ) && buffer != QString::fromLatin1( "</message>\n" ) /*strcmp("</message>\n", cbuf )*/ )
							{
								fgets(cbuf, CBUFLENGTH, f);
								buffer = QString::fromUtf8(cbuf);

								while ( strchr(cbuf, '\n') == NULL && !feof(f) )
								{
									fgets( cbuf, CBUFLENGTH, f );
									buffer += QString::fromUtf8(cbuf);
								}
								msgBlock.append(buffer);
							}

							// now let's work on this new block
							xmllist.setContent(msgBlock, false);
							msgelement = xmllist.documentElement();
							node = msgelement.firstChild();

							if( msgelement.attribute( QString::fromLatin1( "direction" ) ) == QString::fromLatin1( "inbound" ) )
								dir = Kopete::Message::Inbound;
							else
								dir = Kopete::Message::Outbound;

							// Read all the elements.
							QString tagname;
							QDomElement element;

							while ( ! node.isNull() )
							{
								if ( node.isElement() )
								{
									element = node.toElement();
									tagname = element.tagName();

									if( tagname == QString::fromLatin1( "srcnick" ) )
										nick = element.text();

									else if( tagname == QString::fromLatin1( "date" ) )
										date = element.text();
									else if( tagname == QString::fromLatin1( "body" ) )
										body = element.text().stripWhiteSpace();
								}

								node = node.nextSibling();
							}
							//FIXME!! The date in logs writed with kopete running with QT 3.0 is Localised.
							// so QT can't parse it correctly.
							QDateTime dt=QDateTime::fromString(date);
							if(dt.date().month() != month || dt.date().year() != year)
							{
								if(!docElem.isNull())
								{
									QDate date(year,month,1);
									QString name = protocolId.replace( QRegExp( QString::fromLatin1( "[./~?*]" ) ), QString::fromLatin1( "-" ) ) +
											QString::fromLatin1( "/" ) +
											contactId.replace( QRegExp( QString::fromLatin1( "[./~?*]" ) ), QString::fromLatin1( "-" ) ) +
											date.toString(".yyyyMM");
									KSaveFile file(  locateLocal( "data", QString::fromLatin1( "kopete/logs/" ) + name+ QString::fromLatin1( ".xml" ) )  );
									if( file.status() == 0 )
									{
										QTextStream *stream = file.textStream();
										//stream->setEncoding( QTextStream::UnicodeUTF8 ); //???? oui ou non?
										doc.save( *stream , 1 );
										file.close();
									}
								}


								month=dt.date().month();
								year=dt.date().year();
								docElem=QDomElement();
							}

							if(docElem.isNull())
							{
								doc=QDomDocument("Kopete-History");
								docElem= doc.createElement( "kopete-history" );
								docElem.setAttribute ( "version" , "0.7" );
								doc.appendChild( docElem );
								QDomElement headElem = doc.createElement( "head" );
								docElem.appendChild( headElem );
								QDomElement dateElem = doc.createElement( "date" );
								dateElem.setAttribute( "year",  QString::number(year) );
								dateElem.setAttribute( "month", QString::number(month) );
								headElem.appendChild(dateElem);
								QDomElement myselfElem = doc.createElement( "contact" );
								myselfElem.setAttribute( "type",  "myself" );
								myselfElem.setAttribute( "contactId", accountId  );
								headElem.appendChild(myselfElem);
								QDomElement contactElem = doc.createElement( "contact" );
								contactElem.setAttribute( "contactId", contactId );
								headElem.appendChild(contactElem);
								QDomElement importElem = doc.createElement( "imported" );
								importElem.setAttribute( "from",  fi->fileName() );
								importElem.setAttribute( "date", QDateTime::currentDateTime().toString()  );
								headElem.appendChild(importElem);
							}
							QDomElement msgElem = doc.createElement( "msg" );
							msgElem.setAttribute( "in",  dir==Kopete::Message::Outbound ? "0" : "1" );
							msgElem.setAttribute( "from", dir==Kopete::Message::Outbound ? accountId : contactId  );
							msgElem.setAttribute( "nick",  nick ); //do we have to set this?
							msgElem.setAttribute( "time",  QString::number(dt.date().day()) + " " +  QString::number(dt.time().hour()) + ":" + QString::number(dt.time().minute())  );
							QDomText msgNode = doc.createTextNode( body.stripWhiteSpace() );
							docElem.appendChild( msgElem );
							msgElem.appendChild( msgNode );
						}
					}

					fclose( f );
					if(deleteFiles)
						d2.remove(fi2->fileName() , false);

					if(!docElem.isNull())
					{
						QDate date(year,month,1);
						QString name = protocolId.replace( QRegExp( QString::fromLatin1( "[./~?*]" ) ), QString::fromLatin1( "-" ) ) +
								QString::fromLatin1( "/" ) +
								contactId.replace( QRegExp( QString::fromLatin1( "[./~?*]" ) ), QString::fromLatin1( "-" ) ) +
								date.toString(".yyyyMM");
						KSaveFile file(  locateLocal( "data", QString::fromLatin1( "kopete/logs/" ) + name+ QString::fromLatin1( ".xml" ) )  );
						if( file.status() == 0 )
						{
							QTextStream *stream = file.textStream();
							//stream->setEncoding( QTextStream::UnicodeUTF8 ); //???? oui ou non?
							doc.save( *stream ,1 );
							file.close();
						}
					}

				}
				progressDlg->progressBar()->setProgress(progressDlg->progressBar()->progress()+1);
				++it2;
			}
		}
		++it;
	}
	delete progressDlg;

}


bool HistoryPlugin::detectOldHistory()
{
	KGlobal::config()->setGroup("History Plugin");
	QString version=KGlobal::config()->readEntry( "Version" ,"0.6" );

	if(version != "0.6")
		return false;


	QDir d( locateLocal( "data", QString::fromLatin1( "kopete/logs")) );
	d.setFilter( QDir::Dirs  );
	if(d.count() >= 3)  // '.' and '..' are included
		return false;  //the new history already exists

	QDir d2( locateLocal( "data", QString::fromLatin1( "kopete")) );
	d2.setFilter( QDir::Dirs  );
	const QFileInfoList *list = d2.entryInfoList();
	QFileInfoListIterator it( *list );
	QFileInfo *fi;
	while ( (fi = it.current()) != 0 )
	{
		if( dynamic_cast<Kopete::Protocol *>( Kopete::PluginManager::self()->plugin( fi->fileName() ) ) )
			return true;

		if(fi->fileName() == "MSNProtocol" || fi->fileName() == "msn_logs" )
			return true;
		else if(fi->fileName() == "ICQProtocol" || fi->fileName() == "icq_logs" )
			return true;
		else if(fi->fileName() == "AIMProtocol" || fi->fileName() == "aim_logs" )
			return true;
		else if(fi->fileName() == "OscarProtocol" )
			return true;
		else if(fi->fileName() == "JabberProtocol" || fi->fileName() == "jabber_logs")
			return true;
		++it;
	}
	return false;
}
