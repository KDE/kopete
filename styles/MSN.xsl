<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="html"/>
	<xsl:template match="message">
		<div style="padding-bottom:10px;" class="KopeteMessage">
			<xsl:attribute name="id">
				<xsl:value-of select="@id"/>
			</xsl:attribute>
			<xsl:if test="@direction &lt; 2">
				<div style="color:gray">
					(<xsl:value-of select="@time"/>)
					<span>
						<xsl:attribute name="title">
							<xsl:value-of disable-output-escaping="yes" select="from/contact/@contactId"/>
						</xsl:attribute>
						<xsl:value-of disable-output-escaping="yes" select="from/contact/@contactDisplayName"/> says:
					</span>
				</div>
				<xsl:text disable-output-escaping="yes">&#160;&#160;&#160;&#160;</xsl:text>
			</xsl:if>
			<span>
				<xsl:attribute name="style">
					<xsl:if test="body/@color">
						<xsl:text>color:</xsl:text>
						<xsl:value-of select="body/@color"/>
						<xsl:text>;</xsl:text>
					</xsl:if>
					<xsl:if test="body/@bgcolor">
						<xsl:text>background-color:</xsl:text>
						<xsl:value-of select="body/@bgcolor"/>
					</xsl:if>
					 <xsl:if test="body/@font">
						<xsl:text>; </xsl:text>
						<xsl:value-of select="body/@font"/>
					</xsl:if>
				</xsl:attribute>
				<xsl:if test="@importance='2'">
					<xsl:attribute name="class">
						<xsl:text>highlight</xsl:text>
					</xsl:attribute>
				</xsl:if>
				<xsl:if test="@direction='3'">
					<xsl:attribute name="style">
						<xsl:text>color:darkGreen;bold</xsl:text>
					</xsl:attribute>
					<xsl:text>* </xsl:text>
					<span>
						<xsl:attribute name="title">
							<xsl:value-of disable-output-escaping="yes" select="from/contact/@contactId"/>
						</xsl:attribute>
						<xsl:value-of disable-output-escaping="yes" select="from/contact/@contactDisplayName"/>
						<xsl:text> </xsl:text>
					</span>
				</xsl:if>
				<xsl:if test="@direction='2'">
					<xsl:attribute name="style">
						<xsl:text>color:darkviolet;italic</xsl:text>
					</xsl:attribute>
					<xsl:text disable-output-escaping="yes">-- </xsl:text>
				</xsl:if>
				<xsl:value-of disable-output-escaping="yes" select="body"/>
			</span>
		</div>
	</xsl:template>
</xsl:stylesheet>

