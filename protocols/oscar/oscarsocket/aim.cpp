// aim.cpp
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#include "config.h"

#include <qstring.h>
#include "aim.h"
#include <qregexp.h>

//#define tocNormalize(s) (s.lower().replace(QRegExp(" "),""))

QString tocNormalize(const QString &oldstr){
  return oldstr.lower().replace(QRegExp(" "),"");
}

QString tocRoast(const QString &oldstr){
	const QString tocRoast_string = "Tic/Toc";
	const int tocRoast_length = tocRoast_string.length();

	QString newstr;
	QString workstr;
	unsigned char c;

	newstr = "0x";

	for(unsigned int i = 0; i < oldstr.length(); i++)
	{
		c = oldstr[i] ^ (tocRoast_string[i % tocRoast_length]);
		workstr.sprintf("%02x", c);
		newstr += workstr;
	}

	return newstr;
}
QString tocProcess(const QString &oldstr)
{
	/*Arguments
with whitespace characters should be enclosed in quotes.  Dollar signs, 
curly brackets, square brackets, parentheses, quotes, and backslashes 
must all be backslashed whether in quotes or not.*/
	QString newstr = "\"";
	for(unsigned int i = 0; i < oldstr.length(); i++)
	{
		switch( (char)(QChar)(oldstr.at(i)) )
		{
			//case '\n':
			//	break;
			case '$':
			case '{':
			case '}':
			case '[':
			case ']':
			case '(':
			case ')':
			case '\'':
			case '"':
			case '\\':
				newstr += '\\';
				// fallthrough
			default:
				newstr += (char)(QChar)oldstr[i];
		}
	}
	newstr += "\"";
	return newstr;
}
void tocParseConfig(const QString data, TBuddyList *buddyList, TBuddyList *permitList, TBuddyList *denyList, int *permitStatus)
{
	QString workstr;
	QString holding;
	TBuddy buddy;
	int i;

	workstr = data;

	// which group buddies belong in
	int group = 0;
	// skip "CONFIG:"
	workstr.remove(0, 7);

	while(workstr.length() > 0)
	{
// leave workstr at the start of the line
#define tocParseToHolding( x ) workstr.remove(0, 2);\
                               i = workstr.find("\n");\
                               if(i != -1)\
                               {\
                                  holding = workstr.left(i);\
                                  workstr.remove(0, i + 1);\
                               }\
                               else\
                               {\
                                  holding = workstr;\
                                  workstr.remove(0, workstr.length());\
						 }

		switch((char)(QChar)workstr[0])
		{
			case 'm':
				tocParseToHolding("case m");
				*permitStatus = holding.toInt();
				break;
			case 'g':
				tocParseToHolding("case g");
				// now add the name to the config, saving the current group number
				group = buddyList->addGroup( holding );
				if (group == -1)
					group = buddyList->getNumGroup(holding);
				break;
			case 'b':
				tocParseToHolding("case b");
				// set relevant elements of buddy to add to list
				buddy.name = holding;
				buddy.alias = buddy.name;
				buddy.group = group;
				buddy.idleTime = 0;
				buddy.status = TAIM_OFFLINE;
				buddy.lastOn = 0;
				// now add to list
				buddyList->add(&buddy);
				break;
			case 'p':
				tocParseToHolding("case p");
				// set relevant elements of buddy to add to list
				buddy.name = holding;
				buddy.group = 0;
				// now add to list
				permitList->add(&buddy);
				break;
			case 'd':
				tocParseToHolding("case d");
				// set relevant elements of buddy to add to list
				buddy.name = holding;
				buddy.group = 0;
				// now add to list
				denyList->add(&buddy);
				break;
			default:
				tocParseToHolding("default");
		}
	}
}


void tocParseConfigAlias(const QString data, TBuddyList *buddyList, TBuddyList *permitList, TBuddyList *denyList, int *permitStatus)
{
	QString workstr = data;
	QString holding;
	TBuddy buddy;
	int i;

	// which group buddies belong in
	int group = -1;
	// skip "CONFIG:"
	workstr.remove(0, 7);

	while(workstr.length() > 0)
	{
  		int colen;
		switch((char)(QChar)workstr[0])
		{
			case 'm':
				tocParseToHolding("case m");
				*permitStatus = holding.toInt();
				break;
			case 'g':
				tocParseToHolding("case g");
				// now add the name to the config, saving the current group number
				group = buddyList->addGroup( holding );
				if (group == -1)
				  group = buddyList->getNumGroup(holding);
				break;
			case 'b':
				tocParseToHolding("case b");
				// set relevant elements of buddy to add to list
				// Make sure there is an alias to get first!!!
				colen = holding.find(QString(":"), 0);
				if (colen != -1){
				  buddy.name = holding.mid(0,colen);
                                  buddy.alias = holding.mid(colen+1,holding.length()-colen-1);
				}
				else{
				   buddy.name = holding;
				   buddy.alias = buddy.name;
                                }


				buddy.group = group;
				if (group == -1){
			          QString d = QString("Warning, buddy ") + (buddy.name) + " not added, please report.\n";
 qDebug(d.latin1());
				}
				buddy.status = TAIM_OFFLINE;
				buddy.lastOn = 0;
				// now add to list
				buddyList->add(&buddy);
				break;
			case 'p':
				tocParseToHolding("case p");
				// set relevant elements of buddy to add to list
				buddy.name = holding;
				buddy.group = 0;
				// now add to list
				permitList->add(&buddy);
				break;
			case 'd':
				tocParseToHolding("case d");
				// set relevant elements of buddy to add to list
				buddy.name = holding;
				buddy.group = 0;
				// now add to list
				denyList->add(&buddy);
				break;
			default:
				tocParseToHolding("default");
		}
	}
}
QString tocWriteConfig(const TBuddyList *buddyList, const TBuddyList *permitList, const TBuddyList *denyList, int permitStatus)
{
	QString data;
	data.sprintf("m %1i\n", permitStatus);

	int i = -1;
	
	for (int group =0; group < buddyList->getCountGroup(); group++){
	  data += "g " + buddyList->getNameGroup(group) + "\n";
	  
	 // int currentGroup = buddyList->getGroup(group);
	  i = -1;
	  while(++i < buddyList->getCount()){
	    if(group == buddyList->getGroup(i))
	       data += "b " + buddyList->getName(i) + "\n";
	  }
        }		  
	
	    
	i = -1;
	while(++i < permitList->getCount())
	{
		data += "p " + permitList->getName(i) + "\n";
	}
	i = -1;
	while(++i < denyList->getCount())
	{
		data += "d " + denyList->getName(i) + "\n";
	}
	return data;
}
