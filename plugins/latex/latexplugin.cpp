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

#include "latexplugin.h"

#include <qregexp.h>
#include <qimage.h>
#include <qbuffer.h>
#include <QTextDocument>

#include <kgenericfactory.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kprocess.h>
#include <ktemporaryfile.h>
#include <kcodecs.h>
#include <kmessagebox.h>

#include "kopetechatsessionmanager.h"
#include "kopeteuiglobal.h"

#include "latexconfig.h"
#include "latexguiclient.h"

#define ENCODED_IMAGE_MODE 0


K_PLUGIN_FACTORY(LatexPluginFactory, registerPlugin<LatexPlugin>();)
K_EXPORT_PLUGIN(LatexPluginFactory( "kopete_latex" ))


LatexPlugin::LatexPlugin( QObject *parent, const QVariantList &/*args*/ )
: Kopete::Plugin( LatexPluginFactory::componentData(), parent )
{
//	kDebug(14317) ;
	if( !s_pluginStatic )
		s_pluginStatic = this;

	mMagickNotFoundShown = false;
	connect( Kopete::ChatSessionManager::self(), SIGNAL(aboutToDisplay(Kopete::Message&)), SLOT(slotMessageAboutToShow(Kopete::Message&)) );
	connect( Kopete::ChatSessionManager::self(), SIGNAL(aboutToSend(Kopete::Message&)), this,  SLOT(slotMessageAboutToSend(Kopete::Message&)) );
	connect( Kopete::ChatSessionManager::self(), SIGNAL(chatSessionCreated(Kopete::ChatSession*)),
			 this, SLOT(slotNewChatSession(Kopete::ChatSession*)) );
	
	m_convScript = KStandardDirs::findExe("kopete_latexconvert.sh");

		//Add GUI action to all already existing kmm (if the plugin is launched when kopete already rining)
	QList<Kopete::ChatSession*> sessions = Kopete::ChatSessionManager::self()->sessions();
	foreach( Kopete::ChatSession* cs, sessions )
		slotNewChatSession( cs );
}

LatexPlugin::~LatexPlugin()
{
	qDeleteAll( m_tempFiles );
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
			    KMessageBox::Error, i18n("Cannot find the Magick 'convert' program.\nconvert is required to render the LaTeX formulae.\nPlease get the software from www.imagemagick.org or from your distribution's package manager.")
			);
			mMagickNotFoundShown = true;
		}
		// don't try to parse if convert is not installed
		return;
	}
	
	QString messageText = msg.plainBody();
	if( !messageText.contains("$$"))
		return;

	//kDebug(14317) << " Using converter: " << m_convScript;

	// /\[([^]]).*?\[/$1\]/
	// \$\$.+?\$\$
	
	// this searches for $$formula$$ 
	QRegExp rg("\\$\\$.+\\$\\$");
	rg.setMinimal(true);
	// this searches for [latex]formula[/latex]
	//QRegExp rg("\\[([^]\]).*?\\[/$1\\]");
	
	int pos = 0;
	
	QMap<QString, QString> replaceMap;
	while (pos >= 0 && pos < messageText.length())
	{
//		kDebug(14317) << " searching pos: " << pos;
		pos = rg.indexIn(messageText, pos);
		
		if (pos >= 0 )
		{
			const QString match = rg.cap(0);
			pos += rg.matchedLength();

			QString formul=match;
			// first remove the $$ delimiters on start and end
			formul.remove("$$");
			// then trim the result, so we can skip totally empty/whitespace-only formulas
			formul = formul.trimmed();
			if (formul.isEmpty() || !securityCheck(formul))
				continue;
			
			const QString fileName = handleLatex(formul);
			
			// get the image and encode it with base64
			#if ENCODED_IMAGE_MODE
			QImage renderedImage( fileName );
			imagePxWidth = renderedImage.width();
			imagePxHeight = renderedImage.height();
			if ( !renderedImage.isNull() )
			{
				QByteArray ba;
				QBuffer buffer( ba );
				buffer.open( QIODevice::WriteOnly );
				renderedImage.save( &buffer, "PNG" );
				QString imageURL = QString::fromLatin1("data:image/png;base64,%1").arg( KCodecs::base64Encode( ba ) );
				replaceMap[match] = imageURL;
			}
			#else
			replaceMap[match] = fileName;
			#endif
		}
	}

	if(replaceMap.isEmpty()) //we haven't found any LaTeX strings
		return;

	messageText= msg.escapedBody();

	int imagePxWidth,imagePxHeight;
	for (QMap<QString,QString>::ConstIterator it = replaceMap.constBegin(); it != replaceMap.constEnd(); ++it)
	{
		QImage theImage(*it);
		if(theImage.isNull())
			continue;
		imagePxWidth = theImage.width();
		imagePxHeight = theImage.height();
		QString escapedLATEX=Qt::escape(it.key()).replace('\"',"&quot;");  //we need  the escape quotes because that string will be in a title="" argument, but not the \n
		messageText.replace(Kopete::Message::escape(it.key()), " <img width=\"" + QString::number(imagePxWidth) + "\" height=\"" + QString::number(imagePxHeight) + "\" align=\"middle\" src=\"" + (*it) + "\"  alt=\"" + escapedLATEX +"\" title=\"" + escapedLATEX +"\"  /> ");
	}

	msg.setForcedHtmlBody( messageText );
}


void LatexPlugin::slotMessageAboutToSend( Kopete::Message& msg)
{
	Q_UNUSED(msg)
	//disabled because to work correctly, we need to find what special has the gif we can send over MSN
#if 0
	KSharedConfig::Ptr config = KGlobal::config();
	config->setGroup("Latex Plugin");

	if(!config->readEntry("ParseOutgoing", false))
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
			QString escapedLATEX= Qt::escape(messageText).replace('\"',"&quot;");
			QString messageText="<img src=\"" + url + "\" alt=\"" + escapedLATEX + "\" title=\"" + escapedLATEX +"\"  />";
			msg.setBody( messageText, Kopete::Message::RichText );
		}
	}
#endif
}

QString LatexPlugin::handleLatex(const QString &latexFormula)
{
	KTemporaryFile *tempFile=new KTemporaryFile();
	tempFile->setPrefix("kopetelatex-");
	tempFile->setSuffix(".png");
	tempFile->open();
	m_tempFiles.append(tempFile);
	QString fileName = tempFile->fileName();

	KProcess p;
	
	QString argumentRes = QString("-r %1x%2").arg(LatexConfig::horizontalDPI()).arg(LatexConfig::verticalDPI());
	QString argumentOut = QString("-o %1").arg(fileName);
	QString argumentInclude ("-x %1");
	//QString argumentFormat = "-fgif";  //we uses gif format because MSN only handle gif
	LatexConfig::self()->readConfig();
	QString includePath = LatexConfig::latexIncludeFile();
	if (!includePath.isNull())
		p << m_convScript <<  argumentRes << argumentOut /*<< argumentFormat*/ << argumentInclude.arg(includePath) << latexFormula;
	else
		p << m_convScript <<  argumentRes << argumentOut /*<< argumentFormat*/ << latexFormula;
	
	kDebug(14317) << "Rendering" << m_convScript << argumentRes << argumentOut << argumentInclude << latexFormula ;
	
	// FIXME our sucky sync filter API limitations :-)
	p.execute();
	return fileName;
}

bool LatexPlugin::securityCheck(const QString &latexFormula)
{
	return !latexFormula.contains(QRegExp("\\\\(def|let|futurelet|newcommand|renewcomment|else|fi|write|input|include"
			"|chardef|catcode|makeatletter|noexpand|toksdef|every|errhelp|errorstopmode|scrollmode|nonstopmode|batchmode"
			"|read|csname|newhelp|relax|afterground|afterassignment|expandafter|noexpand|special|command|loop|repeat|toks"
			"|output|line|mathcode|name|item|section|mbox|DeclareRobustCommand)[^a-zA-Z]"));

}

#include "latexplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:

