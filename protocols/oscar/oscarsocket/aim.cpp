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
