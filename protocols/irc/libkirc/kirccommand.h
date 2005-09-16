/*
    kircmessageredirector.h - IRC Client

    Copyright (c) 2004-2005 by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2004-2005 by the Kopete developers <kopete-devel@kde.org>

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

namespace KIRC
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
	Command(QObject *parent = 0);
	~Command();

public: // READ properties accessors.
//	int min() const;
//	int max() const;
	QString help() const;

public slots: // WRITE properties accessors.
//	Command &setMin();
//	Command &setMax();
	Command &setHelp(const QString &help);

public slots:
//	Command &setMinMax(int minMax);
//	Command &setMinMax(int min, int max);

	/**
	 * Attempt to send the message.
	 */
	virtual void invoke(KIRC::Message msg);

signals:
	void redirect(KIRC::Message);

protected:
	/**
	 * Check that the given message can be send.
	 * @return true if the message can be send.
	 */
	bool checkValidity(const KIRC::Message &msg);

private:
	Q_DISABLE_COPY(Command);

	class Private;
	Private * const d;
};

}

#endif
