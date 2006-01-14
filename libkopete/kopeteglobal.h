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

#include "kopete_export.h"

/**
 * This namespace contains all of Kopete's core classes and functions.
 */
namespace Kopete
{

/**
 * This namespace contains Kopete's global settings and functions
 */
namespace Global
{
	class PropertiesPrivate;

	/**
	 * \brief Installs one or more kopete emoticon themes from a tarball
	 * (either .kopete-emoticons or .tar.gz or .tar.bz2)
	 *
	 * @p localPath Full path to a local emoticon archive, use KIO to download
	 * files in case their are non-local.
	 *
	 * @return true in case install was successful, false otherwise. Errors are
	 * displayed by either KIO or by using KMessagebox directly.
	 *
	 * TODO: If possible, port it to KIO instead of using ugly blocking KTar
	 **/
	KOPETE_EXPORT void installEmoticonTheme(const QString &localPath);

	/**
	 * \brief Global facility to query/store templates that are needed by KopeteContactProperty
	 *
	 * Basically all a plugin author needs to worry about is creating ContactPropertyTmpl
	 * objects for all the properties he wants to set for a Kopete::Contact,
	 * everything else is handled behind the scenes.
	 **/
	class KOPETE_EXPORT Properties
	{
		friend class Kopete::ContactPropertyTmpl;
		public:
			/**
			 * \brief Singleton accessor for this class.
			 *
			 * Use it to access the global list of property-templates or to get
			 * a reference to one of the common ContactPropertyTmpl objects
			 */
			static Properties *self();

			/**
			 * Return a template with defined by @p key, if no such template has
			 * been registered ContactPropertyTmpl::null will be returned
			 */
			const ContactPropertyTmpl &tmpl(const QString &key) const;

			/**
			 * @return a ready-to-use template for a contact's full name.
			 *
			 * This is actually no real property, it makes use of
			 * firstName() and lastName() to assemble an name that consists of
			 * both name parts
			 */
			const ContactPropertyTmpl &fullName() const;

			/**
			 * Return default template for a contact's idle-time
			 */
			const ContactPropertyTmpl &idleTime() const;
			/**
			 * Return default template for a contact's online-since time
			 * (i.e. time since he went from offline to online)
			 */
			const ContactPropertyTmpl &onlineSince() const;
			/**
			 * @return default template for a contact's last-seen time
			 */
			const ContactPropertyTmpl &lastSeen() const;
			/**
			 * @return default template for a contact's away-message
			 */
			const ContactPropertyTmpl &awayMessage() const;
			/**
			 * @return default template for a contact's first name
			 */
			const ContactPropertyTmpl &firstName() const;
			/**
			 * @return default template for a contact's last name
			 */
			const ContactPropertyTmpl &lastName() const;
			/**
			 * @return default template for a contact's email-address
			 */
			const ContactPropertyTmpl &emailAddress() const;
			/**
			 * @return default template for a contact's private phone number
			 */
			const ContactPropertyTmpl &privatePhone() const;
			/**
			 * @return default template for a contact's private mobile number
			 */
			const ContactPropertyTmpl &privateMobilePhone() const;
			/**
			 * @return default template for a contact's work phone number
			 */
			const ContactPropertyTmpl &workPhone() const;
			/**
			 * @return default template for a contact's work mobile number
			 */
			const ContactPropertyTmpl &workMobilePhone() const;
			/**
			 * @return default template for a contact's nickname (set by the contact)
			 */
			const ContactPropertyTmpl &nickName() const;
			/**
			 * default template for a contact's photo.
			 *
			 * It could be either a QString or a QImage.
			 * If it's a QString, it should points to the path the image is stored.
			 */
			const ContactPropertyTmpl &photo() const;

			/**
			 * @return a map of all registered ContactPropertyTmpl object
			 */
			const ContactPropertyTmpl::Map &templateMap() const;

			/**
			 * return true if a template with key @p key is already registered,
			 * false otherwise
			 */
			bool isRegistered(const QString &key);

		private:
			Properties();
			~Properties();

			bool registerTemplate(const QString &key,
				const ContactPropertyTmpl &tmpl);
			void unregisterTemplate(const QString &key);

			const ContactPropertyTmpl &createProp(const QString &key,
				const QString &label, const QString &icon=QString::null,
				bool persistent = false) const;

		private:
			static Properties *mSelf;
			PropertiesPrivate *d;
	}; // end class Properties

} // Global

} // Kopete

#endif
// vim: set noet ts=4 sts=4 sw=4:
