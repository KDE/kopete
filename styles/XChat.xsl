<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="html"/>
	<xsl:template match="message">
		<div class="KopeteMessage">
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
				</xsl:if>
			</xsl:attribute>

			<xsl:if test="@direction='3'">
				<xsl:attribute name="style">
					<xsl:text>color:darkgreen</xsl:text>
				</xsl:attribute>
			</xsl:if>
			[<xsl:value-of select="@time"/>]
			<!-- Choose based on message direction -->
			<xsl:choose>
				<xsl:when test="@direction='2'">
					<!--internal message-->
					-
					<font color="cyan">--</font>
				</xsl:when>
				<xsl:when test="@direction='3'">
					<!--action message-->
					<span>
						<xsl:attribute name="dir">
							<xsl:value-of select="from/contact/contactDisplayName/@dir"/>
						</xsl:attribute>
						<xsl:attribute name="title">
							<xsl:value-of disable-output-escaping="yes" select="from/contact/@contactId"/>
						</xsl:attribute>
						<xsl:value-of disable-output-escaping="yes" select="from/contact/contactDisplayName/@text"/>
					</span>
				</xsl:when>
				<xsl:otherwise>
					<font color="blue">&lt;</font>
					<font>
						<xsl:attribute name="color">
							<xsl:choose>
								<xsl:when test="@direction='1'">
									<!-- Outgoing -->
									<xsl:text>yellow</xsl:text>
								</xsl:when>
								<xsl:otherwise>
									<!-- Incoming -->
									<xsl:value-of select="from/contact/@color"/>
								</xsl:otherwise>
							</xsl:choose>
						</xsl:attribute>
						<span>
							<xsl:attribute name="title">
								<xsl:value-of disable-output-escaping="yes" select="from/contact/@contactId"/>
							</xsl:attribute>
							<xsl:attribute name="dir">
								<xsl:value-of select="from/contact/contactDisplayName/@dir"/>
							</xsl:attribute>
							<xsl:value-of disable-output-escaping="yes" select="from/contact/contactDisplayName/@text"/>
						</span>
					</font>
					<font color="blue">&gt;</font>
				</xsl:otherwise>
			</xsl:choose>
			<span style="margin-left:0.5em;">
				<xsl:attribute name="dir">
					<xsl:value-of select="body/@dir"/>
				</xsl:attribute>
				<xsl:if test="@importance='2'">
					<xsl:attribute name="class">
						<xsl:text>KopeteMessage highlight</xsl:text>
					</xsl:attribute>
				</xsl:if>
				<xsl:value-of disable-output-escaping="yes" select="body"/>
			</span>
		</div>
	</xsl:template>
</xsl:stylesheet>

