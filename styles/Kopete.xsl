<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="html"/>
<xsl:template match="message">
<div class="KopeteMessage" style="padding-bottom:10px;"><xsl:attribute name="id"><xsl:value-of select="@id"/></xsl:attribute>
	<xsl:if test="number(@direction) &lt; 2">
		<div>
			<xsl:choose>
			<xsl:when test="@direction='1'">
				<xsl:attribute name="style"><xsl:text>color:red;font-weight:bold;</xsl:text></xsl:attribute>
				<xsl:text>Message to </xsl:text><xsl:value-of disable-output-escaping="yes" select="to/contact/@contactDisplayName"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:attribute name="style"><xsl:text>color:blue;font-weight:bold;</xsl:text></xsl:attribute>
				<xsl:text>Message from </xsl:text><xsl:value-of disable-output-escaping="yes" select="from/contact/@contactDisplayName"/>
			</xsl:otherwise>
			</xsl:choose>
			<xsl:text> at </xsl:text><xsl:value-of select="@time"/>
		</div>
		<xsl:text disable-output-escaping="yes">&#160;&#160;&#160;&#160;</xsl:text>
	</xsl:if>
	<span>
	<xsl:attribute name="style">		<xsl:if test="body/@color"><xsl:text>color:</xsl:text><xsl:value-of select="body/@color"/><xsl:text>;</xsl:text></xsl:if><xsl:if test="body/@bgcolor"><xsl:text>background-color:</xsl:text><xsl:value-of select="body/@bgcolor"/></xsl:if>
					<xsl:if test="body/@font"><xsl:text>; </xsl:text><xsl:value-of select="body/@font"/></xsl:if> </xsl:attribute>
		<xsl:if test="@importance='2'">
			<xsl:attribute name="class"><xsl:text>highlight</xsl:text></xsl:attribute>
		</xsl:if>
		<xsl:choose>
		<xsl:when test="@direction='3'">
			<span style="color:darkgreen">
				* <xsl:value-of disable-output-escaping="yes" select="from/contact/@contactDisplayName"/> <xsl:text> </xsl:text> <xsl:value-of disable-output-escaping="yes" select="body"/>
			</span>
		</xsl:when>
		<xsl:when test="@direction='2'">
			<span style="color:darkviolet;font-weight:bold;">
				<xsl:value-of disable-output-escaping="yes" select="body"/>
			</span>
		</xsl:when>

		<xsl:otherwise>
			<xsl:value-of disable-output-escaping="yes" select="body"/>
		</xsl:otherwise>
		</xsl:choose>
	</span>
</div>
</xsl:template>
</xsl:stylesheet>
