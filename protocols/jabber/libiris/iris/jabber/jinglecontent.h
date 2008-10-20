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

		/*
		 * Set the content type, this will set the "media" attribute of
		 * the content tag in the stanza.
		 */
		void setType(Type);

		/*
		 * Gets the type of this content.
		 */
		Type type() const;

		/*
		 * Set the creator of this content, the creator only accept 2 values :
		 * 	* initiator
		 * 	* responder
		 * TODO:An enum should be created to avoid confusion
		 */
		void setCreator(const QString&);
		
		/*
		 * Set this content's name
		 */
		void setName(const QString&);

		/*
		 * Set this content description namespace.
		 * The only one supported currently is
		 * 	NS
		 */
		void setDescriptionNS(const QString&);

		/*
		 * Returns the payload type list. those payloads are
		 * our payloads if in Pending state or the content
		 * used payloads if in Active state. (TODO)
		 */
		QList<QDomElement> payloadTypes() const;

		/*
		 * Returns the transport XML element for this content.
		 */
		QDomElement transport() const;

		/*
		 * Fill this content from a QDomElement.
		 * The payloads in this QDomElement will be considered as the responder's
		 * TODO:add an argument to tell the method if those payloads are our's or
		 * responder's payloads.
		 */
		void fromElement(const QDomElement&);

		/*
		 * Return a QDomElement with the content element and all it's children
		 * so it's ready to be sent.
		 */
		QDomElement contentElement();

		/*
		 * Returns a list with the available candidates for this content.
		 * TODO:should return the used candidate when in Active state.
		 */
		QList<QDomElement> candidates() const;

		/*
		 * Adds a candidate to this content. Doing so will add this content(s)
		 * to the transport when calling contentElement()
		 */
		void addCandidate(const QDomElement&);
		
		/*
		 * Adds transport info (mostly a candidate). Doing so will try to
		 * connect to this candidate.
		 */
		void addTransportInfo(const QDomElement&);
		void createUdpInSocket();
		
		QString creator() const;
		QString name() const;
		QString descriptionNS() const;
		QString iceUdpPassword();
		QString iceUdpUFrag();
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
		QList<QDomElement> responderPayloads() const;

		QDomElement bestPayload();

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
