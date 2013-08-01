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

#include "kopeteproperty.h"

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
	 * \brief Global facility to query/store templates that are needed by KopeteProperty
	 *
	 * Basically all a plugin author needs to worry about is creating PropertyTmpl
	 * objects for all the properties he wants to set for a Kopete::Contact,
	 * everything else is handled behind the scenes.
	 **/
	class KOPETE_EXPORT Properties
	{
		friend class Kopete::PropertyTmpl;
		public:
			/**
			 * \brief Singleton accessor for this class.
			 *
			 * Use it to access the global list of property-templates or to get
			 * a reference to one of the common PropertyTmpl objects
			 */
			static Properties *self();

			/**
			 * Return a template with defined by @p key, if no such template has
			 * been registered PropertyTmpl::null will be returned
			 */
			const PropertyTmpl &tmpl(const QString &key) const;

			/**
			 * @return a ready-to-use template for a contact's full name.
			 *
			 * This is actually no real property, it makes use of
			 * firstName() and lastName() to assemble an name that consists of
			 * both name parts
			 */
			const PropertyTmpl &fullName() const;

			/**
			 * Return default template for a contact's idle-time
			 */
			const PropertyTmpl &idleTime() const;
			/**
			 * Return default template for a contact's online-since time
			 * (i.e. time since he went from offline to online)
			 */
			const PropertyTmpl &onlineSince() const;
			/**
			 * @return default template for a contact's last-seen time
			 */
			const PropertyTmpl &lastSeen() const;
			/**
			 * @return default template for a contact's status title
			 */
			const PropertyTmpl &statusTitle() const;
			/**
			 * @return default template for a contact's status message
			 */
			const PropertyTmpl &statusMessage() const;
			/**
			 * @return default template for a contact's first name
			 */
			const PropertyTmpl &firstName() const;
			/**
			 * @return default template for a contact's last name
			 */
			const PropertyTmpl &lastName() const;
			/**
			 * @return default template for a contact's email-address
			 */
			const PropertyTmpl &emailAddress() const;
			/**
			 * @return default template for a contact's private phone number
			 */
			const PropertyTmpl &privatePhone() const;
			/**
			 * @return default template for a contact's private mobile number
			 */
			const PropertyTmpl &privateMobilePhone() const;
			/**
			 * @return default template for a contact's work phone number
			 */
			const PropertyTmpl &workPhone() const;
			/**
			 * @return default template for a contact's work mobile number
			 */
			const PropertyTmpl &workMobilePhone() const;
			/**
			 * @return default template for a contact's nickname (set by the contact)
			 * This property comes from contact and cannot be changed by user custom name
			 */
			const PropertyTmpl &nickName() const;
			/**
			 * @return default template for a contact's nickname (set by the contact)
			 * This property is set by user and stored on server contact list
			 */
			const PropertyTmpl &customName() const;
			/**
			 * @return default template for a contact's visibility even if offline
			 */
			const PropertyTmpl &isAlwaysVisible() const;
			/**
			 * default template for a contact's photo.
			 *
			 * It could be either a QString or a QImage.
			 * If it's a QString, it should points to the path the image is stored.
			 */
			const PropertyTmpl &photo() const;

			/**
			 * @return a map of all registered PropertyTmpl object
			 */
			const PropertyTmpl::Map &templateMap() const;

			/**
			 * return true if a template with key @p key is already registered,
			 * false otherwise
			 */
			bool isRegistered(const QString &key);

		private:
			Properties();
			~Properties();

			bool registerTemplate(const QString &key,
				const PropertyTmpl &tmpl);
			void unregisterTemplate(const QString &key);

			const PropertyTmpl &createProp(const QString &key,
				const QString &label, const QString &icon=QString(),
				bool persistent = false) const;

		private:
			static Properties *mSelf;
			PropertiesPrivate *d;
	}; // end class Properties

} // Global

} // Kopete

#endif
// vim: set noet ts=4 sts=4 sw=4:
