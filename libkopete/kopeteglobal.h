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
		friend class ContactPropertyTmpl;
		public:
			static Properties *self();

			const ContactPropertyTmpl &tmpl(const QString &key) const;

			const ContactPropertyTmpl &fullName() const;
			const ContactPropertyTmpl &idleTime() const;
			const ContactPropertyTmpl &onlineSince() const;
			const ContactPropertyTmpl &lastSeen() const;
			const ContactPropertyTmpl &awayMessage() const;
			const ContactPropertyTmpl &firstName() const;
			const ContactPropertyTmpl &lastName() const;
			const ContactPropertyTmpl &emailAddress() const;
			const ContactPropertyTmpl &privatePhone() const;
			const ContactPropertyTmpl &privateMobilePhone() const;
			const ContactPropertyTmpl &workPhone() const;
			const ContactPropertyTmpl &workMobilePhone() const;

			const ContactPropertyTmpl::Map &templateMap() const;
			/**
			 * return true is a template with key @p key is already registered,
			 * false otherwise
			 */
			bool isRegistered(const QString &key);

		private:
			Properties();
			~Properties();

		protected:
			bool registerTemplate(const QString &key,
				const ContactPropertyTmpl &tmpl);
			void unregisterTemplate(const QString &key);

			const ContactPropertyTmpl &Properties::createProp(const QString &key,
				const QString &label, const QString &icon=QString::null) const;

		private:
			ContactPropertyTmpl::Map mTemplates;
			ContactPropertyTmpl mFullName;
			ContactPropertyTmpl mIdleTime;
			ContactPropertyTmpl mOnlineSince;
			ContactPropertyTmpl mLastSeen;
			ContactPropertyTmpl mAwayMessage;

			static Properties *mSelf;
	}; // END class Properties

} // Global

} // Kopete

#endif
// vim: set noet ts=4 sts=4 sw=4:
