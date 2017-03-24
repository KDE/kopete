/*
    latexguiclient.cpp - Kopete LaTeX plugin

    Copyright (c) 2003-2005 by Olivier Goffart <ogoffart@kde.org>

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

#include "latexguiclient.h"

#include <qvariant.h>

#include <QAction>
#include <KLocalizedString>
#include <QKeySequence>
#include <kmessagebox.h>
#include <kicon.h>
#include <qimage.h>
#include <qregexp.h>

#include "kopetechatsession.h"
#include "kopeteview.h"
#include "kopetemessage.h"

#include "latexplugin.h"
#include <kactioncollection.h>

LatexGUIClient::LatexGUIClient( Kopete::ChatSession *parent )
: QObject( parent), KXMLGUIClient( parent )
{
	//setComponentData( LatexPlugin::plugin()->componentData() );
	connect( LatexPlugin::plugin(), SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater()) );

	m_manager = parent;

	QAction *previewAction = new QAction( KIcon(QStringLiteral("latex")), i18n( "Preview Latex Images" ), this );
        actionCollection()->addAction( QStringLiteral("latexPreview"), previewAction );
    previewAction->setShortcut( QKeySequence(Qt::CTRL + Qt::Key_L) );
	connect(previewAction, SIGNAL(triggered(bool)), this, SLOT(slotPreview()) );

	setXMLFile( QStringLiteral("latexchatui.rc") );
}

LatexGUIClient::~LatexGUIClient()
{
}

void LatexGUIClient::slotPreview()
{
	if ( !m_manager->view() )
		return;

	Kopete::Message msg = m_manager->view()->currentMessage();
	const QString messageText = msg.plainBody();
	if(!messageText.contains(QLatin1String("$$"))) //we haven't found any latex strings
	{
		KMessageBox::sorry(m_manager->view()->mainWidget() , i18n("The message you are typing does not contain any LaTeX.  A LaTeX formula must be enclosed within two pairs of dollar signs: $$formula$$ "), i18n("No LaTeX Formula") );
		return;
	}

	const QString oldBody = msg.escapedBody();
	msg=Kopete::Message( msg.from() , msg.to() );
	msg.setHtmlBody( i18n("<b>Preview of the LaTeX message :</b> <br />%1", oldBody) );
	msg.setDirection( Kopete::Message::Internal );

	m_manager->appendMessage(msg) ;
}



// vim: set noet ts=4 sts=4 sw=4:

