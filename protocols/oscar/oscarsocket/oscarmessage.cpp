
#include <qregexp.h>
#include <qstylesheet.h>

#include <kdebug.h>
#include "oscarmessage.h"
#include "rtf2html.h"

OscarMessage::OscarMessage()
{
	timestamp = QDateTime::currentDateTime();
}

void OscarMessage::setText(const QString &txt, MessageFormat format)
{
	if(format == AimHtml)
	{
		mText = txt;
		mText.replace( QRegExp(
			QString::fromLatin1("<[hH][tT][mM][lL].*>(.*)</[hH][tT][mM][lL]>") ),
			QString::fromLatin1("\\1") );
		mText.replace( QRegExp(
			QString::fromLatin1("<[bB][oO][dD][yY].*>(.*)</[bB][oO][dD][yY]>") ),
			QString::fromLatin1("\\1") );
		mText.replace( QRegExp(
			QString::fromLatin1("<[bB][rR]>") ),
			QString::fromLatin1("<br />") );
		mText.replace( QRegExp(
			QString::fromLatin1("<[fF][oO][nN][tT].*[bB][aA][cC][kK]=(.*).*>") ),
			QString::fromLatin1("<span style=\"background-color:\\1 ;\"") );
		mText.replace( QRegExp(
			QString::fromLatin1("</[fF][oO][nN][tT]>") ),
			QString::fromLatin1("</span>") );
	}
	else if (format == Plain)
	{
		mText = QStyleSheet::escape(txt);
		mText.replace(QString::fromLatin1("\n"),
			QString::fromLatin1("<br/>"));
		mText.replace(QString::fromLatin1("\t"),
			QString::fromLatin1("&nbsp;&nbsp;&nbsp;&nbsp; "));
		mText.replace(QRegExp(QString::fromLatin1("\\s\\s")),
			QString::fromLatin1("&nbsp; "));
	}
	else
	{
		RTF2HTML parser;
		/*kdDebug(14150) << k_funcinfo <<
			"Original message text: " << txt << endl;*/
		mText = parser.Parse(txt.latin1(), "");
		/*kdDebug(14150) << k_funcinfo <<
			"Message text after RTF2HTML: " << mText << endl;*/
	}
}

const QString &OscarMessage::text()
{
	return mText;
}


const OscarMessage::MessageType OscarMessage::type()
{
	return mType;
}

void OscarMessage::setType(const MessageType val)
{
	mType = val;
}
