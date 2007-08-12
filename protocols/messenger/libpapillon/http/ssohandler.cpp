/*
   ssohandler.cpp - Windows Live Messenger SSO authentication

   Copyright (c) 2007 		by Zhang PanYong <pyzhang@gmail.com>
   Kopete    (c) 2002-2005	by the Kopete developers  <kopete-devel@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/

#include "Papillon/Http/SsoHandler"
// Qt includes
#include <QtDebug>
#include <QtCrypto>

// Papillon includes
#include "Papillon/Network/IpEndpointConnector"

namespace Papillon 
{
class SSOHandler::Private
{
public:
	Private()
	 : connection(0), success(false), connectionTry(0)
	{}

	~Private()
	{
		delete connection;
	}

	QString ssoMethod;
	QString passportId;
	QString password;

	HttpConnection *connection;
	bool success;
	int connectionTry;
};

SSOHandler::SSOHandler(HttpConnection *connection, QObject *parent)
 : QObject(0), d(new Private)
{
	d->connection = connection;
	connect(d->connection, SIGNAL(readyRead()), this, SLOT(slotConnectionReadyRead()));
}

SSOHandler::~SSOHandler()
{
	delete d;
}

void SSOHandler::setLoginInformation(const QString &ssoMethod, const QString &passportId, const QString &password)
{
	d->ssoMethod = ssoMethod;
	d->passportId = passportId;
	d->password = password;
}

void SSOHandler::connectToServer(const QString &server)
{
	QEventLoop tempEventLoop;
	connect(d->connection, SIGNAL(connected()), &tempEventLoop, SLOT(quit()));
	// FIXME: React on a connecting error.
	d->connection->connectToServer(server);

	tempEventLoop.exec();
}

void SSOHandler::start()
{
	qDebug() << Q_FUNC_INFO << "Begin sso ticket negotiation.";
	Q_ASSERT( !d->ssoMethod.isEmpty() );
	Q_ASSERT( !d->passportId.isEmpty() );
	Q_ASSERT( !d->password.isEmpty() );

	d->state = SSOGetServer;
	QString server = QLatin1String("login.live.com");

	// Connect to server
	connectToServer(server);

	HttpTransfer *transfer = new HttpTransfer;
	transfer->setRequest( QLatin1String("POST"), QLatin1String("/RST.srf") );
	transfer->setValue( QLatin1String("Host"), server );
	transfer->setValue( QLatin1String("User-Agent"), QLatin1String("libpapillon") );

	// TODO: Use Papillon Soap classes instead.
	QByteArray soapData = QString::fromUtf8("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
		"<Envelope xmlns=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:wsse=\"http://schemas.xmlsoap.org/ws/2003/06/secext\" xmlns:saml=\"urn:oasis:names:tc:SAML:1.0:assertion\" xmlns:wsp=\"http://schemas.xmlsoap.org/ws/2002/12/policy\" xmlns:wsu=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\" xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/03/addressing\" xmlns:wssc=\"http://schemas.xmlsoap.org/ws/2004/04/sc\" xmlns:wst=\"http://schemas.xmlsoap.org/ws/2004/04/trust\">"
		"<Header>"
		"<ps:AuthInfo xmlns:ps=\"http://schemas.microsoft.com/Passport/SoapServices/PPCRL\" Id=\"PPAuthInfo\">"
		"<ps:HostingApp>"
		"{7108E71A-9926-4FCB-BCC9-9A9D3F32E423}"
		"</ps:HostingApp>"
		"<ps:BinaryVersion>4</ps:BinaryVersion>"
		"<ps:UIVersion>1</ps:UIVersion>"
		"<ps:Cookies></ps:Cookies>"
		"<ps:RequestParams>AQAAAAIAAABsYwQAAAAyMDUy</ps:RequestParams>"
		"</ps:AuthInfo><wsse:Security>"
		"<wsse:UsernameToken Id=\"user\">"
		"<wsse:Username>%1</wsse:Username>"
		"<wsse:Password>%2</wsse:Password>"
		"</wsse:UsernameToken>"
		"</wsse:Security>"
		"</Header>"
		"<Body>"
		"<ps:RequestMultipleSecurityTokens xmlns:ps=\"http://schemas.microsoft.com/Passport/SoapServices/PPCRL\" Id=\"RSTS\">"
		"<wst:RequestSecurityToken Id=\"RST0\">"
		"<wst:RequestType>http://schemas.xmlsoap.org/ws/2004/04/security/trust/Issue</wst:RequestType>"
		"<wsp:AppliesTo>"
		"<wsa:EndpointReference>"
		"<wsa:Address>http://Passport.NET/tb</wsa:Address>"
		"</wsa:EndpointReference>"
		"</wsp:AppliesTo>"
		"</wst:RequestSecurityToken>"
		"<wst:RequestSecurityToken Id=\"RST1\">"
		"<wst:RequestType>http://schemas.xmlsoap.org/ws/2004/04/security/trust/Issue</wst:RequestType>"
		"<wsp:AppliesTo>"
		"<wsa:EndpointReference>"
		"<wsa:Address>messengerclear.live.com</wsa:Address>"
		"</wsa:EndpointReference>"
		"</wsp:AppliesTo>"
		"<wsse:PolicyReference URI=\"%3\">"
		"</wsse:PolicyReference>"
		"</wst:RequestSecurityToken>"
		"<wst:RequestSecurityToken Id=\"RST2\">"
		"<wst:RequestType>http://schemas.xmlsoap.org/ws/2004/04/security/trust/Issue</wst:RequestType>"
		"<wsp:AppliesTo>"
		"<wsa:EndpointReference>"
		"<wsa:Address>messenger.msn.com</wsa:Address>"
		"</wsa:EndpointReference>"
		"</wsp:AppliesTo>"
		"<wsse:PolicyReference URI=\"?id=507\">"
		"</wsse:PolicyReference></wst:RequestSecurityToken>"
		"<wst:RequestSecurityToken Id=\"RST3\">"
		"<wst:RequestType>http://schemas.xmlsoap.org/ws/2004/04/security/trust/Issue</wst:RequestType>"
		"<wsp:AppliesTo><wsa:EndpointReference>"
		"<wsa:Address>contacts.msn.com</wsa:Address>"
		"</wsa:EndpointReference>"
		"</wsp:AppliesTo>"
		"<wsse:PolicyReference URI=\"MBI\">"
		"</wsse:PolicyReference>"
		"</wst:RequestSecurityToken>"
		"<wst:RequestSecurityToken Id=\"RST4\">"
		"<wst:RequestType>http://schemas.xmlsoap.org/ws/2004/04/security/trust/Issue</wst:RequestType>"
		"<wsp:AppliesTo>"
		"<wsa:EndpointReference>"
		"<wsa:Address>messengersecure.live.com</wsa:Address>"
		"</wsa:EndpointReference>"
		"</wsp:AppliesTo>"
		"<wsse:PolicyReference URI=\"MBI_SSL\"></wsse:PolicyReference>"
		"</wst:RequestSecurityToken>"
		"<wst:RequestSecurityToken Id=\"RST5\">"
		"<wst:RequestType>http://schemas.xmlsoap.org/ws/2004/04/security/trust/Issue</wst:RequestType>"
		"<wsp:AppliesTo>"
		"<wsa:EndpointReference>"
		"<wsa:Address>spaces.live.com</wsa:Address>"
		"</wsa:EndpointReference>"
		"</wsp:AppliesTo>"
		"<wsse:PolicyReference URI=\"MBI\">"
		"</wsse:PolicyReference>"
		"</wst:RequestSecurityToken>"
		"<wst:RequestSecurityToken Id=\"RST6\">"
		"<wst:RequestType>http://schemas.xmlsoap.org/ws/2004/04/security/trust/Issue</wst:RequestType>"
		"<wsp:AppliesTo>"
		"<wsa:EndpointReference>"
		"<wsa:Address>storage.msn.com</wsa:Address>"
		"</wsa:EndpointReference>"
		"</wsp:AppliesTo>"
		"<wsse:PolicyReference URI=\"MBI\">"
		"</wsse:PolicyReference>"
		"</wst:RequestSecurityToken>"
		"</ps:RequestMultipleSecurityTokens>"
		"</Body>"
		"</Envelope>").arg(d->passportId).arg(d->password),arg(d->ssoMethod).toUtf8();

	transfer->setBody( soapData );

	d->connection->write(transfer);
}

void SSOHandler::parseToken(QDomElement & tokenElement)
{
	QDomNode tNode = tokenElement.firstChild();
	while(!tNode.isNull())
	{
		QDomElement tElement = tNode.toElement();
//		qDebug()<< "tElement"<< tElement.tagName();
		if(tElement.tagName() == QLatin1String("RequestedSecurityToken"))
		{
			QDomNode binaryNode = tElement.firstChild();
			while(!binaryNode.isNull())
			{
				QDomElement binaryElement = binaryNode.toElement();
				if(binaryElement.tagName() == QLatin1String("BinarySecurityToken"))
				{
					QString idKey = binaryElement.attribute("Id");
					QString data = binaryElement.text();
					d->Tokens.insert(idKey, data);
					qDebug()<< "Id" << idKey;
					qDebug()<< "binaryToken"<<data;
				}
				else if(binaryElement.tagName() == QLatin1String("EncryptedData"))
				{
					QDomNode ciphNode = binaryElement.firstChild();
					while(!ciphNode.isNull())
					{
						QDomElement ciphElement = ciphNode.toElement();
//						qDebug()<< "ciphData" << ciphElement.tagName();
						if(ciphElement.tagName() == QLatin1String("CipherData"))
						{
							QDomElement ciphValueElement = ciphElement.firstChild().toElement();
							qDebug()<<"CiphValue"<< ciphValueElement.text();
						}
						ciphNode = ciphNode.nextSibling();
					}
				}
				binaryNode = binaryNode.nextSibling();
			}
		}
		else if(tElement.tagName() == QLatin1String("AppliesTo"))
		{
			QDomElement endpointElement = tElement.firstChild().toElement();
			QDomElement addrElement = endpointElement.firstChild().toElement();
			if(addrElement.tagName() ==	QLatin1String("Address"))
			{
				qDebug() << "Address"<< addrElement.text();
			}
		}
		else if(tElement.tagName() == QLatin1String("LifeTime"))
		{
			QDomNode timeNode = tElement.firstChild();
			while(!timeNode.isNull())
			{
				QDomElement timeElement = timeNode.toElement();
				if(timeElement.tagName() ==	QLatin1String("Created"))
				{
					qDebug() << "Created"<< timeElement.text();
				}
				if(timeElement.tagName() ==	QLatin1String("Expires"))
				{
					qDebug() << "Expires"<< timeElement.text();
				}
				timeNode = timeNode.nextSibling();
			}

		}
		tNode = tNode.nextSibling();
	}
}

void SSOHandler::parseTokens(QDomElement & tokensElement)
{
	QDomNode tokenNode = tokensElement.firstChild();

	while(!tokenNode.isNull())
	{
		QDomElement tokenElement = tokenNode.toElement();
//		qDebug() << "tokenElement"<< tokenElement.tagName();
		parseToken(tokenElement);
		tokenNode = tokenNode.nextSibling();
	}
}

void SSOHandler::slotConnectionReadyRead()
{
	HttpTransfer *transfer = d->connection->read();
	if( !transfer )
	{
		qDebug() << Q_FUNC_INFO <<"No HTTP Transfer received!";
		return;
	}

	// We no longer need the HTTP connection, close it.
	d->connection->disconnectFromServer();

	QByteArray data = transfer->body();

	QDomDocument responseDocument;
	responseDocument.setContent(data, true);
	QDomNode node = responseDocument.documentElement().firstChild();

	while( !node.isNull() )
	{
		QDomElement bodyElement = node.toElement();
		if( !bodyElement.isNull() )
		{
			if( bodyElement.tagName() == QLatin1String("Body") )
			{
				QDomElement tokensElement = bodyElement.firstChild().toElement();
				if( (!tokensElement.isNull()) &&
						(tokensElement.tagName() == QLatin1String("RequestSecurityTokenResponseCollection")) )
				{
					parseTokens(tokensElement);
				}
			}
		}

		node = node.nextSibling();
	}
	emitResult(true);
}

void SSOHandler::emitResult(bool success)
{
	d->stream->disconnectFromServer();

	d->success = success;
	emit result(this);
}

/*derive keys*/
QByteArray SSOHandler::derive_key(QByteArray sso_key, QString sso_magic)
{
	QCA::Base64 encoder;
	QCA::SecureArray key(sso_key);

	qDebug() << "=====================================";
	qDebug() << "entering derive_key";
	qDebug() << "key: " << QString(encoder.encode(sso_key).toByteArray());
	qDebug() << "magic: "<< encoder.encodeString(sso_magic);

	if( !QCA::isSupported("hmac(sha1)") ) {
		qDebug() << "HMAC(SHA1) not supported!";
		return NULL;
	}

	/*hash1*/
	QCA::MessageAuthenticationCode hmacObject( "hmac(sha1)", QCA::SecureArray() );
	QCA::SymmetricKey keyObject(key);
	hmacObject.setup(key);
	QCA::SecureArray magic(sso_magic.toLatin1()); 
	hmacObject.update(magic);
	QCA::SecureArray resultArray = hmacObject.final();
	QByteArray hash1 = resultArray.toByteArray();
	qDebug() <<"hash1: "<< QString(encoder.encode(hash1).toByteArray());

	/*hash2*/
	hmacObject.clear();
	hmacObject.setup(key);
	QByteArray hash2_data = hash1 + sso_magic.toLatin1();
	QCA::SecureArray magic2(hash2_data); 
	hmacObject.update(magic2);
	resultArray = hmacObject.final();
	QByteArray hash2 = resultArray.toByteArray();
	qDebug() <<"hash2: "<< QString(encoder.encode(hash2).toByteArray());

	/*hash3*/
	hmacObject.clear();
	hmacObject.setup(key);
	QCA::SecureArray magic3(hash1); 
	hmacObject.update(magic3);
	resultArray = hmacObject.final();
	QByteArray hash3 = resultArray.toByteArray();
	qDebug() <<"hash3: "<< QString(encoder.encode(hash3).toByteArray());

	/*hash4*/
	hmacObject.clear();
	hmacObject.setup(key);
	QCA::SecureArray magic4(hash3 + sso_magic.toLatin1()); 
	hmacObject.update(magic4);
	resultArray = hmacObject.final();
	QByteArray hash4 = resultArray.toByteArray();
	qDebug() <<"hash4: "<< QString(encoder.encode(hash4).toByteArray());

	QByteArray result = hash2+ hash4.left(4);

	qDebug() << "finish derive_key...";
	qDebug() << "=====================================";
	return result;
}

QByteArray SSOHandler::getIVData()
{
	QByteArray ivArray;
#if 1
	for(int i=0;i<8; i++){
		ivArray += (qint8)qrand();
	}
#else
	ivArray = base64Object.decode(QString("hVxKua5sagM=").toLatin1()).toByteArray();
#endif
	return ivArray;
}

/*mbi_encrypt */
QString SSOHandler::mbi_encypt(QString key, QString nonce)
{
	QCA::Base64 base64Object;
	MessengerUserKey_t  msg_user_key;

	/*setup const key structure*/
	msg_user_key.uStructHeaderSize = 28;
	msg_user_key.uCryptMode 	= 1;
	msg_user_key.uCipherType	= 26115;
	msg_user_key.uHashType		= 32772;
	msg_user_key.uIVLen			= sizeof (msg_user_key.aIVBytes);
	msg_user_key.uHashLen		= sizeof (msg_user_key.aHashBytes);
	msg_user_key.uCipherLen 	= sizeof (msg_user_key.aCipherBytes);

	qDebug() << ("entering mbi_encypt...");
	qDebug() << "key:"<<key;
	qDebug() << "nonce:" << nonce;

	QByteArray key1 = base64Object.decode(key.toLatin1()).toByteArray();

	QByteArray key2 = derive_key(key1, 
			"WS-SecureConversationSESSION KEY HASH");
	qDebug() << "key2:" << QString(base64Object.encode(key2).toByteArray());
	QByteArray key3 = derive_key(key1, 
			"WS-SecureConversationSESSION KEY ENCRYPTION");
	qDebug() << "key3:" << QString(base64Object.encode(key3).toByteArray());

	/*compute hash*/
	if( !QCA::isSupported("hmac(sha1)") ) {
		printf("HMAC(SHA1) not supported!\n");
		return NULL;
	}
	QCA::MessageAuthenticationCode hmacObject("hmac(sha1)", QCA::SecureArray() );
	QCA::SymmetricKey keyObject(key2);
	hmacObject.setup(key2);
	QCA::SecureArray magic(nonce.toLatin1());
	hmacObject.update(magic);
	QCA::SecureArray resultArray = hmacObject.final();
	QByteArray hashArray = resultArray.toByteArray();
	qDebug() << "hash:" << QString(base64Object.encode(hashArray).toByteArray());

	memcpy(&msg_user_key.aHashBytes, hashArray.data(), msg_user_key.uHashLen);
	QString hash = QCA::arrayToHex(resultArray.toByteArray());

	/*Windows always add the padding \x08*/
	nonce = nonce + "\x08\x08\x08\x08\x08\x08\x08\x08";

	if(!QCA::isSupported("tripledes-cbc")){
		qDebug()<< "tripledes-cbc not supported!";
		return NULL;
	}
	const QString provider("qca-ossl");
	const QString method("tripledes");
	QCA::SymmetricKey key3Object(key3);

	/*get a random IV key*/
	QByteArray ivArray = getIVData();
	memcpy(&msg_user_key.aIVBytes, ivArray.data(), msg_user_key.uIVLen);
	qDebug() << "iv:" << QString(base64Object.encode(ivArray).toByteArray());
	QCA::InitializationVector iv( ivArray );

	/*ciph*/
	QCA::Cipher desCipher(method,
			QCA::Cipher::CBC,
			QCA::Cipher::NoPadding,
			QCA::Encode,
			key3Object,
			iv,
			provider);
	QByteArray ciphArray = desCipher.update(nonce.toLatin1()).toByteArray();
	memcpy(&msg_user_key.aCipherBytes, ciphArray.data(), msg_user_key.uCipherLen);
	qDebug() << "ciph:" << QString(base64Object.encode(ciphArray).toByteArray());

	QByteArray blob_array = QByteArray::fromRawData((char *)&msg_user_key, 
														sizeof(msg_user_key));
	QString blob_base64 = QString(base64Object.encode(blob_array).toByteArray());
	return blob_base64;
}

QString SSOHandler::getToken(QString key)
{
	return d->Tokens.value(key);
}

QString SSOHandler::ticket()
{
	return getToken("PPToken2");
}

}
#include "ssohandler.moc"
