<?xml version="1.0" encoding="UTF-8"?>
<!-- Minimal Kopete style. Copyright (c)2004, Gav Wood. This file may be distributed under
       the GPL licence. -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="html"/>
<xsl:template match="message">
<div class="KopeteMessage"><xsl:attribute name="id"><xsl:value-of disable-output-escaping="yes" select="@id"/></xsl:attribute>
<!-- Choose based on message direction -->
<xsl:choose>
	<xsl:when test="@direction='2'"><!--internal message-->
		<span>
			<xsl:attribute name="dir">
				<xsl:value-of select="body/@dir"/>
			</xsl:attribute>
			<xsl:attribute name="style">
				<xsl:if test="body/@bgcolor">
					<xsl:text>background-color: </xsl:text>
					<xsl:value-of select="body/@bgcolor"/>
					<xsl:text>; </xsl:text>
				</xsl:if>
				<xsl:if test="body/@font">
					<xsl:text>font: </xsl:text>
					<xsl:value-of select="body/@font"/>
					<xsl:text>; </xsl:text>
				</xsl:if>
				<xsl:text>font-size: small; color: #dddddd; padding: 1px; border-width: 1px; border-style: solid; border-color: #cccccc; </xsl:text>
			</xsl:attribute>
			<xsl:value-of disable-output-escaping="yes" select="body"/>
		</span>
	</xsl:when>
	<xsl:when test="@direction='3'"><!--action message-->
		<span>
			<xsl:attribute name="dir">
				<xsl:value-of select="body/@dir"/>
			</xsl:attribute>
			<xsl:attribute name="style">
				<xsl:if test="body/@bgcolor">
					<xsl:text>background-color: </xsl:text>
					<xsl:value-of select="body/@bgcolor"/>
					<xsl:text>; </xsl:text>
				</xsl:if>
				<xsl:if test="body/@font">
					<xsl:text>font: </xsl:text>
					<xsl:value-of select="body/@font"/>
					<xsl:text>; </xsl:text>
				</xsl:if>
				<xsl:text>color: #80d380; </xsl:text>
			</xsl:attribute>
			<xsl:text>* </xsl:text><xsl:value-of disable-output-escaping="yes" select="from/contact/contactDisplayName/@text"/>
			<xsl:text> </xsl:text><xsl:value-of disable-output-escaping="yes" select="body"/>
		</span>
	</xsl:when>
	<xsl:when test="@direction='1'"><!-- outgoing -->
		<span>
			<xsl:attribute name="dir">
				<xsl:value-of select="body/@dir"/>
			</xsl:attribute>
			<xsl:attribute name="style">
				<xsl:if test="body/@bgcolor">
					<xsl:text>background-color:</xsl:text>
					<xsl:value-of select="body/@bgcolor"/>
					<xsl:text>; </xsl:text>
				</xsl:if>
				<xsl:if test="body/@font">
					<xsl:text>font: </xsl:text>
					<xsl:value-of select="body/@font"/>
					<xsl:text>; </xsl:text>
				</xsl:if>
				<xsl:text>color: #9999ff; </xsl:text>
			</xsl:attribute>
			<xsl:value-of disable-output-escaping="yes" select="body"/>
		</span>
	</xsl:when>
	<xsl:when test="@direction='0'"><!-- incoming -->
		<span>
			<xsl:attribute name="dir">
				<xsl:value-of select="body/@dir"/>
			</xsl:attribute>
			<xsl:attribute name="style">
				<xsl:if test="body/@color">
					<xsl:text>color: </xsl:text>
					<xsl:value-of select="body/@color"/>
					<xsl:text>;</xsl:text>
				</xsl:if>
				<xsl:if test="body/@bgcolor">
					<xsl:text>background-color: </xsl:text>
					<xsl:value-of select="body/@bgcolor"/>
					<xsl:text>;</xsl:text>
				</xsl:if>
				<xsl:if test="body/@font">
					<xsl:text>font: </xsl:text>
					<xsl:value-of select="body/@font"/>
					<xsl:text>;</xsl:text>
				</xsl:if>
				<xsl:text>font-size: small; </xsl:text>
			</xsl:attribute>
			<xsl:value-of disable-output-escaping="yes" select="from/contact/contactDisplayName/@text"/>
			<xsl:text>: </xsl:text>
		</span>
		<span>
			<xsl:attribute name="dir">
				<xsl:value-of select="body/@dir"/>
			</xsl:attribute>
			<xsl:attribute name="style">
				<xsl:if test="body/@color">
					<xsl:text>color: </xsl:text>
					<xsl:value-of select="body/@color"/>
					<xsl:text>; </xsl:text>
				</xsl:if>
				<xsl:if test="body/@bgcolor">
					<xsl:text>background-color: </xsl:text>
					<xsl:value-of select="body/@bgcolor"/>
					<xsl:text>; </xsl:text>
				</xsl:if>
				<xsl:if test="body/@font">
					<xsl:text>font: </xsl:text>
					<xsl:value-of select="body/@font"/>
					<xsl:text>; </xsl:text>
				</xsl:if>
				<xsl:text>font-weight: bold; </xsl:text>
			</xsl:attribute>
			<xsl:value-of disable-output-escaping="yes" select="body"/>
		</span>
	</xsl:when>
</xsl:choose>
</div>
</xsl:template>
</xsl:stylesheet>
			<!--xsl:text>(</xsl:text>
			<xsl:value-of disable-output-escaping="yes" select="body/@font"/>
			<xsl:text>,</xsl:text>
			<xsl:value-of disable-output-escaping="yes" select="body/@bgcolor"/>
			<xsl:text>,</xsl:text>
			<xsl:value-of disable-output-escaping="yes" select="body/@color"/>
			<xsl:text>)</xsl:text-->
