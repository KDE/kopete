
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
		kdDebug(14150) << k_funcinfo <<
			"AIM message text: " << txt << endl;

		mText = txt;
		mText.replace(
			QRegExp("<html.*>(.*)</html>", false),
			"\\1");

		mText.replace(
			QRegExp("<body.*>(.*)</body>", false),
			"\\1");

		mText.replace(
			QRegExp("<font(.*)back=\"(.*)\"(.*)>(.*)</font>", false),
			"<font\\1style=\"background-color:\\2\"\\3>\\4</font>");
	}
	else if (format == Plain)
	{
		mText = QStyleSheet::escape(txt);
		mText.replace("\n",
			"<br/>");
		mText.replace("\t",
			"&nbsp;&nbsp;&nbsp;&nbsp; ");
		mText.replace(QRegExp("\\s\\s"),
			"&nbsp; ");
	}
	else
	{
		RTF2HTML parser;
		/*kdDebug(14150) << k_funcinfo <<
			"Original message text: " << txt << endl;*/
		//TODO: encoding
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
