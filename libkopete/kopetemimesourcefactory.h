/*
    kopetemimesourcefactory.h - Kopete mime source factory

    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>

    Kopete    (c) 2004      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEMIMESOURCEFACTORY_H
#define KOPETEMIMESOURCEFACTORY_H

#include <qmime.h>

#include "kopete_export.h"

namespace Kopete
{

/**
 * @brief A mime source factory for providing kopete's various icons for labels and tooltips
 *
 * The following 'protocols' are supported, and provide appropriate icons for
 * various situations:
 *  kopete-contact-icon:\<protocolId\>:\<accountId\>:\<contactId\>
 *  kopete-account-icon:\<protocolId\>:\<accountId\>
 *  kopete-metacontact-icon:\<metaContactId\>
 * Note that the various id strings should be URL-encoded (with, for instance,
 * KURL::encode_string) if they might contain colons.
 */
class KOPETE_EXPORT MimeSourceFactory : public QMimeSourceFactory
{
public:
	MimeSourceFactory();
	~MimeSourceFactory();

	const QMimeSource *data( const QString &abs_name ) const;

private:
	class Private;
	Private *d;
};

} // Kopete

#endif
// vim: set noet ts=4 sts=4 sw=4:
