/*
    Kopete Latex Plugin

    Copyright (c) 2004 by Duncan Mac-Vicar Prett   <duncan@kde.org>

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
#include <kgenericfactory.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kprocess.h>
#include <ktempfile.h>
#include <kmdcodec.h>

#include "kopetemessagemanagerfactory.h"

#include "latexplugin.h"
#include "latexconfig.h"

#define ENCODED_IMAGE_MODE 0

typedef KGenericFactory<LatexPlugin> LatexPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_latex, LatexPluginFactory( "kopete_latex" )  )

LatexPlugin::LatexPlugin( QObject *parent, const char *name, const QStringList &/*args*/ )
: KopetePlugin( LatexPluginFactory::instance(), parent, name )
{
	kdDebug() << k_funcinfo << endl;
	if( !s_pluginStatic )
		s_pluginStatic = this;

	connect( KopeteMessageManagerFactory::factory(), SIGNAL( aboutToDisplay( KopeteMessage & ) ), SLOT( slotHandleLatex( KopeteMessage & ) ) );
	connect ( this , SIGNAL( settingsChanged() ) , this , SLOT( slotSettingsChanged() ) );

	m_config = new LatexConfig;
	m_config->load();
	
	m_convScript = KStandardDirs::findExe("kopete_latexconvert.sh");
}

LatexPlugin::~LatexPlugin()
{
	s_pluginStatic = 0L;
	delete m_config;
}

LatexPlugin* LatexPlugin::plugin()
{
	return s_pluginStatic ;
}

LatexPlugin* LatexPlugin::s_pluginStatic = 0L;


void LatexPlugin::slotHandleLatex( KopeteMessage& msg )
{
	kdDebug() << k_funcinfo << endl;
	kdDebug() << k_funcinfo << " Using converter: " << m_convScript << endl;
	QString messageText = msg.plainBody();
	// /\[([^]]).*?\[/$1\]/
	// \$\$.+?\$\$
	
	// this searches for $$formula$$ 
	QRegExp rg("\\$\\$.+\\$\\$");
	rg.setMinimal(true);
	// this searches for [latex]formula[/latex]
	//QRegExp rg("\\[([^]\]).*?\\[/$1\\]");
	
	int pos = 0;
	int count = 0;
	
	QMap<QString, QString> replaceMap;
	while (pos >= 0 && pos < messageText.length())
	{
		kdDebug() << k_funcinfo  << " searching pos: " << pos << " count: " << count << endl;
		rg.search(messageText, pos);
		
		if (pos >= 0 )
		{
			QString match = rg.cap(0);
			
			if ( !match.length() )
			{
				pos += rg.matchedLength();
				count++;
				continue;
			}
			
			kdDebug() << k_funcinfo << " captured: " << match << endl;
			QString latexFormula = match;
			latexFormula.replace("$$","");
			
			// setup a temp file for the rendered image
			KTempFile tempFile;
			tempFile.close();
			
			KProcess p;
			QString fileName;
			
			fileName = tempFile.name();
			
			kdDebug() << k_funcinfo  << " Rendering " << latexFormula << " to: " << fileName<< endl;
			p << m_convScript << "-o " + fileName << +KShellProcess::quote(latexFormula).utf8();
			
			// FIXME our sucky sync filter API limitations :-)
			p.start(KProcess::Block);
			
			kdDebug() << k_funcinfo  << " render process finished..." << endl;
			
			// get the image and encode it with base64
			#if ENCODED_IMAGE_MODE
			QImage renderedImage( fileName );
			if ( !renderedImage.isNull() )
			{
				QByteArray ba;
				QBuffer buffer( ba );
				buffer.open( IO_WriteOnly );
				renderedImage.save( &buffer, "PNG" );
				QString imageURL = QString::fromLatin1("data:image/png;base64,%1").arg( KCodecs::base64Encode( ba ) );
				replaceMap[match] = imageURL;
				// ok, go for the next one
				pos += rg.matchedLength();
				count++;
			}
			#else
			QString imageURL = fileName;
			replaceMap[match] = imageURL;
			// ok, go for the next one
			pos += rg.matchedLength();
			count++;
			#endif
		}
	}
	
	for (QMap<QString,QString>::ConstIterator it = replaceMap.begin(); it != replaceMap.end(); ++it)
	{
		messageText.replace(it.key(), "<img src=\"" + (*it) + "\"/>;");
	}
	msg.setBody( messageText, KopeteMessage::RichText );
	
	//replaced_message.replace( QRegExp( match.arg( QRegExp::escape( it.key() ) ) ), map.find( 
}

void LatexPlugin::slotSettingsChanged()
{
	m_config->load();
}

#include "latexplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:

