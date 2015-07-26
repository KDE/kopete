#ifndef GNUPGPLUGIN_H
#define GNUPGPLUGIN_H


#include "kopeteplugin.h"

namespace Kopete
{
	class Message;
	class MessageEvent;
	class ChatSession;
}

class CryptographyPlugin : public Kopete::Plugin
{
		Q_OBJECT

	public:
		static CryptographyPlugin  *plugin();
};

#endif