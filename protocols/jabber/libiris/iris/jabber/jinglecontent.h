#ifndef JINGLE_CONTENT_H
#define JINGLE_CONTENT_H

#include <QObject>
#include <QDomElement>
#include <QUdpSocket>

#include "im.h"

namespace XMPP
{
	/*
	 * This class contains all informations about a particular content in a jingle session.
	 * It also has the socket that will be used for streaming.
	 */
	class IRIS_EXPORT JingleContent : public QObject
	{
		Q_OBJECT
	public:
		JingleContent();
		~JingleContent();
		enum Type {
			Audio = 0,
			Video,
			FileTransfer,
			Unknown
		};

		void addPayloadType(const QDomElement&);
		void addPayloadTypes(const QList<QDomElement>&);
		void setTransport(const QDomElement&);
		void setType(Type);
		void setCreator(const QString&);
		void setName(const QString&);
		void setDescriptionNS(const QString&);
		void setProfile(const QString&);

		QList<QDomElement> payloadTypes() const;
		QDomElement transport() const;
		void fromElement(const QDomElement&);
		QDomElement contentElement();
		QList<QDomElement> candidates() const;
		QString creator() const;
		QString profile() const;
		QString name() const;
		QString descriptionNS() const;
		Type dataType();
		void addTransportInfo(const QDomElement&);
		QString iceUdpPassword();
		QString iceUdpUFrag();
		void createUdpInSocket();
		QUdpSocket *socket(); // FIXME:Is it socket for data IN or for data OUT ?
				      //       Currently, it's data IN.
		bool sending();
		void setSending(bool);
		bool receiving();
		void setReceiving(bool);

		void startSending();
		
		JingleContent& operator=(const JingleContent&);
	signals:
		void rawUdpDataReady();
		void needData(XMPP::JingleContent*);

	private:
		class Private;
		Private *d;
	};
}

#endif
