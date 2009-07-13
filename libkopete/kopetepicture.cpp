/*
    kopetepicture.cpp - Kopete Picture

    Copyright (c) 2005      by Michaël Larouche       <larouche@kde.org>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#include "kopetepicture.h"

#include <qbuffer.h>

#include <kabc/picture.h>

#include <kcodecs.h>
#include <kstandarddirs.h>
#include <kdebug.h>


namespace Kopete
{

class Picture::Private : public KShared
{
public:
	Private()
	{}

	QString pictureBase64;
	QImage pictureImage;
	QString picturePath;
};

Picture::Picture()
 : d(new Private)
{
}

Picture::Picture(const QString &path)
 : d(new Private)
{
	setPicture(path);
}

Picture::Picture(const QImage &image)
 : d(new Private)
{
	setPicture(image);
}

Picture::Picture(const KABC::Picture &picture)
 : d(new Private)
{
	setPicture(picture);
}

Picture::Picture(const Picture &other)
 : d(other.d)
{}

Picture::~Picture()
{}

Picture &Picture::operator=(const Picture &other)
{
	d = other.d;
	return *this;
}

QImage Picture::image()
{
	// Do the conversion if only needed.
	// If the image is null, the path is not empty then.
	if( d->pictureImage.isNull() )
	{
		d->pictureImage = QImage(d->picturePath);
	}

	return d->pictureImage;
}

QString Picture::base64()
{
	if( d->pictureBase64.isEmpty() )
	{
		// Generate base64 cache for the picture.
		QByteArray tempArray;
		QBuffer tempBuffer( &tempArray );
		tempBuffer.open( QIODevice::WriteOnly );
		// Make sure it create a image cache.
		if( image().save( &tempBuffer, "PNG" ) )
		{
			d->pictureBase64 = tempArray.toBase64();
		}
	}

	return d->pictureBase64;
}

QString Picture::path()
{
	if( d->picturePath.isEmpty() )
	{
		// For a image source, finding a filename is tricky.
		// I decided to use MD5 Hash as the filename.
		QString localPhotoPath;
		
		// Generate MD5 Hash for the image.
		QByteArray tempArray;
		QBuffer tempBuffer(&tempArray);
		tempBuffer.open( QIODevice::WriteOnly );
		image().save(&tempBuffer, "PNG");
		KMD5 context(tempArray);
		// Save the image to a file.
		localPhotoPath = context.hexDigest() + ".png";
		localPhotoPath = KStandardDirs::locateLocal( "appdata", QString::fromUtf8("metacontactpicturecache/%1").arg( localPhotoPath) );
		if( image().save(localPhotoPath, "PNG") )
		{
			d->picturePath = localPhotoPath;
		}
	}

	return d->picturePath;
}

bool Picture::isNull()
{
	if( d->pictureBase64.isEmpty() && d->picturePath.isEmpty() && d->pictureImage.isNull() )
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Picture::clear()
{
	detach();
	d->pictureBase64.clear();
	d->picturePath.clear();
	d->pictureImage = QImage();
}

void Picture::setPicture(const QImage &image)
{
	detach();

	d->pictureImage = image;

	// Clear the path and base64, it will call the update of then when "getted"
	d->picturePath.clear();
	d->pictureBase64.clear();
}

void Picture::setPicture(const QString &path)
{
	detach();
	d->picturePath = path;
	
	// Clear the image and base64, it will call the update of then when "getted"
	d->pictureImage = QImage();
	d->pictureBase64.clear();
}

void Picture::setPicture(const KABC::Picture &picture)
{
	// No need to call detach() here because setPicture will do it.
	if ( picture.isIntern())
	{
		setPicture( picture.data() );
	}
	else
	{
		setPicture( picture.url() );
	}
}

void Picture::detach()
{
	// there is no detach in KSharedPtr.
	if( d.count() == 1 )
		return;

	// Warning: this only works as long as the private object doesn't contain pointers to allocated objects.
	d = new Private(*d);
}

} // END namespace Kopete
