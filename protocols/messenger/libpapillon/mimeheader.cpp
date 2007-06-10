/*
   mimeheader.cpp - Create/Manage a MIME header.

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
#include "Papillon/MimeHeader"

// Qt includes
#include <QtCore/QSharedData>
#include <QtCore/QHash>
#include <QtCore/QLatin1String>
#include <QtCore/QTextStream>
#include <QtCore/QStringList>

namespace Papillon 
{

class MimeHeader::Private : public QSharedData
{
public:
	Private()
	{}

	QHash<QString, QVariant> hashMimeHeader;
	QString charset;
};

MimeHeader::MimeHeader()
 : d(new Private)
{}

MimeHeader::~MimeHeader()
{}

MimeHeader::MimeHeader(const MimeHeader &copy)
 : d(copy.d)
{}

MimeHeader &MimeHeader::operator=(const MimeHeader &other)
{
	d = other.d;
	return *this;
}

/*Convert String to MimeHeader*/
MimeHeader MimeHeader::parseMimeHeader(const QString &data)
{
	MimeHeader parsedMimeHeader;
	QString line, temp;
	temp = data;
	QTextStream parser(&temp);
	
	line = parser.readLine();
	// A MIME header is separated from its body with a empty field (\r\n),
	// so stop when we found an empty line.
	while( !line.isEmpty() )
	{
		int firstColonPosition = line.indexOf( QChar(':') );
		// This is a MIME line
		if( firstColonPosition != -1 )
		{
			QString key = line.left(firstColonPosition);
			QString value = line.mid(firstColonPosition+1).trimmed(); // Skip the space

			// Look for charset value.
			if( key.toLower() == QLatin1String("content-type") )
			{
				// Only extract charset if present.
				if( value.indexOf( QChar(';') ) != 1 )
				{
					QStringList args = value.split( QChar(';') );
					value = args[0];
					QString charset = args[1].section( QChar('='), 1, 1 );
					parsedMimeHeader.setCharset(charset);
				}
			}
			parsedMimeHeader.setValue(key, QVariant(value));
		}
		line = parser.readLine();
	}

	return parsedMimeHeader;
}


bool MimeHeader::isValid() const
{
	return d->hashMimeHeader.isEmpty();
}

bool MimeHeader::hasKey(const QString &key) const
{
	return d->hashMimeHeader.contains(key);
}

QVariant MimeHeader::value(const QString &key) const
{
	return d->hashMimeHeader.value(key);
}

void MimeHeader::setValue(const QString &key, const QVariant &value)
{
	d->hashMimeHeader.insert(key, value);
}

QString MimeHeader::mimeVersion() const
{
	return value( QLatin1String("MIME-Version") ).toString();
}

void MimeHeader::setMimeVersion(const QString &mimeVersion)
{
	setValue( QLatin1String("MIME-Version"), mimeVersion );
}

QString MimeHeader::contentType() const
{
	return value( QLatin1String("Content-Type") ).toString();
}
void MimeHeader::setContentType(const QString &contentType)
{
	setValue( QLatin1String("Content-Type"), contentType );
}

QString MimeHeader::charset() const
{
	return d->charset;
}

void MimeHeader::setCharset(const QString &charset)
{
	d->charset = charset;
}

/*Convert MimeHeader object to QString*/
QString MimeHeader::toString() const
{
	QString result;
	QHash<QString, QVariant> generateHash = d->hashMimeHeader;
	QString currentKey = QLatin1String("MIME-Version");
	// Always put MIME-Version and Content-Type first
	if( hasKey(currentKey) )
		result += QString("%1: %2\r\n").arg( currentKey ).arg( generateHash.take(currentKey).toString() );

	currentKey = QLatin1String("Content-Type");
	if( hasKey(currentKey) )
	{
		result += QString("%1: %2").arg( currentKey ).arg( generateHash.take(currentKey).toString() );
		// Append charset to Content-Type field if needed
		if( !charset().isEmpty() )
			result += QString("; charset=%1").arg( charset() );
		result += QLatin1String("\r\n");
	}

	QHash<QString, QVariant>::ConstIterator it, itEnd = generateHash.constEnd();
	for( it = generateHash.constBegin(); it != itEnd; ++it )
	{
		QString temp;
		temp = QString("%1: %2\r\n").arg( it.key() ).arg( it.value().toString() );

		result += temp;
	}

	return result;
}

}
