/*
    kircmessageredirector.h - IRC Client

    Copyright (c) 2004-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2004-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRC_COMMAND_H
#define KIRC_COMMAND_H

#include "kircmessage.h"

namespace KIrc
{

class Message;

class Command
	: public QObject
{
	Q_OBJECT

//	Q_PROPERTY(int min READ min WRITE setMin)
//	Q_PROPERTY(int max READ max WRITE setMax)
//	Q_PROPERTY(QString help READ help WRITE setHelp)

public:
	static QString expand(QString command, QString args);

public:
	enum {
		Unknown = -1,
		Unlimited = -2
	};
/*
	typedef enum {
		CHECK_CONNECTED = 0x01
	} CheckFlag;

	typdef QFlags<CheckFlag> CheckFlags;
*/
	explicit Command(QObject *parent = 0);
	~Command();

public: // READ properties accessors.
//	int min() const;
//	int max() const;
	QString help() const;

public slots: // WRITE properties accessors.
//	void setMin();
//	void setMax();
	void setHelp(const QString &help);

public slots:
//	void setMinMax(int minMax);
//	void setMinMax(int min, int max);

	/**
	 * Attempt to send the message.
	 */
	virtual void handleMessage(KIrc::Message msg);

signals:
	void redirect(KIrc::Message);

protected:
	/**
	 * Check that the given message can be send.
	 * @return true if the message can be send.
	 */
	bool checkValidity(const Message &msg);

private:
	Q_DISABLE_COPY(Command)

	class Private;
	Private * const d;
};

}

#endif
