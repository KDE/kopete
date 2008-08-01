#ifndef JINGLE_CONTENT_H
#define JINGLE_CONTENT_H

#include <QObject>
#include <QDomElement>
#include <QUdpSocket>

#include "im.h"

class QHostAddress;
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

		/*
		 * Adds a payload type to this content.
		 */
		void addPayloadType(const QDomElement&);
		
		/*
		 * Adds a payload type list to this content.
		 */
		void addPayloadTypes(const QList<QDomElement>&);

		/*
		 * Sets the transport for this content.
		 */
		void setTransport(const QDomElement&);
		void setType(Type);
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
		Type dataType();
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
	public slots:
		void slotRawUdpDataReady();
	signals:

		// Emitted when the content is ready to send data to try to connect.
		void needData(XMPP::JingleContent*);
		
		// Emitted when the IN socket is ready to receive data (it is bound).
		// Can be used to prepare a rtp session with the socket.
		void inSocketReady();
		
		// Emitted when the OUT socket is ready to send data (it is connected).
		// Can be used to prepare a rtp session with the socket.
		void outSocketReady();

	private:
		class Private;
		Private *d;
	};
}

#endif
