
#include "kircentity.h"

const QRegExp userRegexp(QString::fromLatin1("(.*)(?:!(.*))(?:@(.*))"));
const QRegExp channelRegexp( QString::fromLatin1("^[#!+&][^\\s,:]+$") );

#include "kircentity.moc"