<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="html"/>
	<xsl:template match="message">
		<div class="KopeteMessage" style="padding-bottom:10px;">
			<xsl:attribute name="id">
				<xsl:value-of select="@id"/>
			</xsl:attribute>
			<xsl:if test="number(@direction) &lt; 2">
				<div>
					<xsl:choose>
						<xsl:when test="@direction='1'">
							<xsl:attribute name="style">
								<xsl:text>color:red;font-weight:bold;</xsl:text>
							</xsl:attribute>
						    <kopete-i18n>Message to %TO_CONTACT_DISPLAYNAME% at %FORMATTEDTIMESTAMP%</kopete-i18n>
						</xsl:when>
						<xsl:otherwise>
							<xsl:attribute name="style">
								<xsl:text>color:blue;font-weight:bold;</xsl:text>
							</xsl:attribute>
							<kopete-i18n>Message from %FROM_CONTACT_DISPLAYNAME% at %FORMATTEDTIMESTAMP%</kopete-i18n>
						</xsl:otherwise>
					</xsl:choose>
				</div>
				<xsl:text disable-output-escaping="yes">&#160;&#160;&#160;&#160;</xsl:text>
			</xsl:if>
			<span>
				<xsl:attribute name="dir">
					<xsl:value-of select="body/@dir"/>
				</xsl:attribute>
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
				<xsl:choose>
					<xsl:when test="@direction='3'">
						<!-- Action Message -->
						<span style="color:darkgreen">
							<kopete-i18n>* %FROM_CONTACT_DISPLAYNAME%&#160;%BODY%</kopete-i18n>
						</span>
					</xsl:when>
					<xsl:when test="@direction='2'">
						<!-- Internal Message -->
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

