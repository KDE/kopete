/*
    kcodecaction.cpp

    Copyright (c) 2003      by Jason Keirstead        <jason@keirstead.org>
    Copyrigth (c) 2006      by Michel Hermier         <michel.hermier@wanadoo.fr>
    Kopete    (c) 2003-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kcodecaction.h"

#include <kcharsets.h>
#include <kdebug.h>
#include <klocale.h>

#include <qtextcodec.h>

class KCodecAction::Private
{
    QAction *defaultAction;
#if 0
    QAction *configureAction;
#endif
};

KCodecAction::KCodecAction( KActionCollection *parent, const QString &name )
    : KSelectAction( parent, name )
    , d(new Private)
{
    init();
}

KCodecAction::KCodecAction( const QString &text, KActionCollection *parent, const QString &name )
    : KSelectAction( text, parent, name )
    , d(new Private)
{
    init();
}

KCodecAction::KCodecAction( const KIcon &icon, const QString &text, KActionCollection *parent, const QString &name )
    : KSelectAction( icon, text, parent, name )
    , d(new Private)
{
    init();
}

KCodecAction::~KCodecAction()
{
    delete d;
}

void KCodecAction::init()
{
#if 0
    d->defaultAction = addAction(i18n("Default"));
#endif
    foreach(QString encodingName, KGlobal::charsets()->descriptiveEncodingNames())
       addAction(encodingName);
#if 0
    addSeparator();
    d->configureAction = addAction(i18n("Configure"));
#endif
    setCurrentItem(0);

//    setEditable(true);
}

void KCodecAction::actionTriggered(QAction *action)
{
    // cache values so we don't need access to members in the action
    // after we've done an emit()
    bool ok = false;
    KCharsets *charsets = KGlobal::charsets(); 
    QString encoding = charsets->encodingForName(action->text());
    QTextCodec *codec = charsets->codecForName(encoding, ok);

    if (ok)
    {
        KSelectAction::actionTriggered(action);

        emit triggered(codec);
    }
    else
    {
        kWarning() << k_funcinfo << "Invalid codec convertion for :\""  << action->text() << '"' << endl;
//        Warn the user in the gui somehow
    }
}

QTextCodec *KCodecAction::currentCodec() const
{
#warning Implement me
    return 0;
}
/*
bool KCodecAction::setCurrentCodec( QTextCodec *codec )
{
    if (codec)
        return setCurrent(codec->name());
    else
        return false;
}
*/
QString KCodecAction::currentCodecName() const
{
    return currentText();
}

bool KCodecAction::setCurrentCodec( const QString &codecName )
{
    return setCurrentAction(codecName, Qt::CaseInsensitive);
}

int KCodecAction::currentCodecMib() const
{
#warning Implement me
    return 0;
}
/*
bool KCodecAction::setCurrentCodec( int mib )
{
    return setCurrentCodec( QTextCodec::codecForMib(mib) );
}
*/
#include "kcodecaction.moc"

