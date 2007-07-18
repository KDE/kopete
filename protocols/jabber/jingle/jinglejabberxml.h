#ifndef JINGLEJABBERXML_H_
#define JINGLEJABBERXML_H_ 

//#include <QtXml>

#include "xmpp_xmlcommon.h"

#include "jinglecontenttype.h"

//BEGIN JingleContentType
struct JingleContentType
{
	JingleContentType(xmlns_,name_):xmlns(xmlns_),name(name_);
	QString xmlns;
	QString name;
}
//END JingleContentType

QDomElement createInitializationMessage(QString* from, QString* to, QString* id, QString* sid, QString* initiator, QList<JingleContentType> types);

QDomElement createAcceptMessage(QString* from, QString* to, QString* initiator, QString* responder, QString* id, QString* sid, QList<JingleContentType> types);

QDomElement createTerminateMessage(QString* from, QString* to, QString* initiator, QString* responder, QString* id, QString* sid, QString* reason);
#endif
