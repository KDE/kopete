#ifndef OSCARMESSAGE_H
#define OSCARMESSAGE_H

#include <qstring.h>
#include <qdatetime.h>
#include <qcolor.h>

/**
 * @author Stefan Gehn
 */
class OscarMessage
{
	public:
		enum MessageFormat
		{
			Plain=0, Rtf, AimHtml
		};

		enum MessageType
		{
			Normal=0, Away, URL, SMS, WebPanel, EMail, GrantedAuth, DeclinedAuth
		};

		OscarMessage();
		void setText(const QString &txt, MessageFormat format);
		const QString &text();

		/**
		 * Describes the message type, i.e. normal-msg, away-msg, sms-msg ...
		 */
		const MessageType type();
		void setType(const MessageType val);

	public:
		QDateTime timestamp;
		QColor fgColor;
		QColor bgColor;

	private:
		QString mText;
		MessageType mType;
};

#endif
