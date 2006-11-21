/*
   papillonstatusmessage.cpp - Dynamic Personal Status Message for Windows Live Messenger.

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
 */
#include "Papillon/StatusMessage"

// Qt includes
#include <QtCore/QSharedData>
#include <QtCore/QLatin1String>
#include <QtCore/QStringList>
#include <QtDebug>
#include <QtXml/QDomDocument>
#include <QtXml/QDomNode>
#include <QtXml/QDomElement>

namespace Papillon
{

class StatusMessage::Private : public QSharedData
{
public:
	Private()
	 : currentMediaEnabled(true), currentMediaType(Papillon::OnlineStatus::MediaNone)
	{}

	QString plainMessage;
	QString application;
	QString formatterString;
	
	bool currentMediaEnabled;

	Papillon::OnlineStatus::MediaType currentMediaType;

	QList<QVariant> arguments;

	static QString mediaTypeToString(Papillon::OnlineStatus::MediaType);
	static Papillon::OnlineStatus::MediaType mediaTypeFromString(const QString &media);
};

StatusMessage::StatusMessage()
 : d(new Private)
{}

StatusMessage::StatusMessage(const QString &plainStatusMessage)
 : d(new Private)
{
	d->plainMessage = plainStatusMessage;
}

StatusMessage::StatusMessage(const StatusMessage &copy)
 : d(copy.d)
{}

StatusMessage &StatusMessage::operator=(const StatusMessage &other)
{
	d = other.d;

	return *this;
}

StatusMessage::~StatusMessage()
{
}

void StatusMessage::clear()
{
	d->plainMessage.clear();
	d->formatterString.clear();
	d->currentMediaEnabled = true;
	d->application.clear();
	d->currentMediaType = Papillon::OnlineStatus::MediaNone;
	d->arguments.clear();
}

QString StatusMessage::message() const
{
	return d->plainMessage;
}

void StatusMessage::setMessage(const QString &message)
{
	d->plainMessage = message;
}

void StatusMessage::setCurrentMediaEnabled(bool enabled)
{
	d->currentMediaEnabled = enabled;
}

void StatusMessage::setCurrentMediaApplication(const QString &application)
{
	d->application = application;
}

void StatusMessage::setCurrentMediaType(Papillon::OnlineStatus::MediaType type)
{
	d->currentMediaType = type;
}

void StatusMessage::setCurrentMediaFormatterString(const QString &formatterString)
{
	d->formatterString = formatterString;
}

void StatusMessage::setCurrentMediaArguments(const QList<QVariant> &args)
{
	d->arguments = args;
}

bool StatusMessage::isCurrentMediaEnabled() const
{
	return d->currentMediaEnabled;
}

QString StatusMessage::currentMediaApplication() const
{
	return d->application;
}

Papillon::OnlineStatus::MediaType StatusMessage::currentMediaType() const
{
	return d->currentMediaType;
}

QString StatusMessage::currentMediaFormatterString() const
{
	return d->formatterString;
}

QList<QVariant> StatusMessage::currentMediaArguments() const
{
	return d->arguments;
}

QString StatusMessage::formattedMediaString() const
{
	QString result = d->formatterString;
	
	for(int i = 0; i < d->arguments.size(); i++)
	{
		result = result.replace( QString("{%1}").arg(i), d->arguments.at(i).toString() );
	}

	return result;
}

StatusMessage StatusMessage::fromXml(const QString &xml)
{
	QDomDocument psm;
	Papillon::StatusMessage statusMessage;

	if( psm.setContent(xml) )
	{
		// <Data><PSM>My Personal Message</PSM><CurrentMedia></CurrentMedia></Data>
		/*
			The value of the CurrentMedia tag you can think of like an array
			separated by "\0" characters (literal backslash followed by zero, not NULL).
	
			The elements of this "array" are as follows:
	
			* Application - This is the app you are using. Usually empty
			* Type - This is the type of PSM, either “Music”, “Games” or “Office”
			* Enabled - This is a boolean value (0/1) to enable/disable
			* Format - A formatter string ala .Net; For example, “{0} - {1}”
			* First line - The first line (Matches {0} in the Format)
			* Second line - The second line (Matches {1} in the Format)
			* Third line - The third line (Matches {2} in the Format)
	
			There is probably no limit to the number of lines unless you go over the maximum length of the tag.
	
			Example of currentMedia xml tag:
			<CurrentMedia>\0Music\01\0{0} - {1}\0 Song Title\0Song Artist\0Song Album\0\0</CurrentMedia>
			<CurrentMedia>\0Games\01\0Playing {0}\0Game Name\0</CurrentMedia>
			<CurrentMedia>\0Office\01\0Office Message\0Office App Name\0</CurrentMedia>
	
			From http://msnpiki.msnfanatic.com/index.php/MSNP11:Changes
		*/

		// Get the first child of the xml "document";
		QDomElement psmElement = psm.documentElement().firstChild().toElement();

		while( !psmElement.isNull() )
		{
			if( psmElement.tagName() == QLatin1String("PSM") )
			{
				statusMessage.setMessage( psmElement.text() );
			}
			else if( psmElement.tagName() == QLatin1String("CurrentMedia") )
			{
				if( !psmElement.text().isEmpty() )
				{
					QStringList mediaParameters = psmElement.text().split( QLatin1String("\\0"), QString::KeepEmptyParts );
					
					// Use take instead of at to remove the "proceeded" parameters from the string list
					statusMessage.setCurrentMediaApplication( mediaParameters.takeFirst() );
					statusMessage.setCurrentMediaType( StatusMessage::Private::mediaTypeFromString( mediaParameters.takeFirst() ) );
					statusMessage.setCurrentMediaEnabled( static_cast<bool>(mediaParameters.takeFirst().toInt()) );
					statusMessage.setCurrentMediaFormatterString( mediaParameters.takeFirst() );

					QList<QVariant> arguments;
					foreach(QString arg, mediaParameters)
					{
						arguments.append( arg );
					}
					statusMessage.setCurrentMediaArguments(arguments);
				}
			}
			// Ignore other tags such as MachineGuid.

			psmElement = psmElement.nextSibling().toElement();
		}
	}

	return statusMessage;
}

QString StatusMessage::toXml() const
{
	// <Data><PSM>My Personal Message</PSM><CurrentMedia>\0Music\01\0{0} - {1}\0 Song Title\0Song Artist\0Song Album\0\0</CurrentMedia></Data>

	// Create the root node
	QDomDocument smData;
	smData.appendChild( smData.createElement( "Data" ) );

	// Add the plain status message.
	QDomElement psm = smData.createElement("PSM");
	psm.appendChild( smData.createTextNode( d->plainMessage ) );
	smData.documentElement().appendChild( psm );

	// Add the current media node
	QDomElement currentMedia = smData.createElement("CurrentMedia");
	QString mediaString;

	// Do nothing if current media type is None.
	if( d->currentMediaType != Papillon::OnlineStatus::MediaNone )
	{
		Q_ASSERT( !d->formatterString.isEmpty() );
		Q_ASSERT( !d->arguments.isEmpty() );

		QString endArgs("\\0");
		
		// Add application name
		mediaString += d->application + endArgs;
		// Add current media type
		mediaString += d->mediaTypeToString( d->currentMediaType ) + endArgs;
		// Add isCurrentMediaEnabled
		mediaString += QString::number( (int)d->currentMediaEnabled ) + endArgs;
		// Add formatterString
		mediaString += d->formatterString + endArgs;
		// Add arguments
		foreach(QVariant arg, d->arguments)
		{
			mediaString += arg.toString() + endArgs;
		}
	}
	
	currentMedia.appendChild( smData.createTextNode( mediaString ) );
	smData.documentElement().appendChild( currentMedia );

	// Do no ident and remove line breaks.
	return smData.toString(0).replace("\n", "");
}

QString StatusMessage::Private::mediaTypeToString(Papillon::OnlineStatus::MediaType type)
{
	QString result;
	switch(type)
	{
		case Papillon::OnlineStatus::MediaMusic:
			result = QLatin1String("Music");
			break;
		case Papillon::OnlineStatus::MediaGames:
			result = QLatin1String("Games");
			break;
		case Papillon::OnlineStatus::MediaOffice:
			result = QLatin1String("Office");
			break;
		default:
			break;
	}

	return result;
}

Papillon::OnlineStatus::MediaType StatusMessage::Private::mediaTypeFromString(const QString &media)
{
	Papillon::OnlineStatus::MediaType mediaType(Papillon::OnlineStatus::MediaNone);

	if( media == QLatin1String("Music") )
		mediaType = Papillon::OnlineStatus::MediaMusic;
	else if( media == QLatin1String("Games") )
		mediaType = Papillon::OnlineStatus::MediaGames;
	else if( media == QLatin1String("Office") )
		mediaType = Papillon::OnlineStatus::MediaOffice;
	
	return mediaType;	
}

}
