/*
    latexguiclient.cpp

    Kopete Latex plugin

    Copyright (c) 2003-2005 by Olivier Goffart <ogoffart @ kde.org>

    Kopete    (c) 2003-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qvariant.h>

#include <kaction.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qimage.h>
#include <qregexp.h>
#include <qstylesheet.h>

#include "kopetechatsession.h"
#include "kopeteview.h"
#include "kopetemessage.h"

#include "latexplugin.h"
#include "latexguiclient.h"

LatexGUIClient::LatexGUIClient( Kopete::ChatSession *parent, const char *name )
: QObject( parent, name ), KXMLGUIClient( parent )
{
	setInstance( LatexPlugin::plugin()->instance() );
	connect( LatexPlugin::plugin(), SIGNAL( destroyed( QObject * ) ), this, SLOT( deleteLater() ) );

	m_manager = parent;

	new KAction( i18n( "Preview Latex images" ), "latex", CTRL + Key_L, this, SLOT( slotPreview() ), actionCollection(), "latexPreview" );
	
	setXMLFile( "latexchatui.rc" );
}

LatexGUIClient::~LatexGUIClient()
{
}

void LatexGUIClient::slotPreview()
{
	if ( !m_manager->view() )
		return;

	Kopete::Message msg = m_manager->view()->currentMessage();
	QString messageText = msg.plainBody();
	if ( messageText.isEmpty() )
		return;

	QRegExp rg("\\$\\$.+\\$\\$");
	rg.setMinimal(true);

	int pos=0;

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
			if(!LatexPlugin::plugin()->securityCheck(formul))
				continue;
			
			QString fileName=LatexPlugin::plugin()->handleLatex(formul.replace("$$",""));
			
			replaceMap[match] = fileName;
		}
	}

	if(replaceMap.isEmpty()) //we haven't found any latex strings
	{
		KMessageBox::sorry(reinterpret_cast<QWidget*>(m_manager->view()) , i18n("There are no latex in the message you are typing.  The latex formula must be included between $$ and $$ "),	i18n("No Latex formula - Kopete") );
		return;
	}

	messageText= msg.escapedBody();

	int imagePxWidth, imagePxHeight;
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

	msg.setBody( i18n("<b>Preview of the latex message :</b> <br />%1").arg(messageText), Kopete::Message::RichText );
	m_manager->appendMessage(msg);
}


#include "latexguiclient.moc"

// vim: set noet ts=4 sts=4 sw=4:

