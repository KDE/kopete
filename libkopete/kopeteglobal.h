/*
    kopeteglobal.h - Kopete Globals

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

#ifndef KOPETEGLOBAL_H
#define KOPETEGLOBAL_H

#include "kopetecontactproperty.h"

//class QString;

namespace Kopete
{

/**
 * This namespace contains Kopete's global settings and functions
 */
namespace Global
{
	/**
	 * \brief Installs one or more kopete emoticon themes from a tarball
	 * (either .kopete-emoticons or .tar.gz or .tar.bz2)
	 *
	 * @p archiveName Full path to a local emoticon archive, use KIO to download
	 * files in case their are non-local.
	 *
	 * @return true in case install was successful, false otherwise. Errors are
	 * displayed by either KIO or by using KMessagebox directly.
	 *
	 * TODO: If possible, port it to KIO instead of using ugly blocking KTar
	 **/
	void installEmoticonTheme(const QString &localPath);

	/**
	 * \brief TODO
	 **/
	class Properties
	{
		public:
			static Properties *self();
			const ContactProperty &property(const QString &key) const;
			const ContactProperty::Map &map() const;

		private:
			Properties();

		private:
			static Properties *mSelf;
			ContactProperty::Map mProps;
	}; // END class Properties

} // Global

} // Kopete

#endif
// vim: set noet ts=4 sts=4 sw=4:
