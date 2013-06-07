//Olivier Goffart <ogoffart@kde.org>
// 2003 06 26

#include "historyplugin.h" //just needed because we are a member of this class
                           // we don't use any history function here

/**-----------------------------------------------------------
 * CONVERTER from the old kopete history.
 * it port history from kopete 0.6, 0.5 and above the actual
 * this should be placed in a perl script handled by KConf_update
 * but i need to access to some info i don't have with perl, like
 * the accountId, to know each protocol id, and more
 *-----------------------------------------------------------*/

#include <QtCore/QDir>
#include <QtCore/QRegExp>
#include <QtCore/QTextStream>
#include <QtGui/QApplication>
#include <QtXml/QDomDocument>

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kprogressdialog.h>
#include <ksavefile.h>

#include "kopetepluginmanager.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopetecontact.h"
#include "kopetemessage.h"
#include "kopeteprotocol.h"
#include "kopeteuiglobal.h"

#define CBUFLENGTH 512 // buffer length for fgets()

void HistoryPlugin::convertOldHistory()
{
	bool deleteFiles=  KMessageBox::questionYesNo( Kopete::UI::Global::mainWidget(),
		i18n( "Would you like to remove the old history files?" ) , i18n( "History Converter" ), KStandardGuiItem::del(), KGuiItem( i18n("Keep") ) ) == KMessageBox::Yes;

	KProgressDialog *progressDlg=new KProgressDialog(Kopete::UI::Global::mainWidget() , i18n( "History converter" ));
	progressDlg->setModal(true); //modal  to  make sure the user will not doing stupid things (we have a qApp->processEvents())
	progressDlg->setAllowCancel(false); //because i am too lazy to allow to cancel


	QString kopetedir=KStandardDirs::locateLocal( "data", QString::fromLatin1( "kopete"));
	QDir d( kopetedir ); //d should point to ~/.kde/share/apps/kopete/

	d.setFilter( QDir::Dirs  );

	const QFileInfoList list = d.entryInfoList();
	foreach(const QFileInfo &fi, list)
	{
		QString protocolId;
		QString accountId;

		if( Kopete::Protocol *p = dynamic_cast<Kopete::Protocol *>( Kopete::PluginManager::self()->plugin( fi.fileName() ) ) )
		{
			protocolId=p->pluginId();
			
			QList<Kopete::Account*> accountList = Kopete::AccountManager::self()->accounts(p);
			Kopete::Account *a = accountList.first();
			if(a)
				accountId=a->accountId();
		}

		if(accountId.isNull() || protocolId.isNull())
		{
			if(fi.fileName() == "MSNProtocol" || fi.fileName() == "msn_logs" )
			{
				protocolId="MSNProtocol";
				accountId=KGlobal::config()->group("MSN").readEntry( "UserID" );
			}
			else if(fi.fileName() == "ICQProtocol" || fi.fileName() == "icq_logs" )
			{
				protocolId="ICQProtocol";
				accountId=KGlobal::config()->group("ICQ").readEntry( "UIN" );
			}
			else if(fi.fileName() == "AIMProtocol" || fi.fileName() == "aim_logs" )
			{
				protocolId="AIMProtocol";
				accountId=KGlobal::config()->group("AIM").readEntry( "UserID" );
			}
			else if(fi.fileName() == "OscarProtocol" )
			{
				protocolId="AIMProtocol";
				accountId=KGlobal::config()->group("OSCAR").readEntry( "UserID" );
			}
			else if(fi.fileName() == "JabberProtocol" || fi.fileName() == "jabber_logs")
			{
				protocolId="JabberProtocol";
				accountId=KGlobal::config()->group("Jabber").readEntry( "UserID" );
			}
			//TODO: gadu, wp
		}

		if(!protocolId.isEmpty() || !accountId.isEmpty())
		{
			QDir d2( fi.absoluteFilePath() );
			d2.setFilter( QDir::Files  );
			d2.setNameFilters( QStringList("*.log") );
			const QFileInfoList list = d2.entryInfoList();;

			progressDlg->progressBar()->reset();
			progressDlg->progressBar()->setMaximum(d2.count());
			progressDlg->setLabelText(i18n("Parsing the old history in %1", fi.fileName()));
			progressDlg->show(); //if it was not already showed...

			foreach(const QFileInfo &fi2, list)
			{
				//we assume that all "-" are dots.  (like in hotmail.com)
				QString contactId=fi2.fileName().remove(".log").replace('-', '.');

				if(!contactId.isEmpty() )
				{
					progressDlg->setLabelText(i18n("Parsing the old history in %1:\n%2", fi.fileName(), contactId));
					qApp->processEvents(0); //make sure the text is updated in the progressDlg

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

					QString logFileName = fi2.absoluteFilePath();

					// open the file
					FILE *f = fopen(QFile::encodeName(logFileName), "r");

					// create a new <message> block
					while ( ! feof( f ) )
					{
						if ( ! fgets(cbuf, CBUFLENGTH, f) )
							break;
						buffer = QString::fromUtf8(cbuf);

						while ( strchr(cbuf, '\n') == NULL && !feof(f) )
						{
							if ( ! fgets( cbuf, CBUFLENGTH, f ) )
								break;
							buffer += QString::fromUtf8(cbuf);
						}

						if( buffer.startsWith( QString::fromLatin1( "<message " ) ) )
						{
							msgBlock = buffer;

							// find the end of the message block
							while( !feof( f ) && buffer != QString::fromLatin1( "</message>\n" ) /*strcmp("</message>\n", cbuf )*/ )
							{
								if ( ! fgets(cbuf, CBUFLENGTH, f) )
									break;
								buffer = QString::fromUtf8(cbuf);

								while ( strchr(cbuf, '\n') == NULL && !feof(f) )
								{
									if ( ! fgets( cbuf, CBUFLENGTH, f ) )
										break;
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
										body = element.text().trimmed();
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
									KSaveFile file(  KStandardDirs::locateLocal( "data", QString::fromLatin1( "kopete/logs/" ) + name +
									                                             QString::fromLatin1( ".xml" ) )  );
									if( file.open() )
									{
										QTextStream stream ( &file );
										//stream.setEncoding( QTextStream::UnicodeUTF8 ); //???? oui ou non?
										doc.save( stream , 1 );
										file.finalize();
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
								importElem.setAttribute( "from",  fi.fileName() );
								importElem.setAttribute( "date", QDateTime::currentDateTime().toString()  );
								headElem.appendChild(importElem);
							}
							QDomElement msgElem = doc.createElement( "msg" );
							msgElem.setAttribute( "in",  dir==Kopete::Message::Outbound ? "0" : "1" );
							msgElem.setAttribute( "from", dir==Kopete::Message::Outbound ? accountId : contactId  );
							msgElem.setAttribute( "nick",  nick ); //do we have to set this?
							msgElem.setAttribute( "time",  QString::number(dt.date().day()) + ' ' +  QString::number(dt.time().hour()) + ':' + QString::number(dt.time().minute())  );
							QDomText msgNode = doc.createTextNode( body.trimmed() );
							docElem.appendChild( msgElem );
							msgElem.appendChild( msgNode );
						}
					}

					fclose( f );
					if(deleteFiles)
						d2.remove(fi2.fileName());

					if(!docElem.isNull())
					{
						QDate date(year,month,1);
						QString name = protocolId.replace( QRegExp( QString::fromLatin1( "[./~?*]" ) ), QString::fromLatin1( "-" ) ) +
								QString::fromLatin1( "/" ) +
								contactId.replace( QRegExp( QString::fromLatin1( "[./~?*]" ) ), QString::fromLatin1( "-" ) ) +
								date.toString(".yyyyMM");
						KSaveFile file( KStandardDirs::locateLocal( "data", QString::fromLatin1( "kopete/logs/" ) + name +
						                                            QString::fromLatin1( ".xml" ) )  );
						if( file.open() )
						{
							QTextStream stream ( &file );
							//stream.setEncoding( QTextStream::UnicodeUTF8 ); //???? oui ou non?
							doc.save( stream ,1 );
							file.finalize();
						}
					}

				}
				progressDlg->progressBar()->setValue(progressDlg->progressBar()->value()+1);
			}
		}
	}
	delete progressDlg;

}


bool HistoryPlugin::detectOldHistory()
{
	QString version=KGlobal::config()->group("History Plugin").readEntry( "Version" ,"0.6" );

	if(version != "0.6")
		return false;


	QDir d( KStandardDirs::locateLocal( "data", QString::fromLatin1( "kopete/logs")) );
	d.setFilter( QDir::Dirs  );
	if(d.count() >= 3)  // '.' and '..' are included
		return false;  //the new history already exists

	QDir d2( KStandardDirs::locateLocal( "data", QString::fromLatin1( "kopete")) );
	d2.setFilter( QDir::Dirs  );
	const QFileInfoList list = d2.entryInfoList();

	foreach(const QFileInfo &fi, list)
	{
		if( dynamic_cast<Kopete::Protocol *>( Kopete::PluginManager::self()->plugin( fi.fileName() ) ) )
			return true;

		if(fi.fileName() == "MSNProtocol" || fi.fileName() == "msn_logs" )
			return true;
		else if(fi.fileName() == "ICQProtocol" || fi.fileName() == "icq_logs" )
			return true;
		else if(fi.fileName() == "AIMProtocol" || fi.fileName() == "aim_logs" )
			return true;
		else if(fi.fileName() == "OscarProtocol" )
			return true;
		else if(fi.fileName() == "JabberProtocol" || fi.fileName() == "jabber_logs")
			return true;
	}
	return false;
}
