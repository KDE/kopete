<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="html"/>
	<xsl:template match="message">
		<div class="KopeteMessage">
			<xsl:attribute name="style">
				<xsl:text>margin-bottom:2.25em;</xsl:text>
				<xsl:choose>
					<xsl:when test="@direction='2'"><!-- Internal message -->
						<xsl:text>border: 2px solid #800000; border-top:1.5em solid #800000;margin-top: 1.7em;</xsl:text>
					</xsl:when>
					<xsl:when test="@direction='1'"><!-- Outgoing -->
						<xsl:text>border: 2px solid #97ADC3; border-top:1.5em solid #97ADC3;margin-top: 1.7em;</xsl:text>
					</xsl:when>
					<xsl:otherwise><!-- Incoming / Action message -->
						<xsl:text>border: 2px solid #4A98EB; border-top:1.5em solid #4A98EB;margin-top: 1.7em;</xsl:text>
					</xsl:otherwise>
				</xsl:choose>
				<xsl:if test="@importance='2'">
					<xsl:text>background-color:#FFFA95;</xsl:text>
				</xsl:if>
			</xsl:attribute>
			<div>
				<xsl:attribute name="style">
					<xsl:choose>
						<xsl:when test="@direction='2'"><!-- Internal message -->
							<xsl:text>color:white;font-weight:bold;width:55%;margin-top:-2.7em;margin-left: 1em;padding:4px;padding-left:10px;border: 1px solid white;background-color:#800000;</xsl:text>
						</xsl:when>
						<xsl:when test="@direction='1'"><!-- Outgoing -->
							<xsl:text>color:white;font-weight:bold;width:55%;margin-top:-2.7em;margin-left: 1em;padding:4px;padding-left:10px;border: 1px solid white;background-color:#97ADC3;</xsl:text>
						</xsl:when>
						<xsl:otherwise><!-- Incoming / Action message -->
							<xsl:text>color:white;font-weight:bold;width:55%;margin-top:-2.7em;margin-left: 1em;padding:4px;padding-left:10px;border: 1px solid white;background-color:#4A98EB;</xsl:text>
						</xsl:otherwise>
					</xsl:choose>
				</xsl:attribute>
				<xsl:choose>
					<xsl:when test="@direction='2'"><!-- Internal message -->
						<xsl:text>System Message</xsl:text>
					</xsl:when>
					<xsl:when test="@direction='1'"><!-- Outgoing -->
						<xsl:text>To </xsl:text>
						<span class="KopeteDisplayName">
							<xsl:value-of disable-output-escaping="yes" select="from/contact/@metaContactDisplayName"/>
						</span>
						<xsl:text> at </xsl:text>
						<xsl:value-of select="@time"/>
					</xsl:when>
					<xsl:otherwise><!-- Incoming / Action message -->
						<xsl:text>From </xsl:text>
						<span class="KopeteDisplayName">
							<xsl:value-of disable-output-escaping="yes" select="from/contact/@metaContactDisplayName"/>
						</span>
						<xsl:text> at </xsl:text>
						<xsl:value-of select="@time"/>
					</xsl:otherwise>
				</xsl:choose>
			</div>
			<div>
				<xsl:attribute name="id">
					<xsl:value-of select="@id"/>
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
						<xsl:text>; </xsl:text>
					</xsl:if>
					<xsl:if test="body/@font">
						<xsl:text>font-family:</xsl:text>
						<xsl:value-of select="body/@font"/>
						<xsl:text>; </xsl:text>
					</xsl:if>
					<xsl:text>margin-top:8px;padding:8px;padding-top:1em;padding-left: 1em;</xsl:text>
				</xsl:attribute>

				<xsl:value-of disable-output-escaping="yes" select="body"/>
			</div>
		</div>
	</xsl:template>
</xsl:stylesheet>