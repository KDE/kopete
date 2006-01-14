var scriptLoaded = true;
var lastMessage = null;

function unescape( s )
{
	s = s.replace(/&lt;/g, "<");
	s = s.replace(/&gt;/g, ">");
	s = s.replace(/&amp;/g, "&");

	return s;
}

function contactName( nA, nB )
{
	if( nA != nB )
		return nB + " ( " + nA + " ) ";
	else
		return nB;
}

function imgSrc( photo, icon )
{
	if( photo && photo.length > 0 )
		return "<img src=\"data:image/png;base64," + photo + "\" height=\"48\" style=\"border: 1px solid black;\"/>";
	else
		return "<img src=\"" + icon + "\"/>";
}

function appendMessage( fromId, fromName, fromMcName, datetime, color, lightColor, body, tpe, route, photo, icon )
{
	if( lastMessage && lastMessage.contactId == fromId )
	{
		var oDiv = document.createElement("DIV");
		oDiv.style.borderTop = "thin dashed";
		oDiv.style.paddingTop = "5px";
		oDiv.style.marginTop = "5px";
		oDiv.innerHTML = unescape( body );
		lastMessage.appendChild( oDiv );
	}
	else
	{
		var oDiv = document.createElement("DIV");
		oDiv.setAttribute("style", "font-weight:bold;margin:5px;float:left;color:" + color );
		oDiv.innerHTML = imgSrc( photo, icon ) + "&nbsp;" + contactName( fromName, fromMcName );
		document.body.appendChild( oDiv );

		var oDiv2 = document.createElement("DIV");
		oDiv2.setAttribute("style", "margin:5px;float:right;color:gray" );
		oDiv2.innerHTML = datetime;
		document.body.appendChild( oDiv2 );

		var oDiv3 = document.createElement("DIV");
		oDiv3.style.border = "medium solid " + color;
		oDiv3.style.backgroundColor = lightColor;
		oDiv3.style.padding = "5px";
		oDiv3.style.marginLeft = "30px";
		oDiv3.style.marginBottom = "10px";

		if( route == "internal" )
		{
			oDiv3.style.fontSize = "xx-small";
			oDiv3.style.fontWeight = "bold";
		}

		if( tpe == "action" )
			oDiv3.innerHTML = "<i>" + fromName + "</i>&nbsp;" + unescape( body );
		else
			oDiv3.innerHTML = unescape( body );

		oDiv3.contactId = fromId;
		oDiv3.style.clear = "both";
		lastMessage = oDiv3;
		document.body.appendChild( oDiv3 );
	}
}
