/***************************************************************************
                          kopete.cpp  -  description
                             -------------------
    begin                : Wed Dec 26 03:12:10 CLST 2001
    copyright            : (C) 2001 by Duncan Mac-Vicar Prett
    email                : duncan@puc.cl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kopete.h"
#include "kopete.moc"

#include <qvaluelist.h>
#include <qlist.h>
#include <qlayout.h>
#include <qregexp.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kcrash.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kstandarddirs.h>

#include <preferencesdialog.h>
#include <kopetewindow.h>

#include "ui/appearanceconfig.h"
#include "plugin.h"
#include "pluginloader.h"
#include "pluginmodule.h"
#include <addwizardimpl.h>

class Plugins;

Kopete::Kopete(): KUniqueApplication(true, true, true)
{
	//plugins = new PluginManager;
	allConnected = false;
    initEmoticons();

	mLibraryLoader = new LibraryLoader();
	mIconLoader = KGlobal::iconLoader();
	mPref = new PreferencesDialog();
	mPref->hide();

	Plugins *blah = new Plugins(this);

	mainwindow = new KopeteWindow();
	setMainWidget(mainwindow);

	mAppearance = new AppearanceConfig(mainwindow);

	KConfig *config=KGlobal::config();
	config->setGroup("");
	/* Ups! the user dont have plugins selected. */
	if (!config->hasKey("Modules"))
	{
		QStringList modules;
		modules.append("icq.plugin");
		modules.append("msn.plugin");
		config->writeEntry("Modules", modules);
	}

	/* Ok, load saved plugins */
	loadPlugins();
}


Kopete::~Kopete()
{
	kdDebug() << "Kopete::~Kopete()" << endl;

	delete mPref;
	delete mLibraryLoader;

/*
	kdDebug() << "~Kopete(), deleting tray" << endl;
	delete tray;
	}
*/

	kdDebug() << "~Kopete(), deleting mainwindow" << endl;
	delete mainwindow;

	// Only use this if cant find crash cause :-)
	//KCrash::setCrashHandler(Kopete::cleverKCrashHack);
	kdDebug() << "END OF Kopete::~Kopete()" << endl;
}


void Kopete::slotPreferences()
{
	mPref->show();
	mPref->raise();
}


/** No descriptions */
void Kopete::slotExit()
{
	kdDebug() << "Kopete::slotExit()" << endl;
	quit();
}


/** No descriptions */
void Kopete::readOptions()
{
	kdDebug() << "Kopete::readOptions()" << endl;
/*
	KConfig *config = KGlobal::config();
	config->setGroup("General");
	visible = config->readBoolEntry("Visible",true);
	QSize size          = config->readSizeEntry("Geometry");
	config->readBoolEntry("Idle Detection",false);
	config->readNumEntry("MaxIdle",15);

  if(!size.isEmpty())
  {
    resize(size);
  }
  QPoint pos=config->readPointEntry("Position");
  if(!pos.isNull())
  {
    move(pos);
  }
  config->setGroup("ICQ");
  QString icqNick = config->readEntry( "icqNick" );
  QString icqUIN = config->readEntry( "icqUIN" );
	QString icqPass = config->readEntry( "icqPass" );
	config->setGroup("MSN");
  QString msnNick = config->readEntry( "msnNick" );
  QString msnID = config->readEntry( "msnID" );
	QString msnPass = config->readEntry( "msnPass" );

	autoConnect=-1;
  if ( profiles.count() == 0 )
    slotSettings();
  else
  {
    //Construct menu...
    for ( int i = 0 ; i < profiles.count() ; i++ )
    {
      KAction *tmpaction = new KMsnAction( profiles[i] , //Name
                                        0           , //Accel
                                        this        , //Receiver
                                        SLOT(slotConnector( int ) ),
                                        actionCollection(),//Parent (not sure...)
                                        profiles[i] , //Internal name
                                        i );          //ID, needed for connecting
      //Add actions to the submenu
      fileConnector->insert( tmpaction );
      //And add them to our list...
      actions.append( tmpaction );
      //.. and make sure that they're deleted when we delete them
      actions.setAutoDelete( true );

      if (autoStr == profiles[i])
	autoConnect=i;
    }
  }
*/
}

/** No descriptions */
void Kopete::saveOptions()
{
}

/** Connect all loaded protocol plugins */
void Kopete::slotConnectAll()
{
	QValueList<KopeteLibraryInfo> l = kopeteapp->libraryLoader()->loaded();
    for (QValueList<KopeteLibraryInfo>::Iterator i = l.begin(); i != l.end(); ++i)
	{
		kdDebug() << "Kopete: Connect All: "<<(*i).name << endl;
		Plugin *tmpprot = (kopeteapp->libraryLoader())->mLibHash[(*i).specfile]->plugin;				
		IMProtocol *prot =  static_cast<IMProtocol*>(tmpprot);
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
		kdDebug() << "Kopete: Disconnect All: "<<(*i).name << endl;
		Plugin *tmpprot = (kopeteapp->libraryLoader())->mLibHash[(*i).specfile]->plugin;
		IMProtocol *prot =  static_cast<IMProtocol*>(tmpprot);
		if (prot->isConnected())
		{
			prot->Disconnect();
		}
	}
}

/** Add a contact through Wizard */
void Kopete::slotAddContact()
{
	AddWizardImpl *tmpdialog = new AddWizardImpl( mainWindow() );
	tmpdialog->show();
}

/** No descriptions */
void Kopete::slotSetAway()
{
}

/** No descriptions */
void Kopete::initPlugins()
{
}

/** No descriptions */
void Kopete::loadPlugins()
{
	mLibraryLoader->loadAll();
}

void Kopete::cleverKCrashHack(int)
{
	// do nothing

	// Understand that the KDE libraries have a memory leak, and
	// the playlist cannot be unloaded without causing a crash
	// in QApplication::windowMapper() or something similar.
	// this is just to prevent the KCrash window from appearing
	// and bugging the user regularly

	// someone fix the libraries.
	kdDebug() << "Crashed.\n" << endl;
	_exit(255);
}

void Kopete::initEmoticons()
{
	KStandardDirs dir;
	KConfig *config=KGlobal::config();
    config->setGroup("Appearance");
    mEmoticonTheme = config->readEntry("EmoticonTheme", "Default");
	/* Happy emoticons */
	/* :-) */
	mEmoticons.smile = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/smile.mng");
	if ( mEmoticons.smile.isNull() )
		mEmoticons.smile = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/smile.png");
	/* ;-) */
	mEmoticons.wink = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/wink.mng");
    if ( mEmoticons.wink.isNull() )
		mEmoticons.wink = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/wink.png");
	/* :-P */
	mEmoticons.tongue = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/tongue.mng");
	if ( mEmoticons.tongue.isNull() )
		mEmoticons.tongue = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/tongue.png");
	/* :-D*/
	mEmoticons.biggrin = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/bigrin.mng");
	if ( mEmoticons.biggrin.isNull() )
		mEmoticons.biggrin = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/biggrin.png");
	
	/* Sad emoticons */
	mEmoticons.unhappy = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/unhappy.mng");
	if ( mEmoticons.unhappy.isNull() )
		mEmoticons.unhappy = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/unhappy.png");
	
	mEmoticons.cry = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/cry.mng");
    if ( mEmoticons.cry.isNull() )
		mEmoticons.cry = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/cry.png");
	
	/* Surprise */
	mEmoticons.oh = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/oh.mng");
    if ( mEmoticons.oh.isNull() )
		mEmoticons.oh = dir.findResource("data","kopete/pics/emoticons/" + mEmoticonTheme + "/oh.png");
	
}

QString Kopete::parseEmoticons( QString message )
{
	if ( !mEmoticons.smile.isNull() )
	{
		message = message.replace(QRegExp(":-\\)"),"<img src=\""+mEmoticons.smile+"\">");
		message = message.replace(QRegExp(":\\)"),"<img src=\""+mEmoticons.smile+"\">");
	}
	if ( !mEmoticons.wink.isNull() )
	{
		message = message.replace(QRegExp(";-\\)"),"<img src=\""+mEmoticons.wink+"\">");
		message = message.replace(QRegExp(";\\)"),"<img src=\""+mEmoticons.wink+"\">");
    }
	if ( !mEmoticons.tongue.isNull() )
	{
		message = message.replace(QRegExp(":p"),"<img src=\""+mEmoticons.tongue+"\">");
		message = message.replace(QRegExp(":P"),"<img src=\""+mEmoticons.tongue+"\">");
		message = message.replace(QRegExp(":-p"),"<img src=\""+mEmoticons.tongue+"\">");
		message = message.replace(QRegExp(":-P"),"<img src=\""+mEmoticons.tongue+"\">");
	}
	if ( !mEmoticons.biggrin.isNull() )
	{
		message = message.replace(QRegExp(":D"),"<img src=\""+mEmoticons.biggrin+"\">");
		message = message.replace(QRegExp(":d"),"<img src=\""+mEmoticons.biggrin+"\">");
		message = message.replace(QRegExp(":-D"),"<img src=\""+mEmoticons.biggrin+"\">");
		message = message.replace(QRegExp(":-d"),"<img src=\""+mEmoticons.biggrin+"\">");
		message = message.replace(QRegExp(":>"),"<img src=\""+mEmoticons.biggrin+"\">");
		message = message.replace(QRegExp(":->"),"<img src=\""+mEmoticons.biggrin+"\">");
	}
    if ( !mEmoticons.unhappy.isNull() )
	{
		message = message.replace(QRegExp(":-\\("),"<img src=\""+mEmoticons.unhappy+"\">");
		message = message.replace(QRegExp(":\\("),"<img src=\""+mEmoticons.unhappy+"\">");
    }
	if ( !mEmoticons.cry.isNull() )
	{
		message = message.replace(QRegExp(":'-\\("),"<img src=\""+mEmoticons.cry+"\">");
		message = message.replace(QRegExp(":'\\("),"<img src=\""+mEmoticons.cry+"\">");
    }
	if ( !mEmoticons.oh.isNull() )
	{
		message = message.replace(QRegExp(":o"),"<img src=\""+mEmoticons.oh+"\">");
		message = message.replace(QRegExp(":O"),"<img src=\""+mEmoticons.oh+"\">");
		message = message.replace(QRegExp(":-o"),"<img src=\""+mEmoticons.oh+"\">");
		message = message.replace(QRegExp(":-O"),"<img src=\""+mEmoticons.oh+"\">");
	}
	#warning "TODO: Sleep emoticon parsing pending"
	
	return message;
}

QString Kopete::parseHTML( QString message )
{
	QString text, result, enc;
	QRegExp regExp;
	uint len = message.length();
	int matchLen;
	text = message;

	uint lastReplacement = 0;
	for ( uint idx=0; idx<len; idx++)
	{
		switch( text[idx].latin1() )
		{
			case '\r':
				lastReplacement=idx;
				break;
			case '\n':
				lastReplacement=idx;
				result += "<br>";
				break;
			case '\t':		// tab == 4 spaces
				lastReplacement=idx;
				result += "&nbsp;&nbsp;&nbsp;&nbsp;";
				break;

// BROKEN, WILL FIX, mETz, 18.03.2002
/*			case '@':		// email-addresses or message-ids
			{
				uint startIdx = idx;
				// move backwards to the begin of the address, stop when
				// the end of the last replacement is reached. (
				while ( (startIdx>0) && (startIdx>lastReplacement+1)
					&& (text[startIdx-1]!=' ') && (text[startIdx-1]!='\t')
					&& (text[startIdx-1]!=',') && (text[startIdx-1]!='<')
					&& (text[startIdx-1]!='>') && (text[startIdx-1]!='(')
					&& (text[startIdx-1]!=')') && (text[startIdx-1]!='[')
					&& (text[startIdx-1]!=']') && (text[startIdx-1]!='{')
					&& (text[startIdx-1]!='}') )
				{
						startIdx--;
				}

				regExp.setPattern("[^\\s<>\\(\\)\"\\|\\[\\]\\{\\}]+");
				if ( regExp.search(text,startIdx) != -1 )
				{
					matchLen = regExp.matchedLength();
					if (text[startIdx+matchLen-1]=='.')   // remove trailing dot
						matchLen--;
					else if (text[startIdx+matchLen-1]==',')   // remove trailing comma
						matchLen--;
					else if (text[startIdx+matchLen-1]==':')   // remove trailing colon
						matchLen--;

					if (matchLen < 3)
						result += text[idx];
					else
					{
						result.remove(result.length()-(idx-startIdx), idx-startIdx);
						result +=
							QString::fromLatin1("<a href=\"addrOrId://")
							+ parseHTML ( text.mid(startIdx,matchLen) )
							+ QString::fromLatin1("\">")
							+ parseHTML ( text.mid(startIdx,matchLen) )
							+ QString::fromLatin1("</a>");
						idx = startIdx+matchLen-1;
						lastReplacement=idx;
					}
					break;
				}
				result += text[idx];
				break;
			}
*/

			case 'h' :
			{
				if( (text[idx+1].latin1()=='t'))
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
				if( (text[idx+1].latin1()=='w'))
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
						lastReplacement=idx;
						break;
					}
				}
				result+=text[idx];
				break;
			}

			case 'f' :
			{
				if( text[idx+1].latin1()=='t' )
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
				if( (text[idx+1].latin1()=='a') && (text[idx+2].latin1()=='i') )
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
					if (( matchLen > 2 ) &&
					((idx==0)||text[idx-1].isSpace()||(text[idx-1] == '(')) &&
					((idx+matchLen==len)||text[idx+matchLen].isSpace()||(text[idx+matchLen]==',')||
					(text[idx+matchLen]=='.')||(text[idx+matchLen]==')')))
					{
						switch (text[idx].latin1())
						{
							case '_' :
								result += QString("<u>%1</u>").arg(parseHTML(text.mid(idx+1,matchLen-2)));
								break;
							case '/' :
								result += QString("<i>%1</i>").arg(parseHTML(text.mid(idx+1,matchLen-2)));
								break;
							case '*' :
								result += QString("<b>%1</b>").arg(parseHTML(text.mid(idx+1,matchLen-2)));
								break;
						}
						idx += matchLen-1;
						lastReplacement=idx;
						break;
					}
				}
				result += text[idx];
				break;
			}
			default:
				result += text[idx];
		}
	}
	return result;
}

/*
QString Kopete::parseURL( QString message )
{
}
*/
