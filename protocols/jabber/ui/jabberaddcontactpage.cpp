#include <qlayout.h>
#include <klineedit.h>

#include <klocale.h>

#include "jabberaddcontactpage.h"

JabberAddContactPage::JabberAddContactPage (KopeteAccount * owner, QWidget * parent, const char *name):AddContactPage (parent, name)
{
	(new QVBoxLayout (this))->setAutoAdd (true);
	if (owner->isConnected ())
	  {
		  jabData = new dlgAddContact (this);
		  jabData->show ();
		  plugin = owner;
		  canadd = true;

	  }
	else
	  {
		  noaddMsg1 = new QLabel (i18n ("You need to be connected to be able to add contacts."), this);
		  noaddMsg2 = new QLabel (i18n ("Connect to the Jabber network and try again."), this);
		  canadd = false;
	  }

}

JabberAddContactPage::~JabberAddContactPage ()
{
}

bool JabberAddContactPage::validateData ()
{
	return true;
}

void JabberAddContactPage::slotFinish (KopeteMetaContact * mc)
{
	if (canadd)
	  {
		  QString userID = jabData->addID->text ();

		  //plugin->addContact(mc, userID);
	  }
	else
	  {
		  return;
	  }
}

bool JabberAddContactPage::apply (KopeteAccount * i, KopeteMetaContact * m)
{
	if (validateData ())
	  {
		  QString userid = i->myself ()->contactId ();

		  return i->addContact (userid, userid, m);
	  }
	return false;
}

#include "jabberaddcontactpage.moc"

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:
