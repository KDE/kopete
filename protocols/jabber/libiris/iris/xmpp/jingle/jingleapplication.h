/*
 * jingleapplication.h - Jingle Application
 * Copyright (C) 2009 - Detlev Casanova <detlev.casanova@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef JINGLEAPPLICATION_H
#define JINGLEAPPLICATION_H

/*
 * The form of an application is a <description> element
 * which has childs describing the application.
 * For Rtp sessions, this will be payloads.
 */

#include <QObject>
#include <QDomElement>

namespace XMPP
{
	class JingleContent;
	class JingleApplication : public QObject
	{
		Q_OBJECT
	public :
		enum ApplicationType {
			NoApplication = 0,
			LocalApplication,
			RemoteApplication,
			Application
			//AcceptableApplication,
		};

		/**
		 * Defines the content type, this represesent the media attribute.
		 */
		enum MediaType {
			Audio = 0,
			Video,
			FileTransfer,
			NoType
		};

		JingleApplication(JingleContent *parent = 0);
		virtual ~JingleApplication();

		static JingleApplication* createFromXml(const QDomElement&, JingleContent *parent = 0);

		virtual void init() = 0;

		void setParent(JingleContent *c);
		
		/*
		 * Set the content type, this will set the "media" attribute of
		 * the content tag in the stanza.
		 */
		void setMediaType(MediaType);
		
		/*
		 * Gets the type of this content.
		 */
		MediaType mediaType() const;
		
		/*
		 * Set this content description namespace.
		 */
		void setDescriptionNS(const QString&);
		
		QString descriptionNS() const;
		
		QString mediaTypeToString(MediaType);
		MediaType stringToMediaType(const QString& s);
		
		virtual QDomElement toXml(const ApplicationType) = 0;

		/*
		 * This method merges this application with another one and
		 * returns the result (no JingleApplication is modified)
		 * In the case of RTP, it will be called on the local and
		 * the remote application and return an application which
		 * contains the acceptable payloads.
		 */
		virtual JingleApplication *mergeWith(JingleApplication*) = 0;

		/*
		 * Returns true if both JingleApplication's are compatible.
		 * For RTP, this would mean that both have the same media type
		 * and at least one common payload type.
		 */
		virtual bool isCompatibleWith(JingleApplication*) = 0;
		
		void setEncryption(bool);
		bool encryption() const;

		void setEncryptionRequired(bool);
		bool encryptionRequired() const;

		JingleContent* parent() const;

		virtual void sessionInfo(const QDomElement&) = 0;

		virtual int componentCountNeeded() = 0;

	private :
		class Private;
		Private *d;
	};
}

#endif //JINGLEAPPLICATION_H
