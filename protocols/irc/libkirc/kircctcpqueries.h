#ifndef _KIRC_CTCP_QUERIES_H
#define _KIRC_CTCP_QUERIES_H

bool KIRC::CtcpQuery_action(const KIRCMessage &msg)
{
	QString target = msg.args()[0];
	if (target[0] == '#' || target[0] == '!' || target[0] == '&')
		emit incomingAction(msg.prefix(), target, msg.ctcpMessage().ctcpRaw());
	else
		emit incomingPrivAction(msg.prefix(), target, msg.ctcpMessage().ctcpRaw());
	return true;
}


bool KIRC::CtcpQuery_pingPong(const KIRCMessage &msg)
{
	writeCtcpReplyMessage(	msg.prefix(), QString::null,
				msg.ctcpMessage().command(), msg.ctcpMessage().args()[0]);
	return true;
}

bool KIRC::CtcpQuery_version(const KIRCMessage &msg)
{
	QString response = customCtcpMap[ QString::fromLatin1("version") ];
	kdDebug(14120) << "Version check: " << response << endl;

	if( !response.isNull() )
	{
		writeCtcpReplyMessage(msg.prefix(), QString::null,
			msg.ctcpMessage().command(), QStringList(), response);
	}
	else
	{
		writeCtcpReplyMessage(msg.prefix(), QString::null,
			msg.ctcpMessage().command(), QStringList(), m_VersionString);
	}

	return true;
}


bool KIRC::CtcpQuery_userInfo(const KIRCMessage &msg)
{
	QString response = customCtcpMap[ QString::fromLatin1("userinfo") ];
	if( !response.isNull() )
	{
		writeCtcpReplyMessage(msg.prefix(), QString::null,
			msg.ctcpMessage().command(), QStringList(), response);
	}
	else
	{
		writeCtcpReplyMessage( msg.prefix(), QString::null,
				msg.ctcpMessage().command(), QStringList(), m_UserString );
	}

	return true;
}

//	FIXME: the API can now answer to help commands.
bool KIRC::CtcpQuery_clientInfo(const KIRCMessage &msg)
{
	QString response = customCtcpMap[ QString::fromLatin1("clientinfo") ];
	if( !response.isNull() )
	{
		writeCtcpReplyMessage(	msg.prefix(), QString::null,
					msg.ctcpMessage().command(), QStringList(), response);
	}
	else
	{
		QString info = QString::fromLatin1("The following commands are supported, but "
			"without sub-command help: VERSION, CLIENTINFO, USERINFO, TIME, SOURCE, PING,"
			"ACTION.");

		writeCtcpReplyMessage(	msg.prefix(), QString::null,
					msg.ctcpMessage().command(), QStringList(), info);
	}
	return true;
}

bool KIRC::CtcpQuery_time(const KIRCMessage &msg)
{
	writeCtcpReplyMessage(	msg.prefix(), QString::null,
				msg.ctcpMessage().command(), QStringList(QDateTime::currentDateTime().toString()), QString::null,
				false);
	return true;
}

bool KIRC::CtcpQuery_source(const KIRCMessage &msg)
{
	writeCtcpReplyMessage(	msg.prefix(), QString::null,
				msg.ctcpMessage().command(), QStringList(m_SourceString));
	return true;
}

bool KIRC::CtcpQuery_finger( const KIRCMessage & /* msg */ )
{
	// To be implemented
	return true;
}

bool KIRC::CtcpQuery_dcc(const KIRCMessage &msg)
{
	const KIRCMessage &ctcpMsg = msg.ctcpMessage();
	QString dccCommand = ctcpMsg.args()[0].upper();

	if (dccCommand == QString::fromLatin1("CHAT"))
	{
		if(ctcpMsg.args().size()!=4) return false;

		/* DCC CHAT type longip port
		 *
		 *  type   = Either Chat or Talk, but almost always Chat these days
		 *  longip = 32-bit Internet address of originator's machine
		 *  port   = Port on which the originator is waitng for a DCC chat
		 */
		bool okayHost, okayPort;
		// should ctctArgs[1] be tested?
		QHostAddress address(ctcpMsg.args()[2].toUInt(&okayHost));
		unsigned int port = ctcpMsg.args()[3].toUInt(&okayPort);
		if (okayHost && okayPort)
		{
			kdDebug(14120) << "Starting DCC chat window." << endl;
			DCCClient *chatObject = new DCCClient(address, port, 0, DCCClient::Chat);
			emit incomingDccChatRequest(address, port, getNickFromPrefix(msg.prefix()), *chatObject);
			return true;
		}
	}
	else if (dccCommand == QString::fromLatin1("SEND"))
	{
		if(ctcpMsg.args().size()!=5) return false;

		/* DCC SEND (filename) (longip) (port) (filesize)
		 *
		 *  filename = Name of file being sent
		 *  longip   = 32-bit Internet address of originator's machine
		 *  port     = Port on which the originator is waiitng for a DCC chat
		 *  filesize = Size of file being sent
		 */
		bool okayHost, okayPort, okaySize;
		QFileInfo realfile(msg.args()[1]);
		QHostAddress address(ctcpMsg.args()[2].toUInt(&okayHost));
		unsigned int port = ctcpMsg.args()[3].toUInt(&okayPort);
		unsigned int size = ctcpMsg.args()[4].toUInt(&okaySize);
		if (okayHost && okayPort && okaySize)
		{
			kdDebug(14120) << "Starting DCC send file transfert." << endl;
			DCCClient *chatObject = new DCCClient(address, port, size, DCCClient::File);
			emit incomingDccSendRequest(address, port, getNickFromPrefix(msg.prefix()), realfile.fileName(), size, *chatObject);
			return true;
		}
	}
//	else
//		emit unknown dcc command signal
	return false;
}

#endif
