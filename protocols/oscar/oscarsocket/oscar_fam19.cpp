/*
    oscar_fam19.cpp  -  Oscar Protocol, SNAC Family 19 (SSI)

    Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "oscarsocket.h"
#include "oscardebug.h"
#include "oscaraccount.h"

#include <kdebug.h>

const WORD CLI_SSI_CHECKOUT = 0x0005;
const WORD CLI_SSIxADD = 0x0008;
const WORD CLI_SSIxUPDATE = 0x0009;
const WORD CLI_SSIxDELETE = 0x000a;
const WORD CLI_SSI_EDIT_BEGIN = 0x0011;
const WORD CLI_SSI_EDIT_END = 0x0012;



// SNAC(19,02)
void OscarSocket::sendSSIRightsRequest()
{
	kdDebug(14150) << k_funcinfo << "SEND (CLI_REQLISTS)" << endl;
	Buffer outbuf;
	outbuf.addSnac(0x0013,0x0002,0x0000,0x00000002);
	sendBuf(outbuf,0x02);
}


// SNAC(19,03) SRV_SSI_RIGHTS_REPLY
// http://iserverd.khstu.ru/oscar/snac_13_03.html
void OscarSocket::parseSSIRights(Buffer &/*inbuf*/)
{
	kdDebug(14150) << k_funcinfo << "RECV (SRV_REPLYLISTS) IGNORING" << endl;
	//List of TLV's
	//TLV of type 4 contains a bunch of words, representing maximums
	// word 0 of TLV 4 data: max contacts
	// word 1 of TLV 4 data: max groups
	// word 2 of TLV 4 data: max visible-list entries
	// word 3 of TLV 4 data: max invisible-list entries
	// word 4 of TLV 4 data: max vis/invis bitmasks
	// word 5 of TLV 4 data: max presence info fields

	gotAllRights++;
	if (gotAllRights==7)
	{
		kdDebug(14150) << k_funcinfo "gotAllRights==7" << endl;
		sendInfo();
	}
}


// SNAC(19,04)
void OscarSocket::sendSSIRequest()
{
	kdDebug(14150) << "SEND (CLI_REQROSTER), " <<
		"requesting serverside contactlist for the FIRST time" << endl;
	Buffer outbuf;
	outbuf.addSnac(0x0013,0x0004,0x0000,0x00020004);
	sendBuf(outbuf,0x02);
}


// SNAC(19,05)
void OscarSocket::sendRosterRequest()
{
	kdDebug(14150) << k_funcinfo <<
		"SEND (CLI_CHECKROSTER) Requesting SSI data" << endl;
	Buffer outbuf;
	outbuf.addSnac(OSCAR_FAM_19,CLI_SSI_CHECKOUT,0x0000,0x00000000);
	outbuf.addDWord(0x00000000); // FIXME: contactlist timestamp
	outbuf.addWord(0x0000); // FIXME: contactlist length, same as first Word gotten in parseSSIData
	sendBuf(outbuf,0x02);
}


