This is the part of the login sequence that deals with importing server side contacts to the client side lists (oscar's aimbuddylist and the kopete contact list)

OS::slotRead - finds FAM 19, 9 - SRV_REPLYROSTER

calls

OS::parseSSIData - builds a new AIMBuddyList

emits

OS::gotConfig (buddyList)

connected to

OAcct::slotGotServerBuddyList - concatenates the new list with the existing internal buddy list, then, for each buddy in the new list

calls

OAcct::addServerContact - checks new contact not self or already in client list, consistency check - does its group exist, then

calls

KopeteAcct::addContact(...) - creates a KMC if necessary

calls

OAcct::addContactToMetaContact - checks if already in buddy list, if so,

calls

(AIM|ICQ)Acct:createNewContact - creates new kopete contact subclass

And that's the end of it.
