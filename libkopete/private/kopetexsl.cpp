#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxml/xmlIO.h>
#include <libxml/parser.h>

#include <kdebug.h>
#include <kopetexsl.h>

extern int xmlLoadExtDtdDefaultValue;

const QString KopeteXSL::transform( const QString &xmlString, const QString &xslString )
{
	QString parsed;

	xsltStylesheetPtr style_sheet = NULL;
	xmlDocPtr xmlDoc, xslDoc, resultDoc;

	//Init Stuff
	xmlInitMemory();
	xmlLoadExtDtdDefaultValue = 0;
	xmlSubstituteEntitiesDefault(1);

	// Convert QString into a C string
	QCString xmlCString = xmlString.latin1();
	QCString xslCString = xslString.latin1();

	// Read XML docs in from memory
	xmlDoc = xmlParseMemory( xmlCString, xmlCString.length() );
	xslDoc = xmlParseMemory( xslCString, xslCString.length() );

	if( xslDoc != NULL && xmlDoc != NULL )
	{
		style_sheet = xsltParseStylesheetDoc( xslDoc );
		resultDoc = xsltApplyStylesheet(style_sheet, xmlDoc, NULL);
		if( resultDoc != NULL )
		{
			//Save the result into the QString
			xmlOutputBufferPtr outp = xmlOutputBufferCreateIO( writeToQString, (xmlOutputCloseCallback)closeQString, &parsed, 0);
			outp->written = 0;
			xsltSaveResultTo ( outp, resultDoc, style_sheet );
			xmlOutputBufferFlush(outp);
			xmlFreeDoc(resultDoc);
		}
		else
		{
			kdDebug() << "Transformed document is null!!!" << endl;
		}
		xmlFreeDoc(xmlDoc);
		xsltFreeStylesheet(style_sheet);
	}
	else
	{
		kdDebug() << "XML/XSL Document could not be parsed!!!" << endl;
	}

	//Cleanup
	xsltCleanupGlobals();
	xmlCleanupParser();
	xmlMemoryDump();

	return parsed;
}

int KopeteXSL::writeToQString( void * context, const char * buffer, int len )
{
	QString *t = (QString*)context;
	*t += QString::fromUtf8(buffer, len);
	return len;
}

int KopeteXSL::closeQString( void * context )
{
	QString *t = (QString*)context;
	*t += QString::fromLatin1("\n");
	return 0;
}
