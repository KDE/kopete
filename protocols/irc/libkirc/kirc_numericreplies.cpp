
/*
    kircnumericreplies.cpp - IRC Client

    Copyright (c) 2003      by Michel Hermier <michel.hermier@wanadoo.fr>
    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kirc.h"

bool KIRC::numericReply_001(const KIRCMessage &msg)
{
	/* Gives a welcome message in the form of:
	 * "Welcome to the Internet Relay Network <nick>!<user>@<host>"
	 */
	if (m_FailedNickOnLogin == true)
	{
		// this is if we had a "Nickname in use" message when connecting and we set another nick.
		// This signal emits that the nick was accepted and we are now logged in
		emit successfullyChangedNick(m_Nickname, m_PendingNick);
		m_Nickname = m_PendingNick;
		m_FailedNickOnLogin = false;
	}

	/* At this point we are connected and the server is ready for us to being taking commands
	 * although the MOTD comes *after* this.
	 */
	emit incomingConnectString(msg.suffix());
	setStatus(Connected);
	emit connectedToServer();

	return true;
}

bool KIRC::numericReply_004(const KIRCMessage &msg)
{
	/* Gives information about the servername, version, available modes, etc in the form of:
	 * "<servername> <version> <available user modes> <available channel modes>"
	 */
	emit incomingHostInfo(msg.args()[1],msg.args()[2],msg.args()[3],msg.args()[4]);
	return true;
}

bool KIRC::numericReply_252(const KIRCMessage &msg)
{
	emit incomingConnectString( msg.toString() );
	return true;
}

bool KIRC::numericReply_253(const KIRCMessage &msg)
{
	emit incomingConnectString( msg.toString() );
	return true;
}

bool KIRC::numericReply_254(const KIRCMessage &msg)
{
	emit incomingConnectString( msg.toString() );
	return true;
}

bool KIRC::numericReply_265(const KIRCMessage &)
{
	return true;
}

bool KIRC::numericReply_266(const KIRCMessage &)
{
	return true;
}

bool KIRC::numericReply_303(const KIRCMessage &msg)
{
	/* ":*1<nick> *(" " <nick> )"
	 */
	QStringList nicks = QStringList::split(QRegExp(QChar(' ')), msg.suffix());
	for(QStringList::Iterator it = nicks.begin(); it != nicks.end(); ++it)
	{
		if (!(*it).stripWhiteSpace().isEmpty())
			emit incomingUserOnline(*it);
	}
	return true;
}
/*
bool KIRC::numericReply_305(const KIRCMessage &msg)
{
	return true;
}

bool KIRC::numericReply_306(const KIRCMessage &msg)
{
	return true;
}
*/
bool KIRC::numericReply_311(const KIRCMessage &msg)
{
	emit incomingWhoIsUser(msg.args()[1], msg.args()[2], msg.args()[3], msg.suffix());
	return true;
}

bool KIRC::numericReply_312(const KIRCMessage &msg)
{
	emit incomingWhoIsServer(msg.args()[1], msg.args()[2], msg.suffix());
	return true;
}

bool KIRC::numericReply_314(const KIRCMessage &msg)
{
	emit incomingWhoWasUser( msg.args()[1], msg.args()[2], msg.args()[3], msg.suffix() );
	return true;
}

bool KIRC::numericReply_317(const KIRCMessage &msg)
{
	/* RFC say: "<nick> <integer> :seconds idle"
	 * Some servers say: "<nick> <integer> <integer> :seconds idle, signon time"
	 * Show info about someone who is idle (part of a /whois) in the form of:
	 */
	emit incomingWhoIsIdle(msg.args()[1], msg.args()[2].toULong());
	if (msg.args().size()==4)
		emit incomingSignOnTime(msg.args()[1],msg.args()[3].toULong());
	return true;
}

bool KIRC::numericReply_319(const KIRCMessage &msg)
{
	/* Show info a channel a user is logged in (part of a /whois) in the form of:
	 * "<nick> :{[@|+]<channel><space>}"
	 */
	emit incomingWhoIsChannels(msg.args()[1], msg.suffix());
	return true;
}

bool KIRC::numericReply_322(const KIRCMessage &msg)
{
	/* "<channel> <# visible> :<topic>"
	 */
	//kdDebug(14120) << k_funcinfo << "Listed " << msg.args()[1] << endl;

	emit incomingListedChan(msg.args()[1], msg.args()[2].toUInt(), msg.suffix());
	return true;
}