// SNAC(19,06) SRV_SSIxREPLY
// http://iserverd.khstu.ru/oscar/snac_13_06.html
void OscarSocket::parseSSIData(Buffer &inbuf)
{
	unsigned int length = 0;
	QStringList blmBuddies;

	inbuf.getByte(); //get fmt version
	length = inbuf.getWord();

	kdDebug(14150) << k_funcinfo <<
		"RECV (SRV_REPLYROSTER) got contactlist, length =  " << length << endl;

	while(inbuf.length() > 4) //the last 4 bytes are the timestamp
	{
		SSI *ssi = new SSI;
		const char *itemName = inbuf.getBSTR(); //name
		ssi->name = ServerToQString(itemName, 0L, false); // just guess encoding
		//ssi->name = QString::fromLocal8Bit(itemName);
		delete[] itemName;

		ssi->gid = inbuf.getWord();
		ssi->bid = inbuf.getWord();
		ssi->type = inbuf.getWord(); //type of the entry
		ssi->tlvlength = inbuf.getWord(); //length of data
		if (ssi->tlvlength > 0) //sometimes there is additional info
			ssi->tlvlist = inbuf.getBlock(ssi->tlvlength);
		else
			ssi->tlvlist = 0L;
		ssi->waitingAuth = false;

		mSSIData.append(ssi);
#ifdef OSCAR_SSI_DEBUG
		kdDebug(14150) << k_funcinfo << "Read server-side list entry." <<
			" name = '" << ssi->name << "', groupId = " << ssi->gid <<
			", id = " << ssi->bid << ", type = " << ssi->type <<
			", TLV length = " << ssi->tlvlength << endl;
#endif

		switch (ssi->type)
		{
			case ROSTER_CONTACT: // normal contact
			{
				parseSSIContact(ssi, blmBuddies);
				break;
			}

			case ROSTER_GROUP: //group of contacts
			{
				parseSSIGroup(ssi);
				break;
			}

			case ROSTER_VISIBLE: // permit buddy list AKA visible list
				break;

			case ROSTER_INVISIBLE: // deny buddy AKA invisible list
				break;

			case ROSTER_VISIBILITY: // Visibility Setting (probably ICQ only!)
			{
				parseSSIVisibility(ssi);
				break;
			}

			case ROSTER_PRESENCE: // Presence info (if others can see your idle status, etc)
			{
				kdDebug(14150) << k_funcinfo <<
					"TODO: Handle ROSTER_PRESENCE (AIM only)" << endl;
				break;
			}

			case ROSTER_ICQSHORTCUT: // Unknown or ICQ2k shortcut bar items
				break;

			case ROSTER_IGNORE: // Contact on ignore list
				break;

			case ROSTER_LASTUPDATE: // Last update date (name: "LastUpdateDate")
			case ROSTER_NONICQ: // a non-icq contact, no UIN, used to send SMS
			case ROSTER_IMPORTTIME: // roster import time (name: "Import time")
			case ROSTER_BUDDYICONS: // Buddy icon info. (names: from "0" and incrementing by one)
				break;

			default:
			{
				kdDebug(14150) << k_funcinfo <<
					"TODO: Handle UNKNOWN SSI type: " << ssi->type << endl;
				break;
			}
		} // END switch (ssi->type)
	} // END while(inbuf.length() > 4)

	int timestamp = inbuf.getDWord();

#ifdef OSCAR_SSI_DEBUG
	kdDebug(14150) << k_funcinfo <<
		"Finished getting contact list, timestamp=" << timestamp << endl;
#endif

	if (blmBuddies.size() > 0)
		sendBuddylistAdd(blmBuddies);

	// If the server list is splited on more than one packet, only the last one has timestamp != 0
	if (timestamp > 0)
	{
		sendSSIActivate(); // send CLI_ROSTERACK
		emit gotConfig();

		gotAllRights++;
		if (gotAllRights==7)
		{
			kdDebug(14150) << k_funcinfo << "gotAllRights==7" << endl;
			sendInfo();
		}
	}
}


void OscarSocket::parseSSIContact(SSI *pSsi, QStringList &blmContacts)
{
	SSI* group = mSSIData.findGroup(pSsi->gid);
	QString groupName = "\"Group not found\"";
	if (group)
		groupName = group->name;

	/*
	 * FIXME: if the contact has got a groupID assigned which we really don't
	 * have a group for then this code will goddamn blow up because we continue
	 * with the non-existant groupID and just assume some group name
	 */

#ifdef OSCAR_SSI_DEBUG
	kdDebug(14150) << k_funcinfo << "Adding Contact '" << pSsi->name <<
		"' to group " << pSsi->gid << " (" << groupName << ")" << endl;
#endif

	Buffer tmpBuf(pSsi->tlvlist, pSsi->tlvlength);
	QPtrList<TLV> lst = tmpBuf.getTLVList();
	lst.setAutoDelete(TRUE);

	TLV *t;
	for(t=lst.first(); t; t=lst.next())
	{
		switch(t->type)
		{
			case 0x0131: // nickname
			{
				/*if(t->length > 0)
				{
					kdDebug(14150) << k_funcinfo <<
						"TODO: contact '" << pSsi->name <<
						"' has alias on serverside-list: '" <<
						t->data << "'" << endl;
				}*/
				break;
			}

			case 0x0066: // waitauth flag
			{
				/* Signifies that you are awaiting authorization for this buddy.
				The client is in charge of putting this TLV, but you will not
				receiving status updates for the contact until they authorize
				you, regardless if this is here or not. Meaning, this is only
				here to tell your client that you are waiting for authorization
				for the person. This TLV is always empty.
				*/
				kdDebug(14150) << k_funcinfo <<
					"Contact "<<pSsi->name<<" has WAITAUTH set." << endl;
				//TODO: reimplement somehow. Set waitauth flag and add to blm lists
				pSsi->waitingAuth = true;
				blmContacts << pSsi->name;
				break;
			}

			case 0x0137:
			{
				// locally assigned email address for contact, TODO
				if(t->length > 0)
				{
					kdDebug(14150) << k_funcinfo <<
						"TODO: contact '" << pSsi->name <<
						"' has email on serverside-list: '" <<
						t->data << "'" << endl;
				}
				break;
			}

			case 0x0145:
			{
				// unix timestamp when you first talked to this contact
				// set by icqlite
				// Maybe TODO?
				break;
			}

			default:
			{
#ifdef OSCAR_SSI_DEBUG
				kdDebug(14150) << k_funcinfo <<
					"UNKNOWN TLV(" << t->type << "), length=" << t->length << endl;
				QString tmpStr;
				for (unsigned int dc=0; dc < t->length; dc++)
				{
					if (static_cast<unsigned char>(t->data[dc]) < 0x10)
						tmpStr += "0";
					tmpStr += QString("%1 ").arg(static_cast<unsigned char>(t->data[dc]),0,16);
					if ((dc>0) && (dc % 10 == 0))
						tmpStr += QString("\n");
				}
				kdDebug(14150) << k_funcinfo << tmpStr << endl;
#endif
				break;
			} // END default
		} // END switch()
	} // END for()
	lst.clear();
} // END parseSSIContact()


