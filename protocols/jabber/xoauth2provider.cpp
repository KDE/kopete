/*
    xoauth2provider.cpp - X-OAuth2 provider for QCA

    Copyright (c) 2016 by Pali Rohár <pali.rohar@gmail.com>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <qca.h>

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
#include <qjson/parser.h>
#else
#include <QJsonDocument>
#include <QUrlQuery>
#endif

#include "xoauth2provider.h"

class XOAuth2SASLContext : public QCA::SASLContext {
	Q_OBJECT

private:
	QString user;
	QString clientId;
	QString requestUrl;
	QCA::SecureArray clientSecretKey;
	QCA::SecureArray refreshToken;
	QCA::SecureArray accessToken;

	QByteArray data;
	QByteArray result_to_net;
	QByteArray result_to_app;
	QCA::SASLContext::Result result_;
	QCA::SASL::AuthCondition authCondition_;

	QNetworkAccessManager *manager;

public:
	XOAuth2SASLContext(QCA::Provider *p) : QCA::SASLContext(p) {
		manager = new QNetworkAccessManager(this);
		reset();
	}

	virtual ~XOAuth2SASLContext() {
		reset();
	}

	virtual QCA::Provider::Context *clone() const {
		XOAuth2SASLContext *s = new XOAuth2SASLContext(provider());
		s->user = user;
		s->clientId = clientId;
		s->clientSecretKey = clientSecretKey;
		s->refreshToken = refreshToken;
		s->accessToken = accessToken;
		s->requestUrl = requestUrl;
		return s;
	}

	virtual void reset() {
		user.clear();
		clientId.clear();
		clientSecretKey.clear();
		refreshToken.clear();
		accessToken.clear();
		requestUrl.clear();
		data.clear();
		authCondition_ = QCA::SASL::AuthFail;
	}

	virtual void setup(const QString &, const QString &, const QCA::SASLContext::HostPort *, const QCA::SASLContext::HostPort *, const QString &, int) {
	}

	virtual void setConstraints(QCA::SASL::AuthFlags, int, int) {
	}

	virtual void startClient(const QStringList &mechlist, bool) {
		if (!mechlist.contains(QLatin1String("X-OAUTH2"))) {
			qWarning("No X-OAUTH2 auth method");
			authCondition_ = QCA::SASL::NoMechanism;
			QMetaObject::invokeMethod(this, "resultsReady", Qt::QueuedConnection);
			return;
		}
		authCondition_ = QCA::SASL::AuthFail;
		result_ = QCA::SASLContext::Continue;
		data.clear();
		tryAgain();
	}

	virtual void startServer(const QString &, bool) {
		result_ = QCA::SASLContext::Error;
		QMetaObject::invokeMethod(this, "resultsReady", Qt::QueuedConnection);
	}

	virtual void serverFirstStep(const QString &, const QByteArray *) {
		result_ = QCA::SASLContext::Error;
		QMetaObject::invokeMethod(this, "resultsReady", Qt::QueuedConnection);
	}

	virtual void nextStep(const QByteArray &) {
		tryAgain();
	}

	virtual void tryAgain() {
		if (user.isEmpty() || (accessToken.isEmpty() && (clientId.isEmpty() || clientSecretKey.isEmpty() || requestUrl.isEmpty() || refreshToken.isEmpty()))) {
			result_ = QCA::SASLContext::Params;
			QMetaObject::invokeMethod(this, "resultsReady", Qt::QueuedConnection);
			return;
		}
		if (accessToken.isEmpty()) {
			requestAccessToken();
			return;
		}
		sendAuth();
	}

	virtual void update(const QByteArray &from_net, const QByteArray &from_app) {
		result_to_app = from_net;
		result_to_net = from_app;
		result_ = QCA::SASLContext::Success;
		QMetaObject::invokeMethod(this, "resultsReady", Qt::QueuedConnection);
	}

	virtual bool waitForResultsReady(int) {
		return true;
	}

	virtual QCA::SASLContext::Result result() const {
		return result_;
	}

	virtual QStringList mechlist() const {
		return QStringList();
	}

	virtual QString mech() const {
		return QLatin1String("X-OAUTH2");
	}

	virtual bool haveClientInit() const {
		return false;
	}

	virtual QByteArray stepData() const {
		return data;
	}

	virtual QByteArray to_net() {
		return result_to_net;
	}

	virtual int encoded() const {
		return result_to_net.size();
	}

	virtual QByteArray to_app() {
		return result_to_app;
	}

	virtual int ssf() const {
		return 0;
	}

	virtual QCA::SASL::AuthCondition authCondition() const {
		return authCondition_;
	}

	virtual QCA::SASL::Params clientParams() const {
		bool needUser = user.isEmpty();
		bool needPass = (accessToken.isEmpty() && (clientId.isEmpty() || clientSecretKey.isEmpty() || requestUrl.isEmpty() || refreshToken.isEmpty()));
		return QCA::SASL::Params(needUser, false, needPass, false);
	}

	virtual void setClientParams(const QString *userParam, const QString *, const QCA::SecureArray *passParam, const QString *) {
		if (userParam)
			user = *userParam;
		if (passParam) {
			const QList<QByteArray> &params = passParam->toByteArray().split(0x7F);
			if (params.size() == 5) {
				clientId = QString::fromUtf8(params.at(0));
				clientSecretKey = params.at(1);
				refreshToken = params.at(2);
				accessToken = params.at(3);
				requestUrl = QString::fromUtf8(params.at(4));
			} else {
				clientId.clear();
				clientSecretKey.clear();
				refreshToken.clear();
				requestUrl.clear();
				if (params.size() == 1)
					accessToken = params.at(0);
				else
					accessToken.clear();
			}
		}
	}

	virtual QStringList realmlist() const {
		return QStringList();
	}

	virtual QString username() const {
		return QString();
	}

	virtual QString authzid() const {
		return QString();
	}

private:
	void requestAccessToken() {
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
		QUrl query;
#else
		QUrlQuery query;
#endif
		query.addQueryItem(QLatin1String("client_id"), clientId);
		query.addQueryItem(QLatin1String("client_secret"), QString::fromUtf8(clientSecretKey.toByteArray()));
		query.addQueryItem(QLatin1String("refresh_token"), QString::fromUtf8(refreshToken.toByteArray()));
		query.addQueryItem(QLatin1String("grant_type"), QLatin1String("refresh_token"));
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
		const QByteArray &data = query.encodedQuery();
#else
		const QByteArray &data = query.toString(QUrl::FullyEncoded).toUtf8();
#endif
		QNetworkRequest request(requestUrl);
		request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
		QNetworkReply *reply = manager->post(request, data);
		connect(reply, SIGNAL(finished()), this, SLOT(accessTokenReceived()));
	}

	void sendAuth() {
		if (accessToken.isEmpty()) {
			authCondition_ = QCA::SASL::AuthFail;
			result_ = QCA::SASLContext::Error;
		} else {
			data.clear();
			data += '\0';
			data += user.toUtf8();
			data += '\0';
			data += accessToken.toByteArray();
			result_ = QCA::SASLContext::Success;
		}
		QMetaObject::invokeMethod(this, "resultsReady", Qt::QueuedConnection);
	}

private slots:
	void accessTokenReceived() {
		QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
		const QByteArray &replyData = reply->readAll();
		reply->deleteLater();
		QJsonParseError error;
		const QJsonDocument &replyJson = QJsonDocument::fromJson(replyData, &error);
        bool ok = (error.error == QJsonParseError::NoError);
		const QVariant &replyVariant = replyJson.toVariant();
		if (ok && replyVariant.type() == QVariant::Map) {
			const QVariantMap &replyMap = replyVariant.toMap();
			if (replyMap.contains(QLatin1String("access_token"))) {
				const QVariant &accessTokenVariant = replyMap.value(QLatin1String("access_token"));
				if (accessTokenVariant.type() == QVariant::String)
					accessToken = accessTokenVariant.toString().toUtf8();
				else
					ok = false;
			} else {
				ok = false;
			}
			if (!ok || replyMap.contains(QLatin1String("error"))) {
				const QString &error = replyMap.value(QLatin1String("error")).toString();
				const QString &errorDescription = replyMap.value(QLatin1String("error_description")).toString();
				qWarning("requestAccessToken failed, error: %s, description: %s", error.toUtf8().data(), errorDescription.toUtf8().data());
				accessToken.clear();
			}
		} else {
			qWarning("requestAccessToken failed, invalid reply: %s", replyData.data());
			accessToken.clear();
		}
		sendAuth();
	}
};

class QCAXOAuth2SASL : public QCA::Provider {
public:
	QCAXOAuth2SASL() {
	}

	virtual ~QCAXOAuth2SASL() {
	}

	virtual void init() {
	}

	virtual int qcaVersion() const {
		return QCA_VERSION;
	}

	virtual QString name() const {
		return QLatin1String("xoauth2sasl");
	}

	virtual QStringList features() const {
		return QStringList(QLatin1String("sasl"));
	}

	QCA::Provider::Context *createContext(const QString& type) {
		if (type == QLatin1String("sasl"))
			return new XOAuth2SASLContext(this);
		return NULL;
	}
};

QCA::Provider *createProviderXOAuth2() {
	return new QCAXOAuth2SASL();
}

#include "xoauth2provider.moc"
