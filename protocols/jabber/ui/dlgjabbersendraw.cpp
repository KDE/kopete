#include <qlayout.h>
#include <qlineedit.h>
#include <qlabel.h>

#include <kdebug.h>
#include <klocale.h>
#include <qtextedit.h>

#include "kopetewindow.h"
#include "dlgpreferences.h"
#include "dlgjabbersendraw.h"
#include "dlgsendraw.h"
#include "jabberprotocol.h"
#include "jabber.h"

dlgJabberSendRaw::dlgJabberSendRaw(JabberProtocol *owner,
			           QWidget *parent,
			           const char *name)
:dlgSendRaw(parent, name) {
	plugin = owner;
	connect(btnFinish, SIGNAL(clicked()), this, SLOT(slotFinish()));
	connect(btnCancel, SIGNAL(clicked()), this, SLOT(slotCancel()));
	show();
}

dlgJabberSendRaw::~dlgJabberSendRaw() {
}

void dlgJabberSendRaw::slotFinish() {
	plugin->sendRawMessage(tePacket->text());
	hide();
}

void dlgJabberSendRaw::slotCancel() {
	hide();
}

#include "dlgjabbersendraw.moc"
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

