<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="html"/>
<xsl:template match="message">
<div class="KopeteMessage"><xsl:attribute name="id"><xsl:value-of select="@id"/></xsl:attribute>
<xsl:attribute name="style">		<xsl:if test="body/@color"><xsl:text>color:</xsl:text><xsl:value-of select="body/@color"/><xsl:text>;</xsl:text></xsl:if><xsl:if test="body/@bgcolor"><xsl:text>background-color:</xsl:text><xsl:value-of select="body/@bgcolor"/></xsl:if></xsl:attribute>

<xsl:if test="@direction='3'">
	<xsl:attribute name="style"><xsl:text>color:darkgreen</xsl:text></xsl:attribute>
</xsl:if>
[<xsl:value-of select="@time"/>]
<!-- Choose based on message direction -->
<xsl:choose>
	<xsl:when test="@direction='2'"><!--internal message-->
		-<font color="cyan">--</font>
	</xsl:when>
	<xsl:when test="@direction='3'"><!--action message-->
		<span class="KopeteDisplayName"><xsl:value-of disable-output-escaping="yes" select="from/contact/@metaContactDisplayName"/></span><xsl:text disable-output-escaping="yes">&#160;</xsl:text>
	</xsl:when>
	<xsl:otherwise>
		<font color="blue">&lt;</font>
		<font>
			<xsl:attribute name="color">
				<xsl:choose>
					<xsl:when test="@direction='1'"> <!-- Outgoing -->
						<xsl:text>yellow</xsl:text>
					</xsl:when>
					<xsl:otherwise> <!-- Incoming -->
						<xsl:value-of select="from/contact/@color"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:attribute>
			<span class="KopeteDisplayName"><xsl:value-of disable-output-escaping="yes" select="from/contact/@metaContactDisplayName"/></span>
		</font>
		<font color="blue">&gt; </font>
	</xsl:otherwise>
</xsl:choose>
<span>
<xsl:if test="@importance='2'">
	<xsl:attribute name="class"><xsl:text>KopeteMessage highlight</xsl:text></xsl:attribute>
</xsl:if>
<xsl:value-of disable-output-escaping="yes" select="body"/>
</span>
</div>
</xsl:template>
</xsl:stylesheet>
