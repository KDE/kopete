/*
    kopetepicture.h - Kopete Picture

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
#ifndef KOPETEPICTURE_H
#define KOPETEPICTURE_H

#include <kdemacros.h>
#include <ksharedptr.h>
#include "kopete_export.h"

#include <QtGui/QImage>

namespace KABC
{
	class Picture;
}

namespace Kopete
{
/**
 * @brief Represent a picture in Kopete context
 *
 * It kept a cache of a QImage object, a base64 string and
 * a path to a image file. It ensure that all source are synced.
 * Interally, the image is stored in PNG format when possible.
 * It can happen that the image path do not return a PNG file.
 *
 * You can only use an QImage and a image path to create/update
 * the picture.
 * If the picture doesn't exist as a file, it generate a local
 * copy into ~/.kde/share/apps/kopete/metacontactpicturecache
 *
 * This class is implicitly shared, so don't use it as a pointer.
 *
 * How to use this class:
 * @code
 * Kopete::Picture picture;
 * picture.setPicture(QImage());
 * picture.setPicture(QString("/tmp/image.png"));
 * 
 * QString base64 = picture.base64();
 * QString path = picture.path();
 * QImage image = picture.image();
 * @endcode
 *
 * @author Michaël Larouche <larouche@kde.org>
 */
class KOPETE_EXPORT Picture	
{
public:
	/**
	 * Create a empty Kopete::Picture
	 */
	Picture();
	/**
	 * Create a picture from a local path.
	 */
	explicit Picture(const QString &path);
	/**
	 * Create a picture from a QImage.
	 */
	explicit Picture(const QImage &image);
	/**
	 * Create a picture from a KABC::Picture.
	 */
	explicit Picture(const KABC::Picture &picture);
	/**
	 * Copy a picture. It doesn't create a full copy, it just make a reference.
	 */
	Picture(const Picture &other);
	/**
	 * Delete the Kopete::Picture
	 */
	~Picture();
	/**
	 * Assignment operator.
	 * Like the copy constructor, it just make a reference.
	 */
	Picture &operator=(const Picture &other);

	/**
	 * Return the current picture as QImage.
	 * QImage can used to draw the image on a context.
	 * 
	 * @return the QImage cache of current picture.
	 */
	QImage image();
	/**
	 * Return the current picture as a base64 string.
	 * The base64 is used to include the picture into a XML/XHTML context.
	 */
	QString base64();
	/**
	 * Return the local path of the current picture.
	 */
	QString path();

	/**
	 * Check if the picture is null.
 	 */
	bool isNull();
	/**
	 * Reset the picture.
	 */
	void clear();

	/**
	 * Set the picture content.
	 * @param image the picture as a QImage.
	 */
	void setPicture(const QImage &image);
	/**
	 * Set the picture content.
	 * @param path the path to the picture.
	 */
	void setPicture(const QString &path);
	/**
	 * Set the picture content.
	 * @param picture a KABC Picture.
	 */
	void setPicture(const KABC::Picture &picture);
	
private:
	/**
	 * Kopete::Picture is implicitly shared.
	 * Detach the instance when modifying data.
	 */
	void detach();

	class Private;
	KSharedPtr<Private> d;
};

}//END namespace Kopete

#endif
