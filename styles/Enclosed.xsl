<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="html"/>
	<xsl:template match="message">
		<div class="KopeteMessage" style="padding-bottom:5px;">
			<xsl:attribute name="id">
				<xsl:value-of select="@id"/>
			</xsl:attribute>
			<div style="border:1px solid grey;padding:1px;">
				<div>
					<xsl:choose>
						<xsl:when test="@direction='3'"><!-- action message -->
							<xsl:attribute name="style">
								<xsl:text>color:red;font-weight:bold;</xsl:text>
							</xsl:attribute>
							Message from 
							<span class="KopeteDisplayName">
								<xsl:value-of disable-output-escaping="yes" select="from/contact/@metaContactDisplayName"/>
							</span>
							(<xsl:value-of disable-output-escaping="yes" select="from/contact/@contactId"/>)
						</xsl:when>
						<xsl:when test="@direction='2'"><!-- internal message -->
							<xsl:attribute name="style">
								<xsl:text>color:red;font-weight:bold;</xsl:text>
							</xsl:attribute>
							System Message
						</xsl:when>
						<xsl:when test="@direction='1'"><!-- Outgoing -->
							<xsl:attribute name="style">
								<xsl:text>color:red;font-weight:bold;</xsl:text>
							</xsl:attribute>
							Message to 
							<span class="KopeteDisplayName">
								<xsl:value-of disable-output-escaping="yes" select="to/contact/@metaContactDisplayName"/>
							</span>
							(from <xsl:value-of disable-output-escaping="yes" select="from/contact/@contactId"/>)
						</xsl:when>
						<xsl:otherwise><!-- Incoming -->
							<xsl:attribute name="style">
								<xsl:text>color:blue;font-weight:bold;</xsl:text>
							</xsl:attribute>
							Message from
							<span class="KopeteDisplayName">
								<xsl:value-of disable-output-escaping="yes" select="from/contact/@metaContactDisplayName"/>
							</span>
							(<xsl:value-of disable-output-escaping="yes" select="from/contact/@contactId"/>)
						</xsl:otherwise>
					</xsl:choose>
				</div>
				<div style="text-align:right;margin-top:-1em;float:right;">
					<xsl:value-of select="@time"/>
				</div>
				<div>
					<xsl:attribute name="style">
						<xsl:text>padding-left:15px;padding-right:15px;</xsl:text>
						<xsl:if test="body/@color"><xsl:text>color:</xsl:text>
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
						<xsl:when test="@direction='3'"><!--action message-->
							<span style="color:darkgreen">
								<xsl:text>* </xsl:text>
								<span class="KopeteDisplayName">
									<xsl:value-of disable-output-escaping="yes" select="from/contact/@metaContactDisplayName"/>
								</span>
									<xsl:text> </xsl:text>
								<xsl:value-of disable-output-escaping="yes" select="body"/>
							</span>
						</xsl:when>
						<xsl:when test="@direction='2'"><!--internal message-->
							<span style="color:darkviolet;font-weight:bold;">
								<xsl:value-of disable-output-escaping="yes" select="body"/>
							</span>
						</xsl:when>
						<xsl:otherwise>
							<xsl:value-of disable-output-escaping="yes" select="body"/>
						</xsl:otherwise>
					</xsl:choose>
				</div>
			</div>
		</div>
	</xsl:template>
</xsl:stylesheet>
