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

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <qjson/parser.h>
#else
#include <QJsonDocument>
#include <QUrlQuery>
#endif

#include "xoauth2provider.h"

class XOAuth2SASLContext : public QCA::SASLContext
{
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
    XOAuth2SASLContext(QCA::Provider *p) : QCA::SASLContext(p)
    {
        manager = new QNetworkAccessManager(this);
        reset();
    }

    virtual ~XOAuth2SASLContext()
    {
        reset();
    }

    QCA::Provider::Context *clone() const Q_DECL_OVERRIDE
    {
        XOAuth2SASLContext *s = new XOAuth2SASLContext(provider());
        s->user = user;
        s->clientId = clientId;
        s->clientSecretKey = clientSecretKey;
        s->refreshToken = refreshToken;
        s->accessToken = accessToken;
        s->requestUrl = requestUrl;
        return s;
    }

    void reset() Q_DECL_OVERRIDE
    {
        user.clear();
        clientId.clear();
        clientSecretKey.clear();
        refreshToken.clear();
        accessToken.clear();
        requestUrl.clear();
        data.clear();
        authCondition_ = QCA::SASL::AuthFail;
    }

    void setup(const QString &, const QString &, const QCA::SASLContext::HostPort *, const QCA::SASLContext::HostPort *, const QString &, int) Q_DECL_OVERRIDE
    {
    }

    void setConstraints(QCA::SASL::AuthFlags, int, int) Q_DECL_OVERRIDE
    {
    }

    void startClient(const QStringList &mechlist, bool) Q_DECL_OVERRIDE
    {
        if (!mechlist.contains(QStringLiteral("X-OAUTH2"))) {
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

    void startServer(const QString &, bool) Q_DECL_OVERRIDE
    {
        result_ = QCA::SASLContext::Error;
        QMetaObject::invokeMethod(this, "resultsReady", Qt::QueuedConnection);
    }

    void serverFirstStep(const QString &, const QByteArray *) Q_DECL_OVERRIDE
    {
        result_ = QCA::SASLContext::Error;
        QMetaObject::invokeMethod(this, "resultsReady", Qt::QueuedConnection);
    }

    void nextStep(const QByteArray &) Q_DECL_OVERRIDE
    {
        tryAgain();
    }

    void tryAgain() Q_DECL_OVERRIDE
    {
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

    void update(const QByteArray &from_net, const QByteArray &from_app) Q_DECL_OVERRIDE
    {
        result_to_app = from_net;
        result_to_net = from_app;
        result_ = QCA::SASLContext::Success;
        QMetaObject::invokeMethod(this, "resultsReady", Qt::QueuedConnection);
    }

    bool waitForResultsReady(int) Q_DECL_OVERRIDE
    {
        return true;
    }

    QCA::SASLContext::Result result() const Q_DECL_OVERRIDE
    {
        return result_;
    }

    QStringList mechlist() const Q_DECL_OVERRIDE
    {
        return QStringList();
    }

    QString mech() const Q_DECL_OVERRIDE
    {
        return QStringLiteral("X-OAUTH2");
    }

    bool haveClientInit() const Q_DECL_OVERRIDE
    {
        return false;
    }

    QByteArray stepData() const Q_DECL_OVERRIDE
    {
        return data;
    }

    QByteArray to_net() Q_DECL_OVERRIDE
    {
        return result_to_net;
    }

    int encoded() const Q_DECL_OVERRIDE
    {
        return result_to_net.size();
    }

    QByteArray to_app() Q_DECL_OVERRIDE
    {
        return result_to_app;
    }

    int ssf() const Q_DECL_OVERRIDE
    {
        return 0;
    }

    QCA::SASL::AuthCondition authCondition() const Q_DECL_OVERRIDE
    {
        return authCondition_;
    }

    QCA::SASL::Params clientParams() const Q_DECL_OVERRIDE
    {
        bool needUser = user.isEmpty();
        bool needPass = (accessToken.isEmpty() && (clientId.isEmpty() || clientSecretKey.isEmpty() || requestUrl.isEmpty() || refreshToken.isEmpty()));
        return QCA::SASL::Params(needUser, false, needPass, false);
    }

    void setClientParams(const QString *userParam, const QString *, const QCA::SecureArray *passParam, const QString *) Q_DECL_OVERRIDE
    {
        if (userParam) {
            user = *userParam;
        }
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
                if (params.size() == 1) {
                    accessToken = params.at(0);
                } else {
                    accessToken.clear();
                }
            }
        }
    }

    QStringList realmlist() const Q_DECL_OVERRIDE
    {
        return QStringList();
    }

    QString username() const Q_DECL_OVERRIDE
    {
        return QString();
    }

    QString authzid() const Q_DECL_OVERRIDE
    {
        return QString();
    }

private:
    void requestAccessToken()
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        QUrl query;
#else
        QUrlQuery query;
#endif
        query.addQueryItem(QStringLiteral("client_id"), clientId);
        query.addQueryItem(QStringLiteral("client_secret"), QString::fromUtf8(clientSecretKey.toByteArray()));
        query.addQueryItem(QStringLiteral("refresh_token"), QString::fromUtf8(refreshToken.toByteArray()));
        query.addQueryItem(QStringLiteral("grant_type"), QStringLiteral("refresh_token"));
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        const QByteArray &data = query.encodedQuery();
#else
        const QByteArray &data = query.toString(QUrl::FullyEncoded).toUtf8();
#endif
        QNetworkRequest request(requestUrl);
        request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
        QNetworkReply *reply = manager->post(request, data);
        connect(reply, SIGNAL(finished()), this, SLOT(accessTokenReceived()));
    }

