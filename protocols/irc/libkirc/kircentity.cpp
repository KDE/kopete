/*
    kircentity.cpp - IRC Client

    Copyright (c) 2004      by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2004      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/


#include <kdebug.h>

#include "kircentity.h"

const QRegExp KIRCEntity::sm_userRegExp(QString::fromLatin1("^([^\\s,:!@]+)(?:(?:!([^\\s,:!@]+))?(?:@([^\\s,!@]+)))?$"));
const QRegExp KIRCEntity::sm_channelRegExp( QString::fromLatin1("^[#!+&][^\\s,:]+$") );

QString KIRCEntity::userInfo(const QString &s, int num)
{
	QRegExp userRegExp(sm_userRegExp);
	userRegExp.search(s);
	return userRegExp.cap(num);
}
#if KDE_IS_VERSION( 3, 2, 90 )
KResolverResults KIRCEntity::resolve(bool *success)
{
	resolveAsync();

	KResolver *resolver = getResolver();
	resolver->wait();
	if(success) *success = resolver->status() == KResolver::Success;
	return resolver->results();
}

void KIRCEntity::resolveAsync()
{
	KResolver *resolver = getResolver();
	switch(resolver->status())
	{
	case KResolver::Idle:
//	case QResolver::Canceled:
//	case QResolver::Failed:
		resolver->start();
	case KResolver::Success:
		break;
	default:
		kdDebug(14120) << k_funcinfo << "Resolver not started(" << resolver->status() << ")" << endl;
	}
}

KResolver *KIRCEntity::getResolver()
{
	if (!m_resolver)
	{
		m_resolver = new KResolver(userHost(), QString::null, this);
//		m_resolver->setFlags(flags);
//		m_resolver->setFamily(families)
		connect(m_resolver, SIGNAL(finished(KResolverResults)),
			this, SIGNAL(resolverResults(KResolverResults)));
	}

	return m_resolver;
}
#endif

#include "kircentity.moc"

