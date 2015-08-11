#ifndef GNUPGPLUGIN_H
#define GNUPGPLUGIN_H

#include "kopeteplugin.h"
#include <QObject>
#include <QVariantList>


namespace Kopete
{
class Message;
class MessageEvent;
class ChatSession;
}

class GnupgPlugin : public Kopete::Plugin
{
    Q_OBJECT
public:
    static GnupgPlugin  *plugin();
    GnupgPlugin ( QObject *parent, const QVariantList &args );
    ~GnupgPlugin();

private:
    static GnupgPlugin *mPluginStatic;

private slots:
    void slotIncomingMessage(Kopete::MessageEvent *msg);
    void slotSelectContactKey();
    void slotOUtgoingMessage(Kopete::Message &msg);
};

#endif