bool KIRC::numericReply_324(const KIRCMessage &msg)
{
	/* "<channel> <mode> <mode params>"
	 */
	emit incomingChannelMode(msg.args()[1], msg.args()[2], msg.args()[3]);
	return true;
}

bool KIRC::numericReply_328(const KIRCMessage &msg)
{
	/* "<channel> <mode> <mode params>"
	 */
	emit incomingChannelHomePage(msg.args()[1], msg.suffix());
	return true;
}

bool KIRC::numericReply_329( const KIRCMessage & /* msg */ )
{
	/* NOT IN RFC1459 NOR RFC2812
	 * "%s %lu"
	 * FIXME: What is the meaning of this arguments. DAL-ircd say it's a RPL_CREATIONTIME
	 */
	return true;
}

bool KIRC::numericReply_331( const KIRCMessage & /* msg */ )
{
	/* Gives the existing topic for a channel after a join.
	 * "<channel> :No topic is set"
	 */
//	emit incomingExistingTopic(args[1], suffix);
	return true;
}

bool KIRC::numericReply_332(const KIRCMessage &msg)
{
	/* Gives the existing topic for a channel after a join.
	 * "<channel> :<topic>"
	 */
	emit incomingExistingTopic(msg.args()[1], msg.suffix());
	return true;
}

bool KIRC::numericReply_333( const KIRCMessage &msg )
{
	/* Gives the nickname and time who changed the topic
	 */
	 kdDebug(14120) << k_funcinfo << endl;
	 QDateTime d;
	 d.setTime_t( msg.args()[3].toLong() );
	 emit incomingTopicUser( msg.args()[1], msg.args()[2], d );
	 return true;
}

bool KIRC::numericReply_352(const KIRCMessage &msg)
{
	QStringList suffix = QStringList::split( ' ', msg.suffix() );

	emit incomingWhoReply(
		msg.args()[5],
		msg.args()[1],
		msg.args()[2],
		msg.args()[3],
		msg.args()[4],
		msg.args()[6][0] != 'H',
		msg.args()[7],
		suffix[0].toUInt(),
		suffix[1]
	);

	return true;
}

bool KIRC::numericReply_353(const KIRCMessage &msg)
{
	emit incomingNamesList(msg.args()[2], QStringList::split(' ', msg.suffix()));
	return true;
}

bool KIRC::numericReply_372(const KIRCMessage &msg)
{
	m_motdBuffer.append( msg.suffix() );
	return true;
}

bool KIRC::numericReply_375(const KIRCMessage &)
{
	m_motdBuffer.clear();
	return true;
}

bool KIRC::numericReply_376(const KIRCMessage &)
{
	emit incomingMotd(m_motdBuffer);
	return true;
}

bool KIRC::numericReply_433(const KIRCMessage &msg)
{
	/* "<nick> :Nickname is already in use"
	 * Tells us that our nickname is already in use.
	 */
	if(m_status == Authentifying)
	{
		// This tells us that our nickname is, but we aren't logged in.
		// This differs because the server won't send us a response back telling us our nick changed
		// (since we aren't logged in).
		m_FailedNickOnLogin = true;
		emit incomingFailedNickOnLogin(msg.args()[1]);
	}
	else
	{
		// And this is the signal for if someone is trying to use the /nick command or such when already logged in,
		// but it's already in use
		emit incomingNickInUse(msg.args()[1]);
	}


	return true;
}

bool KIRC::numericReply_464(const KIRCMessage &/*msg*/)
{
	/* Server need pass.. Call disconnect */
	emit incomingFailedServerPassword();
	return true;
}

bool KIRC::numericReply_471(const KIRCMessage &msg)
{
	emit incomingFailedChanFull(msg.args()[1]);
	return true;
}

bool KIRC::numericReply_473(const KIRCMessage &msg)
{
	emit incomingFailedChanInvite(msg.args()[1]);
	return true;
}

bool KIRC::numericReply_474(const KIRCMessage &msg)
{
	emit incomingFailedChanBanned(msg.args()[1]);
	return true;
}

bool KIRC::numericReply_475(const KIRCMessage &msg)
{
	emit incomingFailedChankey(msg.args()[1]);
	return true;
}