void OscarSocket::parseSSIGroup(SSI *pSsi)
{
#ifdef OSCAR_SSI_DEBUG
	Buffer tmpBuf(pSsi->tlvlist, pSsi->tlvlength);
	QPtrList<TLV> lst = tmpBuf.getTLVList();
	lst.setAutoDelete(TRUE);
	kdDebug(14150) << k_funcinfo << "Group entry contained TLVs:" << endl;
	TLV *t;
	for(t=lst.first(); t; t=lst.next())
	{
		kdDebug(14150) << k_funcinfo <<
			"TLV(" << t->type << "), length=" << t->length << endl;
	}

	if (!pSsi->name.isEmpty()) //if it's not the master group
	{
		kdDebug(14150) << k_funcinfo << "Adding Group " <<
			pSsi->gid << " (" <<  pSsi->name << ")" << endl;
	}
#endif
}


void OscarSocket::parseSSIVisibility(SSI *pSsi)
{
	kdDebug(14150) << k_funcinfo << "Read server-side list-entry. name='" <<
		pSsi->name << "', groupId=" << pSsi->gid << ", id=" << pSsi->bid <<
		", type=" << pSsi->type << ", TLV length=" << pSsi->tlvlength << endl;

	Buffer tmpBuf(pSsi->tlvlist, pSsi->tlvlength);
	QPtrList<TLV> lst = tmpBuf.getTLVList();
	lst.setAutoDelete(TRUE);

	// visibility setting, needed for invisible mode
	TLV *visibility = findTLV(lst,0x00ca);

	if(visibility)
	{

		int vis = (int)(visibility->data[0]);
		switch(vis)
		{
			case 1:
				kdDebug(14150) << k_funcinfo <<
					"visibility setting = Allow all users to see you" << endl;
				break;

			case 2:
				kdDebug(14150) << k_funcinfo <<
					"visibility setting = Block all users from seeing you" << endl;
				break;

			case 3:
				kdDebug(14150) << k_funcinfo <<
					"visibility setting = Allow only users in the permit list to see you" << endl;
				break;

			case 4:
				kdDebug(14150) << k_funcinfo <<
					"visibility setting = Block only users in the invisible list from seeing you" << endl;
				break;

			case 5:
				kdDebug(14150) << k_funcinfo <<
					"visibility setting = Allow only users in the buddy list to see you" << endl;
				break;

			default:
				kdDebug(14150) << k_funcinfo <<
					"visibility setting (UNKNOWN)=" << vis << endl;
		}
	} // END if(visibility)

	//TODO: TLV(0x00CB) aim visibility
	// http://iserverd.khstu.ru/oscar/ssi_item.html

	TLV *allowOthers = findTLV(lst,0x00CC);
	if(allowOthers)
	{
		DWORD vis = (DWORD)((allowOthers->data[0] << 8) | allowOthers->data[1]);
		if (vis & 0x00000002)
		{
			kdDebug(14150) << k_funcinfo <<
				"Do not allow others to see that I am using a wireless device" << endl;
		}

		if (vis & 0x00000400)
		{
			kdDebug(14150) << k_funcinfo <<
			 	"Allow others to see my idle time" << endl;
		}

		if (vis & 0x00400000)
		{
			kdDebug(14150) << k_funcinfo <<
				"Allow others to see that I am typing a response" << endl;
		}
	} // END if(allowOthers)
}


// SNAC(19,07) CLI_SSI_ACTIVATE
// http://iserverd.khstu.ru/oscar/snac_13_07.html
void OscarSocket::sendSSIActivate()
{
	kdDebug(14150) << k_funcinfo <<
		"SEND (CLI_ROSTERACK), sending SSI Activate" << endl;
	Buffer outbuf;
	outbuf.addSnac(OSCAR_FAM_19,0x0007,0x0000,0x00000000);
	sendBuf(outbuf,0x02);
}


void OscarSocket::parseSSIOk(Buffer &inbuf)
{
	kdDebug(14150) << k_funcinfo << "RECV (SRV_REPLYROSTEROK) " \
		"received ack for contactlist timestamp/size" << endl;

	// TODO: REPLYROSTEROK can happen on login if timestamp and length
	// are both zero.
	// That means the user has no serverside contactlist at all!
	// I hope just going on with login works fine [mETz, 22.06.2003]

	long timestamp = inbuf.getDWord();
	int size = inbuf.getWord();

	kdDebug(14150) << k_funcinfo << "acked list timestamp=" << timestamp <<
	", list size=" << size << endl;

	gotAllRights++;
	if (gotAllRights==7)
	{
		kdDebug(14150) << k_funcinfo "gotAllRights==7" << endl;
		sendInfo();
	}
}


