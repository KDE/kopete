<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:variable name="data"><xsl:value-of select="$appdata"/>/Adium</xsl:variable>
	<xsl:template match="message">
		<script type="text/javascript">
		function execute()
		{
			appendMessage(
				"<xsl:value-of disable-output-escaping="yes" select="from/contact/@contactId"/>",
				"<xsl:value-of disable-output-escaping="yes" select="from/contact/contactDisplayName/@text"/>",
				"<xsl:value-of disable-output-escaping="yes" select="from/contact/metaContactDisplayName/@text"/>",
				"<xsl:value-of select="@formattedTimestamp"/>",
				"<xsl:value-of select="from/contact/@color"/>",
				"<xsl:value-of select="translate(from/contact/@color,'0123456789AaBbCcDdEeFf','8899aabbccddddeeeeffff')"/>",
				'<xsl:call-template name="escape"><xsl:with-param name="string" select="body"/></xsl:call-template>',
				"<xsl:value-of select="@type"/>",
				"<xsl:value-of select="@route"/>",
				'<xsl:value-of select="from/contact/@userPhoto"/>',
				'<xsl:value-of select="from/contact/@protocolIcon"/>'
			);
		}

		if( typeof( scriptLoaded ) == "undefined" )
		{
			document.write("\u003Cscript src=\"<xsl:value-of select="$data"/>/adium.js\"\u003E\u003C/script\u003E");
			window.setTimeout( execute, 500 );
		}
		else
			execute();
		</script>
	</xsl:template>

	<xsl:template name="escape">
	    <xsl:param name="string" />
	    <xsl:choose>
		<xsl:when test='contains($string, "&apos;")'>
		    <xsl:value-of disable-output-escaping="yes" select='substring-before($string, "&apos;")' />
		    <xsl:text>\'</xsl:text>
		    <xsl:call-template name="escape">
			<xsl:with-param name="string" disable-output-escaping="yes" select='substring-after($string, "&apos;")' />
		    </xsl:call-template>
		</xsl:when>
		<xsl:otherwise>
		    <xsl:value-of disable-output-escaping="yes" select="$string" />
		</xsl:otherwise>
	    </xsl:choose>
	</xsl:template>

</xsl:stylesheet>

