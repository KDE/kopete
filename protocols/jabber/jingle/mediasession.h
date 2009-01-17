/*
 * mediasession.h - A helper class to easily manage media data.
 *
 * Copyright (c) 2008 by Detlev Casanova <detlev.casanova@gmail.com>
 *
 * Kopete    (c) by the Kopete developers  <kopete-devel@kde.org>
 *
 * *************************************************************************
 * *                                                                       *
 * * This program is free software; you can redistribute it and/or modify  *
 * * it under the terms of the GNU General Public License as published by  *
 * * the Free Software Foundation; either version 2 of the License, or     *
 * * (at your option) any later version.                                   *
 * *                                                                       *
 * *************************************************************************
 */
#ifndef MEDIA_SESSION_H
#define MEDIA_SESSION_H

#include <QObject>

class MediaManager;
class MediaSession : public QObject
{
	Q_OBJECT
public:
	MediaSession(MediaManager *mediaManager, const QString& codecName);
	~MediaSession();

	void setSamplingRate(int sr);
	void setQuality(int q);
	bool start();
	void write(const QByteArray& sData);
	QByteArray read() const;
	int timeStamp();

public slots:
	void slotReadyRead();
	void slotEncoded();
	void slotDecoded();

signals:
	void readyRead();

private:
	class Private;
	Private * const d;
};

#endif