void OscarSocket::sendAddBuddy(const QString &contactName, const QString &groupName, bool addingAuthBuddy)
{
	SSI *newContact, *group;

	kdDebug(14150) << k_funcinfo << "Sending SSI add buddy" << endl;

	group = mSSIData.findGroup(groupName);
	if (!group)
	{
		group = mSSIData.addGroup(groupName);
		sendAddGroup(groupName);
	}

	newContact = mSSIData.addContact(contactName, groupName, addingAuthBuddy);

	kdDebug(14150) << k_funcinfo << "Adding " << newContact->name << ", gid " <<
		newContact->gid << ", bid " << newContact->bid << ", type " << newContact->type
		<< ", datalength " << newContact->tlvlength << endl;

	DWORD reqId = sendSSIAddModDel(newContact, 0x0008);
	addBuddyToAckMap(contactName, groupName, reqId);

	// TODO: now we need to modify the group our buddy is in, i.e. edit TLV(0x00C9)
	// containing the list of contactIDs inside that group
//	sendSSIAddModDel(group, 0x0009);
}



void OscarSocket::sendChangeVisibility(BYTE value)
{
	kdDebug(14150) << k_funcinfo << "Setting visibility to " << value << endl;

	// Check to make sure that the group has actually changed
	SSI *ssi = mSSIData.findVisibilitySetting();
	if (!ssi)
	{
		kdDebug(14150) << k_funcinfo <<
			"No visibility type found in contactlist, doing nothing" << endl;
		return;
	}

	Buffer tmpBuf(ssi->tlvlist, ssi->tlvlength);
	QPtrList<TLV> lst = tmpBuf.getTLVList();

	// important, we are acting on our fetched serverside data
	// which should NOT get deleted!
	lst.setAutoDelete(FALSE);

	TLV *visibility = findTLV(lst,0x00ca);

	if (visibility)
	{

		if(visibility->data[0] == value)
		{
/*			kdDebug(14150) << k_funcinfo <<
				"Visibility already set to value " << value << ", aborting!" << endl; */
			return;
		}

		kdDebug(14150) << k_funcinfo <<
			"Modifying visibility, current value=" << visibility->data[0] << endl;

		// construct new SSI entry replacing the old one
		SSI *newSSI = new SSI();
		newSSI->name = ssi->name;
		newSSI->gid = ssi->gid;
		newSSI->bid = ssi->bid;
		newSSI->type = ssi->type;
		Buffer *newSSITLV = new Buffer();
		for(TLV* t = lst.first(); t; t = lst.next())
		{
			if(t->type!=0x00ca)
			{
				newSSITLV->addTLV(t->type, t->length, t->data);
				lst.remove(t);
			}
		}

		visibility->data[0] = value;
		newSSITLV->addTLV(visibility->type, visibility->length, visibility->data);

		if (!mSSIData.remove(ssi))
		{
			kdDebug(14150) << k_funcinfo <<
				"Couldn't remove old ssi containing visibility value" << endl;
			delete newSSITLV;
			delete newSSI;
			return;
		}
		newSSI->tlvlist = newSSITLV->buffer();
		newSSI->tlvlength = newSSITLV->length();

		mSSIData.append(newSSI);

		kdDebug(14150) << k_funcinfo <<
			"new visibility value=" << visibility->data[0] << endl;

		kdDebug(14150) << k_funcinfo << "Sending SSI Data to server" << endl;
		// Make the call to sendSSIAddModDel requesting a "modify"
		// SNAC (0x0009) with the buddy with the modified group number
		sendSSIAddModDel(newSSI, 0x0009);
	}
	else
	{
		kdDebug(14150) << k_funcinfo <<
			"No visibility TLV found in contactlist, doing nothing" << endl;
		return;
	}

	// Send debugging info that we're done
	kdDebug(14150) << k_funcinfo << "Completed" << endl;
}


void OscarSocket::sendRenameBuddy(const QString &budName,
	const QString &budGroup, const QString &newAlias)
{
	kdDebug(14150) << k_funcinfo << "Called." << endl;

	SSI *ssi = mSSIData.findContact(budName, budGroup);

	if (!ssi)
	{
		kdDebug(14150) << k_funcinfo << "Item with name '" << budName << "' and group '"
			<< budGroup << "' not found!" << endl;

		emit protocolError(
			i18n("%1 in group %2 was not found on the server's " \
			"contact list and cannot be renamed.")
#if QT_VERSION < 0x030200
			.arg(budName).arg(budGroup)
#else
			.arg(budName,budGroup)
#endif
			,0, false);
		return;
	}

	Buffer tmpBuf(ssi->tlvlist, ssi->tlvlength);
	QPtrList<TLV> lst = tmpBuf.getTLVList();
	lst.setAutoDelete(FALSE);

	/*TLV *oldNick = findTLV(lst,0x0131);

	if (oldNick)
	{
	kdDebug(14150) << k_funcinfo <<
			"Renaming contact, current alias='" << oldNick->data << "'" << endl;
		lst.remove(oldNick); // get rid of TLV copy
	}
	else
	{
		kdDebug(14150) << k_funcinfo <<
			"Renaming contact, no alias had been given before." << endl;
	}*/

	// construct new SSI entry replacing the old one
	SSI *newSSI = new SSI();
	newSSI->name = ssi->name;
	newSSI->gid = ssi->gid;
	newSSI->bid = ssi->bid;
	newSSI->type = ssi->type;
	Buffer *newSSIdata = new Buffer();

	for(TLV* t = lst.first(); t; t = lst.next())
	{
		if(t->type != 0x0131)
		{
			newSSIdata->addTLV(t->type, t->length, t->data);
			lst.remove(t);
		}
	}

	//const char *newNickData = newAlias.local8Bit().copy();
	newSSIdata->addTLV(0x0131, newAlias.local8Bit().length(), newAlias.local8Bit());

	if (!mSSIData.remove(ssi))
	{
		kdDebug(14150) << k_funcinfo <<
			"Couldn't remove old ssi containing contact" << endl;
		delete newSSIdata;
		delete newSSI;
		return;
	}
	newSSI->tlvlist = newSSIdata->buffer();
	newSSI->tlvlength = newSSIdata->length();

	mSSIData.append(newSSI);

	kdDebug(14150) << k_funcinfo << "Renaming, new SSI block: name=" << newSSI->name <<
		", gid=" << newSSI->gid << ", bid=" << newSSI->bid <<
		", type=" << newSSI->type << ", datalength=" << newSSI->tlvlength << endl;

	kdDebug(14150) << "new SSI:" << newSSIdata->toString();

	sendSSIAddModDel(newSSI, 0x0009);
}


int OscarSocket::sendAddGroup(const QString &name)
{
	kdDebug(14150) << k_funcinfo << "Called. Adding group to SSI" << endl;

	SSI *newitem = mSSIData.addGroup(name);
	if(!newitem)
	{
		kdDebug(14150) << k_funcinfo <<
			"Null SSI returned from addGroup(), group must already exist!" << endl;
		return(0);
	}

	kdDebug(14150) << k_funcinfo << "Adding group, name='" << name <<
		"' gid=" << newitem->gid << endl;
	sendSSIAddModDel(newitem,0x0008);
	return(newitem->gid);
}


// Changes the name of a group on the server side list
void OscarSocket::sendChangeGroupName(const QString &currentName,
	const QString &newName)
{
	kdDebug(14150) << k_funcinfo
		<< "Renaming '" << currentName << "' to '" << newName << "'" << endl;

	// Check to make sure that the name has actually changed
	if (currentName == newName)
	{  // Name hasn't changed, don't do anything
		kdDebug(14150) << k_funcinfo
			<< "Name not actually changed, doing nothing"
			<< endl;
		return;
	}

	// Get the SSI data item to send using the
	// sendSSIAddModDel method
	SSI *updatedItem = mSSIData.renameGroup(currentName, newName);
	// Make the call to sendSSIAddModDel requesting a "modify"
	// SNAC (0x0009)
	sendSSIAddModDel(updatedItem, 0x0009);
}


void OscarSocket::sendDelGroup(const QString &groupName)
{
	kdDebug(14150) << k_funcinfo
		<< "Removing group " << groupName << "from SSI" << endl;

	// Get the SSIData for this operation
	SSI *delGroup = mSSIData.findGroup(groupName);

	// Print out the SSI Data for debugging purposes
	mSSIData.print();

	if (!delGroup)
	{	// There was an error finding the group
		kdDebug(14150) << "Group with name " << groupName
			<< " not found" << endl;
		emit protocolError(
			i18n("Group %1 was not found on the server's " \
				 "buddy list and cannot be deleted.").arg(groupName), 0, false);
		return;
	}

	// We found it, print out a debugging statement saying so
	kdDebug(14150) << k_funcinfo << "Group found, removing" << endl;
	// Send the remove request, which is family 0x0013
	// subtype 0x000a
	sendSSIAddModDel(delGroup,0x000a);

	// Remove it from the internal Server Side Information
	// list
	if (!mSSIData.remove(delGroup))
	{
		kdDebug(14150) << k_funcinfo << delGroup
			<< " was not found in the SSI list" << endl;
	}
}


// Sends SSI add, modify, or delete request, to reuse code
DWORD OscarSocket::sendSSIAddModDel(SSI *item, WORD requestType)
{
	if (!item)
		return(0);

	switch(requestType)
	{
		case CLI_SSIxADD:
		{
			kdDebug(14150) << k_funcinfo << "SEND (CLI_ADDSTART)" << endl;
			Buffer addstart;
			addstart.addSnac(OSCAR_FAM_19,CLI_SSI_EDIT_BEGIN,0x0000,0x00000000);
			sendBuf(addstart,0x02);
			kdDebug(14150) << k_funcinfo << "SEND (CLI_ROSTERADD)" << endl;
			break;
		}
		case CLI_SSIxUPDATE:
		{
			kdDebug(14150) << k_funcinfo << "SEND (CLI_ROSTERUPDATE)" << endl;
			break;
		}
		case CLI_SSIxDELETE:
		{
			kdDebug(14150) << k_funcinfo << "SEND (CLI_ROSTERDELETE)" << endl;
			break;
		}
		default:
		{
			kdDebug(14150) << k_funcinfo << "unknown requestType given, aborting" << endl;
			return(0);
		}
	}

	Buffer outbuf;
	DWORD reqId = outbuf.addSnac(OSCAR_FAM_19,requestType,0x0000,0x00000000);
	outbuf.addBSTR(item->name.latin1()); // TODO: encoding
	outbuf.addWord(item->gid); // TAG
	outbuf.addWord(item->bid); // ID
	outbuf.addWord(item->type); // TYPE
	outbuf.addWord(item->tlvlength); // LEN
	if (item->tlvlength > 0)
	{
		kdDebug(14150) << k_funcinfo << "Adding TLVs with length=" <<
			item->tlvlength << endl;
		outbuf.addString(item->tlvlist,item->tlvlength);
	}

	sendBuf(outbuf,0x02);

	if(requestType==CLI_SSIxADD)
	{
		kdDebug(14150) << k_funcinfo << "SEND (CLI_ADDEND)" << endl;
		Buffer addend;
		addend.addSnac(OSCAR_FAM_19,CLI_SSI_EDIT_END,0x0000,0x00000000);
		sendBuf(addend,0x02);
	}
	return(reqId);
}


// Deletes a buddy from the server side contact list
void OscarSocket::sendDelBuddy(const QString &budName, const QString &budGroup)
{
	kdDebug(14150) << k_funcinfo << "Sending del contact" << endl;

	SSI *delitem = mSSIData.findContact(budName,budGroup);
	mSSIData.print();
	if (!delitem)
	{
		kdDebug(14150) << "Item with name " << budName << " and group "
			<< budGroup << "not found" << endl;
		return;
	}

	kdDebug(14150) << k_funcinfo << "Deleting " << delitem->name << ", gid " <<
		delitem->gid << ", bid " << delitem->bid << ", type " << delitem->type <<
		", datalength " << delitem->tlvlength << endl;

	sendSSIAddModDel(delitem,0x000a);

	if (!mSSIData.remove(delitem))
	{
		kdDebug(14150) << k_funcinfo <<
			"delitem was not found in the SSI list" << endl;
	}
}






void OscarSocket::sendSSIAddIgnore(const QString &name)
{
	SSI *newitem = mSSIData.addIgnore(name);
	if (!newitem)
		return;

	kdDebug(14150) << k_funcinfo << "Adding contact to IGNORE list:" <<
		newitem->name << ", gid=" <<
		newitem->gid << ", bid=" << newitem->bid << ", type=" <<
		newitem->type << ", datalength=" << newitem->tlvlength << endl;

	sendSSIAddModDel(newitem, CLI_SSIxADD);

	// FIXME: Use SNAC headers and SSI acks to do this more correctly
	//emit denyAdded(sname);
}

void OscarSocket::sendSSIRemoveIgnore(const QString &name)
{
	kdDebug(14150) << k_funcinfo << "Removing contact '" <<
		name << "' from IGNORE list" << endl;

	SSI *delitem = mSSIData.findIgnore(name);
	if (!delitem)
	{
		kdDebug(14150) << k_funcinfo << "Item with name " << name <<
			"not found" << endl;
		return;
	}

	kdDebug(14150) << k_funcinfo << "Deleting " << delitem->name <<
		", gid " << delitem->gid <<
		", bid " << delitem->bid << ", type " << delitem->type <<
		", datalength " << delitem->tlvlength << endl;

	sendSSIAddModDel(delitem, CLI_SSIxDELETE);

	if (!mSSIData.remove(delitem))
	{
		kdDebug(14150) << k_funcinfo <<
			"delitem was not found in the SSI list" << endl;
	}

	// FIXME: Use SNAC headers and SSI acks to do this more correctly
	//emit denyRemoved(sname);
}


void OscarSocket::sendSSIAddInvisible(const QString &name)
{
	SSI *newitem = mSSIData.addInvisible(name);
	if (!newitem)
		return;

	kdDebug(14150) << k_funcinfo << "Adding contact to INVISIBLE list:" <<
		newitem->name << ", gid=" <<
		newitem->gid << ", bid=" << newitem->bid << ", type=" <<
		newitem->type << ", datalength=" << newitem->tlvlength << endl;

	sendSSIAddModDel(newitem, CLI_SSIxADD);

	// FIXME: Use SNAC headers and SSI acks to do this more correctly
	//emit denyAdded(sname);
}

void OscarSocket::sendSSIRemoveInvisible(const QString &name)
{
	kdDebug(14150) << k_funcinfo << "Removing contact '" <<
		name << "' from INVISIBLE list" << endl;

	SSI *delitem = mSSIData.findInvisible(name);
	if (!delitem)
	{
		kdDebug(14150) << k_funcinfo << "Item with name " << name <<
			"not found" << endl;
		return;
	}

	kdDebug(14150) << k_funcinfo << "Deleting " << delitem->name <<
		", gid " << delitem->gid <<
		", bid " << delitem->bid << ", type " << delitem->type <<
		", datalength " << delitem->tlvlength << endl;

	sendSSIAddModDel(delitem, CLI_SSIxDELETE);

	if (!mSSIData.remove(delitem))
	{
		kdDebug(14150) << k_funcinfo <<
			"delitem was not found in the SSI list" << endl;
	}

	// FIXME: Use SNAC headers and SSI acks to do this more correctly
	//emit denyRemoved(sname);
}


void OscarSocket::sendSSIAddVisible(const QString &name)
{
	SSI *newitem = mSSIData.addVisible(name);
	if (!newitem)
		return;

	kdDebug(14150) << k_funcinfo << "Adding contact to VISIBLE list:" <<
		newitem->name << ", gid=" <<
		newitem->gid << ", bid=" << newitem->bid << ", type=" <<
		newitem->type << ", datalength=" << newitem->tlvlength << endl;

	sendSSIAddModDel(newitem, CLI_SSIxADD);

	// FIXME: Use SNAC headers and SSI acks to do this more correctly
	//emit denyAdded(sname);
}

void OscarSocket::sendSSIRemoveVisible(const QString &name)
{
	kdDebug(14150) << k_funcinfo << "Removing contact '" <<
		name << "' from VISIBLE list" << endl;

	SSI *delitem = mSSIData.findVisible(name);
	if (!delitem)
	{
		kdDebug(14150) << k_funcinfo << "Item with name " << name <<
			"not found" << endl;
		return;
	}

	kdDebug(14150) << k_funcinfo << "Deleting " << delitem->name <<
		", gid " << delitem->gid <<
		", bid " << delitem->bid << ", type " << delitem->type <<
		", datalength " << delitem->tlvlength << endl;

	sendSSIAddModDel(delitem, CLI_SSIxDELETE);

	if (!mSSIData.remove(delitem))
	{
		kdDebug(14150) << k_funcinfo <<
			"delitem was not found in the SSI list" << endl;
	}

	// FIXME: Use SNAC headers and SSI acks to do this more correctly
	//emit denyRemoved(sname);
}







// Changes the group a buddy is in on the server
void OscarSocket::sendChangeBuddyGroup(const QString &buddyName,
	const QString &oldGroup, const QString &newGroup)
{
	kdDebug(14150) << k_funcinfo <<
			"Moving " << buddyName << " into group " << newGroup << endl;

	// Check to make sure that the group has actually changed
	SSI *buddyItem = mSSIData.findContact(buddyName, oldGroup);
	SSI *groupItem = mSSIData.findGroup(newGroup);
	if (buddyItem == 0L || groupItem == 0L)
	{
		kdDebug(14150) << k_funcinfo <<
			": Buddy or group not found, doing nothing" << endl;
		return;
	}

	if (buddyItem->gid != groupItem->gid)
	{ // The buddy isn't in the group
		kdDebug(14150) << k_funcinfo <<
			": Modifying buddy's group number in the SSI Data" << endl;

		// Ok, this is a strange sequence - Penna
		Buffer editStart,
		       delBuddy,
		       addBuddy,
		       changeGroup,
		       editEnd;

		// Send CLI_SSI_EDIT_BEGIN
		editStart.addSnac(OSCAR_FAM_19,0x0011,0x0000,0x00000000);
		sendBuf(editStart,0x02);

		// Send CLI_SSIxDELETE with the BuddyID and the GroupID, but null screenname
		delBuddy.addSnac(OSCAR_FAM_19,0x000a,0x0000,0x00000000);
		delBuddy.addBSTR(buddyItem->name.latin1());
		delBuddy.addWord(buddyItem->gid);
		delBuddy.addWord(buddyItem->bid);
		delBuddy.addWord(buddyItem->type);
		delBuddy.addWord(0); // no TVL
		sendBuf(delBuddy,0x02);

		// Change the buddy's group number
		buddyItem->gid = groupItem->gid;

		// Send CLI_SSIxADD with the new buddy group
		addBuddy.addSnac(OSCAR_FAM_19,0x0008,0x0000,0x00000000);
		addBuddy.addBSTR(buddyItem->name.latin1());
		addBuddy.addWord(buddyItem->gid);
		addBuddy.addWord(buddyItem->bid);
		addBuddy.addWord(buddyItem->type);
		addBuddy.addWord(buddyItem->tlvlength); // LEN
		if (buddyItem->tlvlength > 0)
		{
			kdDebug(14150) << k_funcinfo << "Adding TLVs with length=" <<
				buddyItem->tlvlength << endl;
			addBuddy.addString(buddyItem->tlvlist,buddyItem->tlvlength);
		}
		sendBuf(addBuddy,0x02);

		// Send CLI_SSIxUPDATE for the group
		changeGroup.addSnac(OSCAR_FAM_19,0x0009,0x0000,0x00000000);
		changeGroup.addBSTR(groupItem->name.latin1());
		changeGroup.addWord(groupItem->gid);
		changeGroup.addWord(groupItem->bid);
		changeGroup.addWord(groupItem->type);
		changeGroup.addWord(6);
		changeGroup.addTLV16(0xc8, buddyItem->bid);
		sendBuf(changeGroup,0x02);

		// Send CLI_SSI_EDIT_END
		editEnd.addSnac(OSCAR_FAM_19,0x0012,0x0000,0x00000000);
		sendBuf(editEnd,0x02);
	}
	else
	{
		kdDebug(14150) << k_funcinfo <<
			"Buddy already in group, doing nothing" << endl;
		return;
	}

	// Send debugging info that we're done
	kdDebug(14150) << k_funcinfo << ": Completed" << endl;
}


// Parses the SSI acknowledgment
void OscarSocket::parseSSIAck(Buffer &inbuf, const DWORD reqId)
{
	kdDebug(14150) << k_funcinfo << "RECV SRV_SSIACK" << endl;

	WORD result = inbuf.getWord();
	AckBuddy buddy = ackBuddy(reqId);

	OscarContact *contact = 0L;

	SSI* ssiItem = mSSIData.findContact(buddy.contactName, buddy.groupName);

	if (!buddy.contactName.isEmpty())
		contact = static_cast<OscarContact*>(mAccount->contacts()[buddy.contactName]);

	switch(result)
	{
		case SSIACK_OK:
			kdDebug(14150) << k_funcinfo << "SSI change succeeded" << endl;
			break;
		case SSIACK_NOTFOUND:
			kdDebug(14150) << k_funcinfo << "Modified item not found on server." << endl;
			break;
		case SSIACK_ALREADYONSERVER:
			kdDebug(14150) << k_funcinfo << "Added item already on server." << endl;
			break;
		case SSIACK_ADDERR:
			kdDebug(14150) << k_funcinfo << "Error adding item (invalid id, already in list, invalid data)" << endl;
			break;
		case SSIACK_LIMITEXD:
			kdDebug(14150) << k_funcinfo << "Cannot add item, item limit exceeded." << endl;
			break;
		case SSIACK_ICQTOAIM:
			kdDebug(14150) << k_funcinfo << "Cannot add ICQ contact to AIM list." << endl;
			break;
		case SSIACK_NEEDAUTH:
		{
			kdDebug(14150) << k_funcinfo << "Cannot add contact because he needs AUTH." << endl;
			contact->requestAuth();
			sendAddBuddy(buddy.contactName, buddy.groupName, true);
			sendAddBuddylist(buddy.contactName);
			ssiItem->waitingAuth = true;
			break;
		}
		default:
			kdDebug(14150) << k_funcinfo << "Unknown result " << result << endl;
	}
}


void OscarSocket::addBuddyToAckMap(const QString &contactName, const QString &groupName, const DWORD id)
{
	kdDebug(14150) << k_funcinfo << "Mapping ID " << id <<
		" to buddy " << contactName << endl;

	AckBuddy buddy;
	buddy.contactName = contactName;
	buddy.groupName = groupName;
	m_ackBuddyMap[id] = buddy;
}

AckBuddy OscarSocket::ackBuddy(const DWORD id)
{
	AckBuddy buddy;
	QMap<DWORD, AckBuddy>::Iterator it;
	for (it = m_ackBuddyMap.begin() ; it != m_ackBuddyMap.end() ; it++)
	{
		if (it.key() == id)
		{
			kdDebug(14150) << k_funcinfo << "Found buddy " <<
				it.data().contactName << ", group " << it.data().groupName << endl;
			buddy = it.data();
			m_ackBuddyMap.remove(it);
			break;
		}
	}
	return(buddy);
}