    void sendAuth()
    {
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
    void accessTokenReceived()
    {
        QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
        const QByteArray &replyData = reply->readAll();
        reply->deleteLater();
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
        QJson::Parser parser;
        bool ok = false;
        const QVariant &replyVariant = parser.parse(replyData, &ok);
#else
        QJsonParseError error;
        const QJsonDocument &replyJson = QJsonDocument::fromJson(replyData, &error);
        bool ok = (error.error() == QJsonParseError::NoError);
        const QVariant &replyVariant = replyJson.toVariant();
#endif
        if (ok && replyVariant.type() == QVariant::Map) {
            const QVariantMap &replyMap = replyVariant.toMap();
            if (replyMap.contains(QStringLiteral("access_token"))) {
                const QVariant &accessTokenVariant = replyMap.value(QStringLiteral("access_token"));
                if (accessTokenVariant.type() == QVariant::String) {
                    accessToken = accessTokenVariant.toString().toUtf8();
                } else {
                    ok = false;
                }
            } else {
                ok = false;
            }
            if (!ok || replyMap.contains(QStringLiteral("error"))) {
                const QString &error = replyMap.value(QStringLiteral("error")).toString();
                const QString &errorDescription = replyMap.value(QStringLiteral("error_description")).toString();
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

class QCAXOAuth2SASL : public QCA::Provider
{
public:
    QCAXOAuth2SASL()
    {
    }

    virtual ~QCAXOAuth2SASL()
    {
    }

    void init() Q_DECL_OVERRIDE
    {
    }

    int qcaVersion() const Q_DECL_OVERRIDE
    {
        return QCA_VERSION;
    }

    QString name() const Q_DECL_OVERRIDE
    {
        return QStringLiteral("xoauth2sasl");
    }

    QStringList features() const Q_DECL_OVERRIDE
    {
        return QStringList(QStringLiteral("sasl"));
    }

    QCA::Provider::Context *createContext(const QString &type) Q_DECL_OVERRIDE
    {
        if (type == QLatin1String("sasl")) {
            return new XOAuth2SASLContext(this);
        }
        return NULL;
    }
};

QCA::Provider *createProviderXOAuth2()
{
    return new QCAXOAuth2SASL();
}

#include "xoauth2provider.moc"
