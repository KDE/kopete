
#ifndef DLGJABBERSENDRAW_H
#define DLGJABBERSENDRAW_H

#include <qwidget.h>
#include <qlabel.h>
#include "jabberprotocol.h"
#include "dlgsendraw.h"
#include <qspinbox.h>

/**
  *@author Till Gerken
  */
class dlgSendRaw;
class JabberProtocol;
class dlgJabberSendRaw;

class dlgJabberSendRaw : public dlgSendRaw {
  Q_OBJECT
  public:
    dlgJabberSendRaw(JabberProtocol *owner, QWidget *parent =
			 0, const char *name = 0);
    ~dlgJabberSendRaw();
    JabberProtocol *plugin;
  public slots:
    void slotFinish();
    void slotCancel();
};


#endif
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

