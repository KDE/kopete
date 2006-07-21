/*
   kcodecaction.h

    Copyright (c) 2003      by Jason Keirstead        <jason@keirstead.org>
    Copyright (c) 2003-2005 by Michel Hermier <michel.hermier@wanadoo.fr>
    Kopete    (c) 2003-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef KCODECACTION_H
#define KCODECACTION_H

#include <kaction.h>

class KCodecAction
	: public KSelectAction
{
	Q_OBJECT

public:
	KCodecAction(const QString &text, const KShortcut &cut = KShortcut(),
		QObject *parent = 0, const char *name = 0);

	void setCodec(QTextCodec *codec);

signals:
	void activated(QTextCodec *);

private slots:
	void slotActivated(int);

private:
	QMap<quint32, QTextCodec *> codecMap;
};

#endif
