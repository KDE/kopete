/*
 * jinglertpapplication.h - Jingle RTP Application
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

#ifndef JINGLE_RTP_APPLICATION_H
#define JINGLE_RTP_APPLICATION_H

#include <QObject>

#include "jingleapplication.h"

namespace XMPP
{
	/**
	 * The JingleRtpApplication class manages an RTP Jingle session.
	 * 
	 * This class will give give all information about RTP events like the
	 * ringing part or holding and muting a session.
	 * 
	 * This is also the class you use to create a JingleContent which represents
	 * an RTP session.
	 * You can add all content types you need and want to be used by the RTP
	 * session, in order of preference.
	 */
	class JingleRtpApplication : public JingleApplication
	{
		Q_OBJECT
	public :
		JingleRtpApplication(JingleContent *parent = 0);
		JingleRtpApplication(const QDomElement&, JingleContent *parent = 0);
		~JingleRtpApplication();

		virtual void init();

		/*
		 * Adds a payload type to this content.
		 */
		void addPayload(const QDomElement&);
		
		/*
		 * Adds a payload type list to this content.
		 */
		void addPayloads(const QList<QDomElement>&);

		/*
		 * Overwrite the current payload types list with this one.
		 */
		void setPayloads(const QList<QDomElement>&);
		
		/*
		 * Returns the payload type list. Those payloads are
		 * our payloads if in Pending state or the content
		 * used payloads if in Active state. (TODO)
		 */
		QList<QDomElement> payloads() const;
		
		virtual JingleApplication* mergeWith(JingleApplication *other);
		
		virtual bool isCompatibleWith(JingleApplication*);

		virtual QDomElement toXml(const ApplicationType);

		static QDomElement bestPayload(const QList<QDomElement>& payload1,
						const QList<QDomElement>& payload2);
		static bool samePayload(const QDomElement& p1, const QDomElement& p2);
		
		virtual void sessionInfo(const QDomElement&);

		virtual int componentCountNeeded();

		enum State {
			Active = 0,
			Muted,
			Held,
			Ringing
		};
	
	signals :
		/**
		 * Three signals emitted when the remote peer wants
		 * to mute, hold or set active again the RTP session.
		 */
		void mute();
		void hold();
		void active();
		void ringing();

	private :
		class Private;
		Private *d;

		void fromXml(const QDomElement&);
	};
}

#endif //JINGLE_RTP_APPLICATION_H
