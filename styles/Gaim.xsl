<?xml version="1.0" encoding="UTF-8"?>

<!--

This is a Kopete messages stylesheet
Made by kanuso (kanuso@kanuso.net)

Nothing is original :P
The theme is mainly based on:
- Gaim's style and colors
- XChat and Kopete stylesheets' code (so I suppose this is GPL code)

-->

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="html"/>
<xsl:template match="message">
<div class="KopeteMessage">

<xsl:attribute name="id">
	<xsl:value-of select="@id"/>
</xsl:attribute>

<!-- First, decide the "full line" style -->
<xsl:attribute name="style">
		<!-- decide font color by message type -->
		<xsl:choose>
			<!-- internal message -->
			<xsl:when test="@direction='2'">
				<xsl:text>color:darkviolet;</xsl:text>
			</xsl:when>
 			<!-- action message -->
			<xsl:when test="@direction='3'">
				<xsl:text>color:darkgreen;</xsl:text>
			</xsl:when>
			<!-- incoming/outgoing message -->
			<xsl:otherwise>
				<xsl:if test="body/@color">
					<xsl:text>color:</xsl:text>
					<xsl:value-of select="body/@color"/>
					<xsl:text>;</xsl:text>
				</xsl:if>
			</xsl:otherwise>
		</xsl:choose>
		<!-- if there si a bgcolor, apply it -->
		<xsl:if test="body/@bgcolor">
			<xsl:text>background-color:</xsl:text>
			<xsl:value-of select="body/@bgcolor"/>
			<xsl:text>;</xsl:text>
		</xsl:if>
</xsl:attribute>

<!-- Decide the Pre-body output by message type -->
<xsl:choose>
	<!--internal message-->
	<xsl:when test="@direction='2'">
		<span class="KopeteDisplayTimestamp">(<xsl:value-of select="@time"/>) </span>
		<span style="font-weight:bold;"><xsl:text disable-output-escaping="yes">#</xsl:text></span>
	</xsl:when>
	<!-- action message -->
	<xsl:when test="@direction='3'">
		<span class="KopeteDisplayTimestamp">(<xsl:value-of select="@time"/>) </span>
		<span class="KopeteDisplayName" style="font-weight:bold;">
			<xsl:attribute name="dir">
				<xsl:value-of select="from/contact/metaContactDisplayName/@dir"/>
			</xsl:attribute>
			<xsl:value-of disable-output-escaping="yes" select="from/contact/metaContactDisplayName/@text"/>
		</span>
	</xsl:when>
	<!-- incoming/outgoing messages -->
	<xsl:otherwise>
		<span>
			<xsl:attribute name="style">
				<xsl:choose>
					<!-- outgoing -->
					<xsl:when test="@direction='1'">
						color:<xsl:text>#16569E</xsl:text>
					</xsl:when>
					<!-- incoming -->
					<xsl:otherwise>
						color:<xsl:text>#A82F2F</xsl:text>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:attribute>
			<span class="KopeteDisplayTimestamp">(<xsl:value-of select="@time"/>) </span>
			<span class="KopeteDisplayName" style="font-weight:bold;">
				<xsl:attribute name="dir">
	                                <xsl:value-of select="from/contact/metaContactDisplayName/@dir"/>
	                        </xsl:attribute>
				<xsl:value-of disable-output-escaping="yes" select="from/contact/metaContactDisplayName/@text"/>:
			</span>
		</span>
	</xsl:otherwise>
</xsl:choose>

<!-- Body output -->
<span>
<xsl:if test="@importance='2'">
	<xsl:attribute name="class"><xsl:text>KopeteMessage highlight</xsl:text></xsl:attribute>
</xsl:if>
<xsl:attribute name="dir">
	<xsl:value-of select="body/@dir"/>
</xsl:attribute>
&#160;<xsl:value-of disable-output-escaping="yes" select="body"/>
</span>

</div>
</xsl:template>
</xsl:stylesheet>
