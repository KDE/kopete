/*
    kopete.cpp

    Kopete Instant Messenger Main Class

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett   <duncan@kde.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopete.h"

#include <qglobal.h>
#include <qregexp.h>
#include <qtimer.h>
#include <qstylesheet.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>

#include "addwizardimpl.h"
#include "appearanceconfig.h"
#include "kopeteaway.h"
#include "kopetecontactlist.h"
#include "kopeteemoticons.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetenotifier.h"
#include "kopeteprefs.h"
#include "kopeteprotocol.h"
#include "kopetetransfermanager.h"
#include "kopeteuserpreferences.h"
#include "kopetewindow.h"
#include "pluginloader.h"
#include "pluginmodule.h"
#include "preferencesdialog.h"

Kopete::Kopete()
: KUniqueApplication( true, true, true )
{
	/*
	 * This is a workaround for a quite odd problem:
	 * When starting up kopete and the msn plugin gets loaded it can bring up
	 * a messagebox, in case the msg configuration is missing. This messagebox
	 * will result in a QApplication::enter_loop() call, an event loop is
	 * created. At this point however the loop_level is 0, because this is all
	 * still inside the Kopete constructor, before the exec() call from main.
	 * When the messagebox is finished the loop_level will drop down to zero and
	 * QApplication thinks the application shuts down (this is usually the case
	 * when the loop_level goes down to zero) . So it emits aboutToQuit(), to
	 * which KApplication is connected and re-emits shutdown() , to which again
	 * KMainWindow (a KopeteWindow instance exists already) is connected. KMainWindow's
	 * shuttingDown() slot calls queryExit() which results in KopeteWindow::queryExit()
	 * calling unloadPlugins() . This of course is wrong and just shouldn't happen.
	 * The workaround is to simply delay the initialization of all this to a point
	 * where the loop_level is already > 0 . That is why I moved all the code from
	 * the constructor to the initialize() method and added this single-shot-timer
	 * setup. (Simon)
	 */
	QTimer::singleShot( 0, this, SLOT( initialize() ) );
}

void Kopete::initialize()
{
	mLibraryLoader = new LibraryLoader();
	mIconLoader = KGlobal::iconLoader();

	// TODO: move that to mainwindow!
	mPref = new PreferencesDialog();
	mPref->hide();

	mPluginsModule = new Plugins(this);

	m_mainWindow = new KopeteWindow( 0, "m_mainWindow" );
	setMainWidget(m_mainWindow);
	connect( m_mainWindow, SIGNAL( destroyed() ),
				this, SLOT( slotMainWindowDestroyed() ) );

	mAppearance = new AppearanceConfig(m_mainWindow);
	mUserPreferencesConfig = new KopeteUserPreferencesConfig(m_mainWindow);

	connect( KopetePrefs::prefs() , SIGNAL(saved()), this, SIGNAL(signalSettingsChanged()));
	mNotifier = new KopeteNotifier(this, "mNotifier");
	mMessageManagerFactory = new KopeteMessageManagerFactory(this, "KMMFactory");
        connect( mMessageManagerFactory, SIGNAL(messageReceived(KopeteMessage&)),
                 SIGNAL(aboutToDisplay(KopeteMessage&)) );
        connect( mMessageManagerFactory, SIGNAL(messageQueued(KopeteMessage&)),
                 SIGNAL(aboutToSend(KopeteMessage&)) );

	KConfig *config=KGlobal::config();
	config->setGroup("");

	// Ups! the user does not have plugins selected.
	if (!config->hasKey("Modules"))
	{
		QStringList modules;
		//modules.append("icq.plugin");
		//modules.append("msn.plugin");
		modules.append("autoaway.plugin");
		config->writeEntry("Modules", modules);
	}

	// Ok, load saved plugins
	loadPlugins();

	KopeteContactList::contactList()->load();
	mTransferManager = new KopeteTransferManager();
}


Kopete::~Kopete()
{
	kdDebug() << "[Kopete] ~Kopete()" << endl;

	delete mPref;
	delete mLibraryLoader;

	kdDebug() << "[Kopete] END ~Kopete()" << endl;
}

void Kopete::slotPreferences()
{
	kdDebug() << "[Kopete] slotPreferences()" << endl;
	mPref->show();
	mPref->raise();
}

/*
void Kopete::slotExit()
{
	kdDebug() << "[Kopete] slotExit()" << endl;
	quit();
}
*/

/** Connect all loaded protocol plugins */
void Kopete::slotConnectAll()
{
	QValueList<KopeteLibraryInfo> l = kopeteapp->libraryLoader()->loaded();
	for (QValueList<KopeteLibraryInfo>::Iterator i = l.begin(); i != l.end(); ++i)
	{
		kdDebug() << "[Kopete] Connect All: " << (*i).name << endl;
		KopetePlugin *tmpprot = (kopeteapp->libraryLoader())->mLibHash[(*i).specfile]->plugin;
		KopeteProtocol *prot =  dynamic_cast<KopeteProtocol*>(tmpprot);

		if (!prot)
			continue;

		if ( !(prot->isConnected()))
		{
			prot->Connect();
		}
	}
}

/** Disconnect all loaded protocol plugins */
void Kopete::slotDisconnectAll()
{
	QValueList<KopeteLibraryInfo> l = kopeteapp->libraryLoader()->loaded();
	for (QValueList<KopeteLibraryInfo>::Iterator i = l.begin(); i != l.end(); ++i)
	{
		kdDebug() << "[Kopete] Disconnect All: "<<(*i).name << endl;
		KopetePlugin *tmpprot = (kopeteapp->libraryLoader())->mLibHash[(*i).specfile]->plugin;
		KopeteProtocol *prot =  dynamic_cast<KopeteProtocol*>(tmpprot);

		if (!prot)
			continue;

		if (prot->isConnected())
		{
			prot->Disconnect();
		}
	}
}


// Set a meta-away in all protocol plugins
// This is a fire and forget thing, we do not check if
// it worked or if the plugin exits away-mode
void Kopete::slotSetAwayAll(void)
{
	KopeteAway::setGlobalAway(true);
	KopeteAway::show();
	setAwayAll();
}

// Set a meta-away in all protocol plugins without showing the dialog
void Kopete::setAwayAll(void)
{
	QValueList<KopeteLibraryInfo> l = kopeteapp->libraryLoader()->loaded();
	for (QValueList<KopeteLibraryInfo>::Iterator i = l.begin(); i != l.end(); ++i)
	{
		kdDebug() << "[Kopete] slotSetAwayAll() for plugin: " << (*i).name << endl;
		KopetePlugin *tmpprot = (kopeteapp->libraryLoader())->mLibHash[(*i).specfile]->plugin;
		KopeteProtocol *prot =  dynamic_cast<KopeteProtocol*>(tmpprot);

		if (!prot)
			continue;

		if ( prot->isConnected() && !prot->isAway() )
		{
			kdDebug() << "[Kopete] setting away-mode for: " << (*i).name << endl;
			prot->setAway(); // sets protocol-plugin into away-mode
		}
	}
}

// Set a meta-available in all protocol plugins
// This is a fire and forget thing, we do not check if
// it worked or if the plugin exits away-mode
void Kopete::slotSetAvailableAll(void)
{
	KopeteAway::setGlobalAway(false);
	QValueList<KopeteLibraryInfo> l = kopeteapp->libraryLoader()->loaded();
	for (QValueList<KopeteLibraryInfo>::Iterator i = l.begin(); i != l.end(); ++i)
	{
		kdDebug() << "[Kopete] slotSetAvailableAll() for plugin: " << (*i).name << endl;
		KopetePlugin *tmpprot = (kopeteapp->libraryLoader())->mLibHash[(*i).specfile]->plugin;
		KopeteProtocol *prot =  dynamic_cast<KopeteProtocol*>(tmpprot);

		if (!prot)
			continue;

		if ( prot->isConnected() && prot->isAway() )
		{
			kdDebug() << "[Kopete] setting available-mode for: " << (*i).name << endl;
			prot->setAvailable(); // sets protocol-plugin into away-mode
		}
	}
}

/** Add a contact through Wizard */
void Kopete::slotAddContact()
{
	AddWizardImpl *tmpdialog = new AddWizardImpl( mainWindow() );
	tmpdialog->show();
}

/** Load all plugins */
void Kopete::loadPlugins()
{
	mLibraryLoader->loadAll();
}

/** Add a Event for notify */
void Kopete::notifyEvent( KopeteEvent *event)
{
	/* See KopeteNotifier and KopeteEvent class */
	mNotifier->notifyEvent( event );
}

/** Cancel an event */
void Kopete::cancelEvent( KopeteEvent *event)
{
	/* deleted events are removed automaticly */
	delete event;
}

/*
	Parse emoticons in a string, returns html/qt richtext.
	If emoticons are activated in preferences it
	replaces known emoticons with themed images,
	if not, it returns the unchanged string.
*/
QString Kopete::parseEmoticons( QString message )
{
	//kdDebug() << "[Kopete] parseEmoticons()" << endl;
	// if emoticons are disabled, we do nothing
	if ( !KopetePrefs::prefs()->useEmoticons() )
		return message;
		
	//kdDebug() << "[[" << message << "]]" << endl;

	QStringList emoticons = KopeteEmoticons::emoticons()->emoticonList();
	QString em;
	int p = -1;
	for ( QStringList::Iterator it = emoticons.begin(); it != emoticons.end(); ++it )
	{
		em = QStyleSheet::escape(*it); 
//		kdDebug() << "looking for " << em << endl;

		#if (QT_VERSION-0 >= 0x030100)
		message.replace( em, "<img src=\"" +
			KopeteEmoticons::emoticons()->emoticonToPicPath(*it) + "\">" );
		#else
		while ( (p = message.find(em,0,true)) != -1 )
		{
			message = message.remove( p, em.length() );
			message = message.insert( p, "<img src=\"" +
				KopeteEmoticons::emoticons()->emoticonToPicPath(*it) + "\">" );
		}
		#endif 
	}
	//FIXME: if I tape "<-)" , "&lt;-)" is interpreted and the ;-) smileys is showed

	return message;
}

QString Kopete::parseHTML( QString message, bool parseURLs )
{
	QString text, result;
	QRegExp regExp;
	uint len = message.length();
	int matchLen;
	uint startIdx;
	int lastReplacement = -1;
	text = message;

	for ( uint idx=0; idx<len; idx++ )
	{
		switch( text[idx].latin1() )
		{
			case '\r':
				lastReplacement = idx;
				break;
			case '\n':
				lastReplacement = idx;
				result += "<br>";
				break;
			case '\t':		// tab == 4 spaces
				lastReplacement = idx;
				result += "&nbsp;&nbsp;&nbsp;&nbsp;";
				break;
			case ' ':		// convert doubles spaces to HTML
				if( (idx>0) && (text[idx-1]==' '))
					result += "&nbsp;";
				else
					result += " ";
				lastReplacement = idx;
				break;

			case '@':		// email-addresses or message-ids
				if ( parseURLs )
				{
					startIdx = idx;
					while (
						(startIdx>=0) &&
						(startIdx>(uint)(lastReplacement+1)) &&
						(text[startIdx-1]!=' ') &&
						(text[startIdx-1]!='\t') &&
						(text[startIdx-1]!=',') &&
						(text[startIdx-1]!='<') && (text[startIdx-1]!='>') &&
						(text[startIdx-1]!='(') && (text[startIdx-1]!=')') &&
						(text[startIdx-1]!='[') && (text[startIdx-1]!=']') &&
						(text[startIdx-1]!='{') && (text[startIdx-1]!='}')
						)
					{
//						kdDebug() << "searching start of email addy at: " << startIdx << endl;
						startIdx--;
					}

					kdDebug() << "found start of email addy at:" << startIdx << endl;

					regExp.setPattern("[^\\s<>\\(\\)\"\\|\\[\\]\\{\\}]+");
					if ( regExp.search(text,startIdx) != -1 )
					{
						matchLen = regExp.matchedLength();
						if (text[startIdx+matchLen-1]=='.')   // remove trailing dot
						{
							matchLen--;
						}
						else if (text[startIdx+matchLen-1]==',')   // remove trailing comma
						{
							matchLen--;
						}
						else if (text[startIdx+matchLen-1]==':')   // remove trailing colon
						{
							matchLen--;
						}

						if ( matchLen < 3 )
						{
							result += text[idx];
						}
						else
						{
//							kdDebug() << "adding email link starting at: " << result.length()-(idx-startIdx) << endl;
							result.remove( result.length()-(idx-startIdx), idx-startIdx );
							QString mailAddr = parseHTML(text.mid(startIdx,matchLen),false);
							result += QString::fromLatin1("<a href=\"mailto:%1\">%2</a>").arg(mailAddr).arg(mailAddr);
/*								QString::fromLatin1("<a href=\"addrOrId://") + // What is this weird adress?
								parseHTML(text.mid(startIdx,matchLen),false) +
								QString::fromLatin1("\">") +
								parseHTML(text.mid(startIdx,matchLen),false) +
								QString::fromLatin1("</a>"); */
							idx = startIdx + matchLen - 1;
							kdDebug() << "index is now: " << idx << endl;
							kdDebug() << "result is: " << result << endl;
							lastReplacement = idx;
						}
						break;
					}
				}
				result += text[idx];
				break;

			case 'h' :
			{
				if( (parseURLs) && (text[idx+1].latin1()=='t') )
				{   // don't do all the stuff for every 'h'
					regExp.setPattern("https?://[^\\s<>\\(\\)\"\\|\\[\\]\\{\\}]+");
					if ( regExp.search(text,idx) == (int)idx )
					{
						matchLen = regExp.matchedLength();

						if (text[idx+matchLen-1]=='.')			// remove trailing dot
							matchLen--;
						else if (text[idx+matchLen-1]==',')		// remove trailing comma
							matchLen--;
						else if (text[idx+matchLen-1]==':')		// remove trailing colon
							matchLen--;

						result +=
							QString::fromLatin1("<a href=\"")
							+ text.mid(idx,matchLen)
							+ QString::fromLatin1("\">")
							+ text.mid(idx,matchLen)
							+ QString::fromLatin1("</a>");
						idx += matchLen-1;
						lastReplacement = idx;
						break;
					}
				}
				result += text[idx];
				break;
			}

			case 'w':
			{
				if( (parseURLs) && (text[idx+1].latin1()=='w') && (text[idx+2].latin1()=='w') )
				{   // don't do all the stuff for every 'w'
					regExp.setPattern("www\\.[^\\s<>\\(\\)\"\\|\\[\\]\\{\\}]+\\.[^\\s<>\\(\\)\"\\|\\[\\]\\{\\}]+");
					if (regExp.search(text,idx)==(int)idx)
					{
						matchLen = regExp.matchedLength();
						if (text[idx+matchLen-1]=='.')   // remove trailing dot
							matchLen--;
						else if (text[idx+matchLen-1]==',')   // remove trailing comma
							matchLen--;
						else if (text[idx+matchLen-1]==':')   // remove trailing colon
							matchLen--;

						result +=
							QString::fromLatin1("<a href=\"http://")
							+ text.mid(idx,matchLen)
							+ QString::fromLatin1("\">")
							+ text.mid(idx,matchLen)
							+ QString::fromLatin1("</a>");
						idx += matchLen-1;
						lastReplacement = idx;
						break;
					}
				}
				result+=text[idx];
				break;
			}

			case 'f' :
			{
				if( (parseURLs) && (text[idx+1].latin1()=='t') && (text[idx+2].latin1()=='p') )
				{   // don't do all the stuff for every 'f'
					regExp.setPattern("ftp://[^\\s<>\\(\\)\"\\|\\[\\]\\{\\}]+");
					if ( regExp.search(text,idx)==(int)idx )
					{
						matchLen = regExp.matchedLength();
						if (text[idx+matchLen-1]=='.')   // remove trailing dot
							matchLen--;
						else if (text[idx+matchLen-1]==',')   // remove trailing comma
							matchLen--;
						else if (text[idx+matchLen-1]==':')   // remove trailing colon
							matchLen--;

						result +=
							QString::fromLatin1("<a href=\"")
							+ text.mid(idx,matchLen)
							+ QString::fromLatin1("\">")
							+ text.mid(idx,matchLen)
							+ QString::fromLatin1("</a>");
						idx += matchLen-1;
						lastReplacement = idx;
						break;
					}

					regExp.setPattern("ftp\\.[^\\s<>\\(\\)\"\\|\\[\\]\\{\\}]+\\.[^\\s<>\\(\\)\"\\|\\[\\]\\{\\}]+");
					if ( regExp.search(text,idx)==(int)idx )
					{
						matchLen = regExp.matchedLength();
						if (text[idx+matchLen-1]=='.')   // remove trailing dot
						matchLen--;
						else if (text[idx+matchLen-1]==',')   // remove trailing comma
						matchLen--;
						else if (text[idx+matchLen-1]==':')   // remove trailing colon
						matchLen--;

						result +=
							QString::fromLatin1("<a href=\"ftp://")
							+ text.mid(idx,matchLen)
							+ QString::fromLatin1("\">")
							+ text.mid(idx,matchLen)
							+ QString::fromLatin1("</a>");
						idx += matchLen-1;
						lastReplacement = idx;
						break;
					}
				}
				result+=text[idx];
				break;
			}

			case 'm' :
			{
				if( (parseURLs) && (text[idx+1].latin1()=='a') && (text[idx+2].latin1()=='i') )
				{   // don't do all the stuff for every 'm'
					regExp.setPattern("mailto:[^\\s<>\\(\\)\"\\|\\[\\]\\{\\}]+");
					if (regExp.search(text,idx)==(int)idx)
					{
						matchLen = regExp.matchedLength();
						if (text[idx+matchLen-1]=='.')   // remove trailing dot
						matchLen--;
						else if (text[idx+matchLen-1]==',')   // remove trailing comma
						matchLen--;
						else if (text[idx+matchLen-1]==':')   // remove trailing colon
						matchLen--;

						result +=
							QString::fromLatin1("<a href=\"")
							+ text.mid(idx,matchLen)
							+ QString::fromLatin1("\">")
							+ text.mid(idx,matchLen)
							+ QString::fromLatin1("</a>");
						idx += matchLen-1;
						lastReplacement = idx;
						break;
					}
				}
				result += text[idx];
				break;
			}

			case '_' :
			case '/' :
			case '*' :
			{
				regExp = QString("\\%1[^\\s%2]+\\%3").arg(text[idx]).arg(text[idx]).arg(text[idx]);
				if ( regExp.search(text,idx) == (int)idx )
				{
					matchLen = regExp.matchedLength();
					if ((matchLen>2) &&
					((idx==0)||text[idx-1].isSpace()||(text[idx-1] == '(')) &&
					((idx+matchLen==len)||text[idx+matchLen].isSpace()||(text[idx+matchLen]==',')||
					(text[idx+matchLen]=='.')||(text[idx+matchLen]==')')))
					{
						switch (text[idx].latin1())
						{
							case '_' :
								result += QString("<u>%1</u>").arg( parseHTML(text.mid(idx+1,matchLen-2),parseURLs) );
								break;
							case '/' :
								result += QString("<i>%1</i>").arg( parseHTML(text.mid(idx+1,matchLen-2),parseURLs) );
								break;
							case '*' :
								result += QString("<b>%1</b>").arg( parseHTML(text.mid(idx+1,matchLen-2),parseURLs) );
								break;
						}
						idx += matchLen-1;
						lastReplacement = idx;
						break;
					}
				}
				result += text[idx];
				break;
			}
			default:
				result += text[idx];
				break;
		}
	}
	return result;
}

KopeteContactListView *Kopete::contactList() const
{
	return m_mainWindow ? m_mainWindow->contactlist : 0L;
}

KStatusBar* Kopete::statusBar() const
{
	return m_mainWindow ? m_mainWindow->statusBar() : 0L;
}

KopeteSystemTray* Kopete::systemTray() const
{
	return m_mainWindow ? m_mainWindow->tray : 0L;
}

void Kopete::slotMainWindowDestroyed()
{
	m_mainWindow = 0L;
}

void Kopete::slotShowTransfers()
{
	transferManager()->show();
}

#include "kopete.moc"

// vim: set noet ts=4 sts=4 sw=4:
