#ifndef JINGLE_CONTENT_H
#define JINGLE_CONTENT_H

#include <QObject>

#include "im.h"

class QHostAddress;
class QDomElement;
class QUdpSocket;
namespace XMPP
{
	/*
	 * This class contains all informations about a particular content in a jingle session.
	 * It also has the socket that will be used for streaming.
	 */
	//This is the Raw-udp jingle content.
	class IRIS_EXPORT JingleContent : public QObject
	{
		Q_OBJECT
	public:
		JingleContent();
		~JingleContent();

		/**
		 * Defines the content type, this represesent the media attribute.
		 */
		enum Type {
			Audio = 0,
			Video,
			FileTransfer,
			Unknown
		};

		/*
		 * Adds a payload type to this content.
		 */
		void addPayloadType(const QDomElement&);
		
		/*
		 * Adds a payload type list to this content.
		 */
		void addPayloadTypes(const QList<QDomElement>&);

		/*
		 * Overwrite the current payload types list with this one.
		 */
		void setPayloadTypes(const QList<QDomElement>&);

		/*
		 * Sets the transport for this content.
		 */
		void setTransport(const QDomElement&);
		void setType(Type);
		Type type() const;
		void setCreator(const QString&);
		void setName(const QString&);
		void setDescriptionNS(const QString&);

		QList<QDomElement> payloadTypes() const;
		QDomElement transport() const;
		void fromElement(const QDomElement&);
		QDomElement contentElement();
		QList<QDomElement> candidates() const;
		void addCandidate(const QDomElement&);
		QString creator() const;
		QString name() const;
		QString descriptionNS() const;
		void addTransportInfo(const QDomElement&);
		QString iceUdpPassword();
		QString iceUdpUFrag();
		void createUdpInSocket();
		QUdpSocket *inSocket();
		QUdpSocket *outSocket();
		bool sending();
		void setSending(bool);
		bool receiving();
		void setReceiving(bool);

		void startSending();
		void startSending(const QHostAddress&, int);

		void bind(const QHostAddress&, int);
		
		JingleContent& operator=(const JingleContent&);
		
		QString typeToString(Type);
		Type stringToType(const QString& s);

		void setResponderPayloads(const QList<QDomElement>&);

		QDomElement bestPayload() const;

	public slots:
		void slotRawUdpDataReady();

		void slotTrySending();

	signals:

		// Emitted when the content is ready to send data to try to connect.
		void needData(XMPP::JingleContent*);
		
		// Emitted when the IN socket is ready to receive data (it is bound).
		// Can be used to prepare a rtp session with the socket.
		void inSocketReady();
		
		// Emitted when the OUT socket is ready to send data (it is connected).
		// Can be used to prepare a rtp session with the socket.
		void outSocketReady();

		/**
		 * Emitted when sending and receiving streams have been established for this content 
		 */
		void established();

		void dataReceived();

	private:
		class Private;
		Private *d;
		
		QDomElement bestPayload(const QList<QDomElement>&, const QList<QDomElement>&);
		bool samePayload(const QDomElement&, const QDomElement&);
	};
}

#endif
