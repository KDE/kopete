#ifndef _KOPETE_XSLT_H
#define _KOPETE_XSLT_H

#include <qstring.h>

class KopeteXSL
{
	public:
		/**
		 * Transforms the XML string using the XSL String
		 */
		static const QString transform( const QString &xmlString, const QString &xslString );
	private:
		/**
		 * Write data to a QString
		 */
		static int writeToQString( void * context, const char * buffer, int len );

		/**
		 * Close the QString
		 */
		static int closeQString( void * context );
};

#endif
