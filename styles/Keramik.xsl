<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="html"/>
	<xsl:template match="message">
		<div class="KopeteMessage">
			<xsl:attribute name="id"><xsl:value-of select="@id"/></xsl:attribute>
			<xsl:attribute name="style">
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
						<img width="16" height="16" align="center" border="0" style="margin-right:6px;">
							<xsl:attribute name="src">
								<xsl:value-of select="to/contact/@protocolIcon"/>
							</xsl:attribute>
						</img>
						<xsl:text>To </xsl:text>
						<span>
							<xsl:attribute name="title"><xsl:value-of disable-output-escaping="yes" select="to/contact/@contactId"/></xsl:attribute>
							<xsl:value-of disable-output-escaping="yes" select="to/contact/@contactDisplayName"/>
						</span>
						<xsl:text> at </xsl:text>
						<xsl:value-of select="@time"/>
					</xsl:when>
					<xsl:otherwise><!-- Incoming / Action message -->
						<img width="16" height="16" align="center" border="0" style="margin-right:6px;">
							<xsl:attribute name="src">
								<xsl:value-of select="from/contact/@protocolIcon"/>
							</xsl:attribute>
						</img>
						<xsl:text>From </xsl:text>
						<span>
							<xsl:attribute name="title"><xsl:value-of disable-output-escaping="yes" select="from/contact/@contactId"/></xsl:attribute>
							<xsl:value-of disable-output-escaping="yes" select="from/contact/@contactDisplayName"/>
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
					<xsl:text>margin-top:8px;padding:6px;</xsl:text>
					<xsl:if test="@direction='3'">
						<xsl:text>color:darkgreen;</xsl:text>
					</xsl:if>
				</xsl:attribute>
				<xsl:if test="@direction='3'"><!-- Action -->
					<xsl:text>*</xsl:text>
					<span>
						<xsl:attribute name="title"><xsl:value-of disable-output-escaping="yes" select="from/contact/@contactId"/></xsl:attribute>
						<xsl:value-of disable-output-escaping="yes" select="from/contact/@contactDisplayName"/>
					</span>
					<xsl:text>&#160;</xsl:text>
				</xsl:if>
				<xsl:value-of disable-output-escaping="yes" select="body"/>
			</div>
		</div>
		<div>
			<xsl:attribute name="style">
				<xsl:text>height:10px;</xsl:text>
			</xsl:attribute>
		</div>

	</xsl:template>
</xsl:stylesheet>
