/*
    messengerobject.cpp - MsnObject

    Copyright (c) 2007		by Zhang Panyong        <pyzhang8@gmail.com>
    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

/*
 * Refer to 
 * http://msnpiki.msnfanatic.com/index.php/MSNC:MSNObject
 * http://zoronax.bot2k3.net/msn/msnc_msnobj.html
 * http://zoronax.bot2k3.net/msn/msnc_msnobj.html
 * */
#include <kcodecs.h>

MessengerObject::MessengerObject()
{
	Type = 0;
	Size = 0;
	Friendly = QString("AAA=");
}

MessengerObject::MessengerObject(QString objData)
{
	QDomDocument objectDocument;
	objectDocument.setContent(objData, true);

	QDomNode node = objectDocument.documentElement().firstChild();
	QDomElement objElement = node.toElement();
	if(objElement.tagName() == "msnobj")
	{
		Creator = objElement.attribute("Creator");
		Size = objElement.attribute("Size").toInt();
		Type = objElement.attribute("Type").toInt();
		Location = objElement.attribute("Location");
		Friendly = objElement.attribute()
		SHA1D = objElement.attribute("SHA1D").toUtf8();
		KCodecs::base64Decode(SHA1D, data);
		SHA1C = objElement.attribute("SHA1C");
		contenttype = objElement.attribute("contenttype")	;
	   	contentid = objElement.attribute("contentid");
		stamp = objElement.attribute("stamp");
		/*{6AD16E96-BC60-401B-89E2-5BB545DD2BF0} shockwave Flash*/
		avatarid = objElement.attribute("avatarid");
		avatarcontentid = objElement.attribute("avatarcontentid");
	}
}

MessengerObject::~MessengerObject()
{

}

QString MessengerObject::toXML()
{
	QString Object;

	QString sha1d = QString(KCodecs::base64Encode(SHA1::hash(ar)));
	QString all = "Creator" + Creator +	"Size" + Size + "Type"+Type+"Location" + Location + "FriendlyAAA=SHA1D" + sha1d;
	QString sha1c = QString(KCodecs::base64Encode(SHA1::hashString(all.toUtf8())));
	Object = "<msnobj Creator=\"" + Creator + "\" Size=\"" + Size  + "\" Type=\""+ Type + "\" Location=\""+ Location + "\" Friendly=\"AAA=\" SHA1D=\""+sha1d+ "\" SHA1C=\""+sha1c+"\"/>";
}

