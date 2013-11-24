/*
    urlpicpreviewplugin.cpp

    Copyright (c) 2005      by Heiko Schaefer        <heiko@rangun.de>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    **************************************************************************
    *                                                                        *
    * This program is free software; you can redistribute it and/or modify   *
    * it under the terms of the GNU General Public License as published by   *
    * the Free Software Foundation; version 2, or (at your option) version 3 *
    * of the License.                                                        *
    *                                                                        *
    **************************************************************************
*/

#include "urlpicpreviewplugin.h"

// Qt
#include <qimage.h>
#include <qregexp.h>

// KDE
#include <kdebug.h>
#include <kimageio.h>
#include <ktemporaryfile.h>
#include <kapplication.h>
#include <kgenericfactory.h>

// KIO
#include <kio/netaccess.h>

// Kopete
#include "linkpreview.h"
#include "kopeteuiglobal.h"
#include "urlpicpreviewconfig.h"
#include "kopetechatsessionmanager.h"

K_PLUGIN_FACTORY( URLPicPreviewPluginFactory, registerPlugin<URLPicPreviewPlugin>(); )
K_EXPORT_PLUGIN( URLPicPreviewPluginFactory( "kopete_urlpicpreview" ) )

URLPicPreviewPlugin::URLPicPreviewPlugin ( QObject* parent, const QVariantList& /* args */ )
		: Kopete::Plugin ( URLPicPreviewPluginFactory::componentData(), parent ), m_pic ( NULL ), m_abortMessageCheck ( false )
{

	kDebug ( 14314 );

	Kopete::ChatSessionManager * chatSessionManager = Kopete::ChatSessionManager::self();
	connect ( chatSessionManager, SIGNAL (aboutToDisplay(Kopete::Message&)),
	          this, SLOT (aboutToDisplay(Kopete::Message&)) );

	connect ( this, SIGNAL (readyForUnload()), this, SLOT (readyForUnload()) );

	m_pic = new QImage;
}

URLPicPreviewPlugin::~URLPicPreviewPlugin()
{

	kDebug ( 14314 ) << "Removing temporary files...";
	for ( int i = 0; i < m_tmpFileRegistry.count(); i++ )
	{
		KIO::NetAccess::removeTempFile ( m_tmpFileRegistry[i] );
	}

	disconnect ( this, SLOT (aboutToDisplay(Kopete::Message&)) );

	delete m_pic;

	kDebug ( 14314 );
}

/*!
    \fn URLPicPreviewPlugin::aboutToDiplay(Kopete::Message& message)
 */
void URLPicPreviewPlugin::aboutToDisplay ( Kopete::Message& message )
{
	if ( message.direction() == Kopete::Message::Inbound )
	{
		// reread configuration
		URLPicPreviewConfig::self()->readConfig();

		QRegExp ex ( "(<a href=\")([^\"]*)(\" )?([^<]*)(</a>)(.*)$" );
		QString myParsedBody = message.parsedBody();		
		if ( ex.indexIn ( myParsedBody ) != -1 )
		{
			// Only change message if it contains urls
			message.setHtmlBody ( prepareBody ( myParsedBody ) );
		}
	}
}

/**
 * @brief Recursively searches the message, downloads and replaces all found imgages
 *
 * @param parsedBody the parsed body of the message
 *
 * @return a new message body with the images as preview
 */
QString URLPicPreviewPlugin::prepareBody ( const QString& parsedBody, uint previewCount )
{

	kDebug ( 14314 ) << "Searching for URLs to pictures";

	static const QString rex = "(<a href=\")([^\"]*)(\" )?([^<]*)(</a>)(.*)$";
	//             Caps:          1           2        3     4      5    6

	QRegExp ex ( rex );
	QString myParsedBody = parsedBody;

	kDebug ( 14314 ) << "Analyzing message: \"" << myParsedBody << "\"";

	if ( ex.indexIn ( myParsedBody ) == -1 || ( previewCount >= URLPicPreviewConfig::self()->previewAmount() ) || m_abortMessageCheck )
	{
		kDebug ( 14314 ) << "No more URLs found in message.";
		return myParsedBody;
	}

	QString foundURL = ex.cap ( 2 );
	KUrl url ( foundURL );
	QString tmpFile;

	kDebug ( 14314 ) << "Found an URL: " << foundURL;

	if ( url.isValid() )
	{
		kDebug ( 14314 ) << "URL \"" << foundURL << "\" is valid.";

		if ( !( tmpFile = createPreviewPicture ( url ) ).isEmpty() )
		{
			if ( URLPicPreviewConfig::self()->scaling() )
			{
				int width = URLPicPreviewConfig::self()->previewScaleWidth();
				kDebug ( 14314 ) << "Try to scale the image to width: " << width;
				if ( m_pic->load ( tmpFile ) )
				{
					// resize but keep aspect ratio
					if ( m_pic->width() > width )
					{
						if ( ! ( (*m_pic = m_pic->scaledToWidth ( width ) ) ).save ( tmpFile, "PNG" ) )
						{
							kWarning ( 14314 ) << "Could not save scaled image" << tmpFile;
						}
					}
				}
				else
				{
					kWarning ( 14314 ) << "Could not load image " << tmpFile;
				}
			}

			myParsedBody.replace ( QRegExp ( rex ), QString ( "<a href=\"%1\" title=\"%2\">%3</a><br /><img align=\"center\" src=\"%4\" title=\"" + i18n ( "Preview of:" ) + " %5\" /><br />" ).arg ( foundURL ).arg ( foundURL ).arg ( foundURL ).arg ( tmpFile ).arg ( foundURL ) );

			if ( URLPicPreviewConfig::self()->previewRestriction() )
			{
				previewCount++;
				kDebug ( 14314 ) << "Updating previewCount: " << previewCount;
			}

			kDebug ( 14314 ) << "Registering temporary file for deletion.";
			m_tmpFileRegistry.append ( tmpFile );
			return myParsedBody + prepareBody ( ex.cap ( 6 ), previewCount );
		}
	}
	else
	{
		kWarning ( 14314 ) << "URL \"" << foundURL << "\" is invalid. Ignoring.";
	}

	return myParsedBody.replace ( QRegExp ( rex ), ex.cap ( 1 ) + ex.cap ( 2 ) + ex.cap ( 3 ) + ex.cap ( 4 ) + ex.cap ( 5 ) ) + prepareBody ( ex.cap ( 6 ), previewCount );
}

/*!
    \fn URLPicPreviewPlugin::abortAllOperations()
 */
void URLPicPreviewPlugin::readyForUnload()
{
	kDebug ( 14314 );
	m_abortMessageCheck = true;
	emit abortAllOperations();
}

/*!
    \fn URLPicPreviewPlugin::createPreviewPicture()
 */
QString URLPicPreviewPlugin::createPreviewPicture ( const KUrl& url )
{
	QString tmpFile;

	if ( !url.fileName ( ).isEmpty() &&
	        KIO::NetAccess::mimetype ( url, Kopete::UI::Global::mainWidget() ).startsWith ( "image/" ) )
	{
		if ( !KIO::NetAccess::download ( url, tmpFile, Kopete::UI::Global::mainWidget() ) )
		{
			return QString();
		}
	}
	else
	{ // Experimental
		/*
		KTempFile tmp;
		tmpFile = tmp.name();
		LinkPreview::self(this)->getPreviewPic(url).save(tmpFile, "PNG");
		*/
	}

	return tmpFile;
}

#include "urlpicpreviewplugin.moc"
