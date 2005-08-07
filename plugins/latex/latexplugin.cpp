/*
    Kopete Latex Plugin

    Copyright (c) 2004 by Duncan Mac-Vicar Prett   <duncan@kde.org>
    Copyright (c) 2004-2005 by Olivier Goffart  <ogoffart@kde. org>

    Kopete    (c) 2001-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qregexp.h>
#include <qimage.h>
#include <qbuffer.h>
#include <qcstring.h>
#include <qstylesheet.h>
#include <kgenericfactory.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kprocess.h>
#include <ktempfile.h>
#include <kmdcodec.h>
#include <kmessagebox.h>

#include "kopetechatsessionmanager.h"
#include "kopeteuiglobal.h"

#include "latexplugin.h"
#include "latexconfig.h"
#include "latexguiclient.h"

#define ENCODED_IMAGE_MODE 0

typedef KGenericFactory<LatexPlugin> LatexPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_latex, LatexPluginFactory( "kopete_latex" )  )

LatexPlugin::LatexPlugin( QObject *parent, const char *name, const QStringList &/*args*/ )
: Kopete::Plugin( LatexPluginFactory::instance(), parent, name )
{
//	kdDebug() << k_funcinfo << endl;
	if( !s_pluginStatic )
		s_pluginStatic = this;

	mMagickNotFoundShown = false;
	connect( Kopete::ChatSessionManager::self(), SIGNAL( aboutToDisplay( Kopete::Message & ) ), SLOT( slotMessageAboutToShow( Kopete::Message & ) ) );
	connect( Kopete::ChatSessionManager::self(), SIGNAL( aboutToSend(Kopete::Message& )  ), this,  SLOT(slotMessageAboutToSend(Kopete::Message& )  ) );
	connect ( this , SIGNAL( settingsChanged() ) , this , SLOT( slotSettingsChanged() ) );
	connect( Kopete::ChatSessionManager::self(), SIGNAL( chatSessionCreated( Kopete::ChatSession * ) ),
			 this, SLOT( slotNewChatSession( Kopete::ChatSession * ) ) );
	
	m_convScript = KStandardDirs::findExe("kopete_latexconvert.sh");
	slotSettingsChanged();

		//Add GUI action to all already existing kmm (if the plugin is launched when kopete already rining)
	QValueList<Kopete::ChatSession*> sessions = Kopete::ChatSessionManager::self()->sessions();
	for (QValueListIterator<Kopete::ChatSession*> it= sessions.begin(); it!=sessions.end() ; ++it)
		slotNewChatSession( *it );
}

LatexPlugin::~LatexPlugin()
{
	s_pluginStatic = 0L;
}

LatexPlugin* LatexPlugin::plugin()
{
	return s_pluginStatic ;
}

LatexPlugin* LatexPlugin::s_pluginStatic = 0L;

void LatexPlugin::slotNewChatSession( Kopete::ChatSession *KMM )
{
	new LatexGUIClient( KMM );
}


void LatexPlugin::slotMessageAboutToShow( Kopete::Message& msg )
{
	QString mMagick = KStandardDirs::findExe("convert");
	if ( mMagick.isEmpty() )
	{
		// show just once
		if (  !mMagickNotFoundShown )
		{
			KMessageBox::queuedMessageBox(
			    Kopete::UI::Global::mainWidget(),
			    KMessageBox::Error, i18n("I cannot find the Magick convert program.\nconvert is required to render the Latex formulas.\nPlease go to www.imagemagick.org or to your distribution site and get the right package.")
			);
			mMagickNotFoundShown = true;
		}
		// dont try to parse if convert is not installed
		return;
	}
	
	QString messageText = msg.plainBody();
	if( !messageText.contains("$$"))
		return;

	//kdDebug() << k_funcinfo << " Using converter: " << m_convScript << endl;

	// /\[([^]]).*?\[/$1\]/
	// \$\$.+?\$\$
	
	// this searches for $$formula$$ 
	QRegExp rg("\\$\\$.+\\$\\$");
	rg.setMinimal(true);
	// this searches for [latex]formula[/latex]
	//QRegExp rg("\\[([^]\]).*?\\[/$1\\]");
	
	int pos = 0;
	
	QMap<QString, QString> replaceMap;
	while (pos >= 0 && (unsigned int)pos < messageText.length())
	{
//		kdDebug() << k_funcinfo  << " searching pos: " << pos << endl;
		pos = rg.search(messageText, pos);
		
		if (pos >= 0 )
		{
			QString match = rg.cap(0);
			pos += rg.matchedLength();

			QString formul=match;
			if(!securityCheck(formul))
				continue;
			
			QString fileName=handleLatex(formul.replace("$$",""));
			
			// get the image and encode it with base64
			#if ENCODED_IMAGE_MODE
			QImage renderedImage( fileName );
			imagePxWidth = renderedImage.width();
			imagePxHeight = renderedImage.height();
			if ( !renderedImage.isNull() )
			{
				QByteArray ba;
				QBuffer buffer( ba );
				buffer.open( IO_WriteOnly );
				renderedImage.save( &buffer, "PNG" );
				QString imageURL = QString::fromLatin1("data:image/png;base64,%1").arg( KCodecs::base64Encode( ba ) );
				replaceMap[match] = imageURL;
			}
			#else
			replaceMap[match] = fileName;
			#endif
		}
	}

	if(replaceMap.isEmpty()) //we haven't found any latex strings
		return;

	messageText= msg.escapedBody();

	int imagePxWidth,imagePxHeight;
	for (QMap<QString,QString>::ConstIterator it = replaceMap.begin(); it != replaceMap.end(); ++it)
	{
		QImage theImage(*it);
		if(theImage.isNull())
			continue;
		imagePxWidth = theImage.width();
		imagePxHeight = theImage.height();
		QString escapedLATEX=QStyleSheet::escape(it.key()).replace("\"","&quot;");  //we need  the escape quotes because that string will be in a title="" argument, but not the \n
		messageText.replace(Kopete::Message::escape(it.key()), " <img width=\"" + QString::number(imagePxWidth) + "\" height=\"" + QString::number(imagePxHeight) + "\" src=\"" + (*it) + "\"  alt=\"" + escapedLATEX +"\" title=\"" + escapedLATEX +"\"  /> ");
	}

	msg.setBody( messageText, Kopete::Message::RichText );
}


void LatexPlugin::slotMessageAboutToSend( Kopete::Message& msg)
{
	Q_UNUSED(msg)
	//disabled because to work correctly, we need to find what special has the gif we can send over MSN
#if 0
	KConfig *config = KGlobal::config();
	config->setGroup("Latex Plugin");

	if(!config->readBoolEntry("ParseOutgoing", false))
		return;

	QString messageText = msg.plainBody();
	if( !messageText.contains("$$"))
		return;
/*	if( msg.from()->protocol()->pluginId()!="MSNProtocol" )
	return;*/

	// this searches for $$formula$$
	QRegExp rg("^\\s*\\$\\$([^$]+)\\$\\$\\s*$");

	if( rg.search(messageText) != -1 )
	{
		QString latexFormula = rg.cap(1);
		if(!securityCheck( latexFormula ))
			return;

		QString url = handleLatex(latexFormula);


		if(!url.isNull())
		{
			QString escapedLATEX= QStyleSheet::escape(messageText).replace("\"","&quot;");
			QString messageText="<img src=\"" + url + "\" alt=\"" + escapedLATEX + "\" title=\"" + escapedLATEX +"\"  />";
			msg.setBody( messageText, Kopete::Message::RichText );
		}
	}
#endif
}

QString LatexPlugin::handleLatex(const QString &latexFormula)
{
	KTempFile *tempFile=new KTempFile( locateLocal( "tmp", "kopetelatex-" ), ".png" );
	tempFile->setAutoDelete(true);
	m_tempFiles.append(tempFile);
	m_tempFiles.setAutoDelete(true);
	QString fileName = tempFile->name();

	KProcess p;
			
	QString argumentRes = "-r %1x%2";
	QString argumentOut = "-o %1";
	//QString argumentFormat = "-fgif";  //we uses gif format because MSN only handle gif
	int hDPI, vDPI;
	hDPI = LatexConfig::self()->horizontalDPI();
	vDPI = LatexConfig::self()->verticalDPI();
	p << m_convScript <<  argumentRes.arg(QString::number(hDPI), QString::number(vDPI)) << argumentOut.arg(fileName) /*<< argumentFormat*/ << latexFormula  ;
			
	kdDebug() << k_funcinfo  << " Rendering " << m_convScript << " " <<  argumentRes.arg(QString::number(hDPI), QString::number(vDPI)) << " " << argumentOut.arg(fileName) << endl;
			
	// FIXME our sucky sync filter API limitations :-)
	p.start(KProcess::Block);
	return fileName;
}

bool LatexPlugin::securityCheck(const QString &latexFormula)
{
	return !latexFormula.contains(QRegExp("\\\\(def|let|futurelet|newcommand|renewcomment|else|fi|write|input|include"
			"|chardef|catcode|makeatletter|noexpand|toksdef|every|errhelp|errorstopmode|scrollmode|nonstopmode|batchmode"
			"|read|csname|newhelp|relax|afterground|afterassignment|expandafter|noexpand|special|command|loop|repeat|toks"
			"|output|line|mathcode|name|item|section|mbox|DeclareRobustCommand)[^a-zA-Z]"));

}

void LatexPlugin::slotSettingsChanged()
{
	LatexConfig::self()->readConfig();
}

#include "latexplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:

