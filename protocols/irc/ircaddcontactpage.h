
#ifndef IRCADDCONTACTPAGE_H
#define IRCADDCONTACTPAGE_H

#include <qwidget.h>
#include <addcontactpage.h>

/**
  *@author duncan
  */
class ircAddUI;
class IRCProtocol;

class IRCAddContactPage : public AddContactPage
{
   Q_OBJECT
public:
	IRCAddContactPage(IRCProtocol *owner, QWidget *parent=0, const char *name=0);
	~IRCAddContactPage();
	ircAddUI *ircdata;
	IRCProtocol *plugin;
public slots: // Public slots
  /** No descriptions */
  virtual void slotFinish();
private slots:
	void connectNowClicked();
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

