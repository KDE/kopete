<?xml version="1.0" encoding="UTF-8"?>
<?Kopete TransformAllMessages?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="html"/>

	<xsl:template match="document">
		<xsl:variable name="messages" select="message"/>
		<xsl:for-each select="message">
			<xsl:call-template name="processMessage">
				<xsl:with-param name="messages" select="$messages"/>
			</xsl:call-template>
		</xsl:for-each>
	</xsl:template>

	<xsl:template name="processMessage">
		<xsl:param name="messages"/>
		<xsl:variable name="position" select="position()"/>
		<xsl:variable name="prev" select="$messages[position()=$position - 1]"/>
		<xsl:variable name="curr" select="$messages[position()=$position]"/>
		<xsl:variable name="prevId" select="$prev/from/contact/@contactId"/>
		<xsl:variable name="currId" select="$curr/from/contact/@contactId"/>
		<xsl:if test="not($prevId=$currId)">
			<table style="width:100%">
				<tr>
					<td style="vertical-align: top;">
						<img style="vertical-align: top; padding-top: 5px;">
							<xsl:attribute name="src">
								<xsl:value-of select="$curr/from/contact/@protocolIcon"/>
							</xsl:attribute>
						</img>
					</td>
					<td style="width:100%">
						<table style="width:100%">
							<tr>
								<td style="align:left">
									<span>
										<xsl:attribute name="style">
											color: <xsl:value-of select="$curr/from/contact/@color"/>;
										</xsl:attribute>
										<b><xsl:value-of select="$curr/from/contact/metaContactDisplayName/@text" disable-output-escaping="yes"/></b>
										<xsl:if test="$curr/from/contact/metaContactDisplayName/@text != $curr/from/contact/contactDisplayName/@text">
											<i> (<xsl:value-of select="$curr/from/contact/contactDisplayName/@text" disable-output-escaping="yes"/>)</i>
										</xsl:if>
									</span>
								</td>
								<td style="text-align: right; color: gray"><xsl:value-of select="$curr/@formattedTimestamp"/></td>
							</tr>
						</table>
						<div>
							<xsl:attribute name="style">
								border: medium solid <xsl:value-of select="$curr/from/contact/@color"/>;
								background-color: <xsl:call-template name="lightenColour"><xsl:with-param name="colour" select="$curr/from/contact/@color"/></xsl:call-template>;
								padding: 5px;
								margin: 0;
							</xsl:attribute>
							<xsl:call-template name="renderFirstMessage"/>
							<xsl:call-template name="renderRest">
								<xsl:with-param name="messages" select="$messages"/>
								<xsl:with-param name="withId" select="$currId"/>
								<xsl:with-param name="position" select="$position + 1"/>
							</xsl:call-template>
						</div>
					</td>
				</tr>
			</table>
		</xsl:if>
	</xsl:template>

	<xsl:template name="renderRest">
		<xsl:param name="messages"/>
		<xsl:param name="withId"/>
		<xsl:param name="position"/>
		<xsl:variable name="curr" select="$messages[position()=$position]"/>
		<xsl:variable name="currId" select="$curr/from/contact/@contactId"/>
		<xsl:if test="$withId = $currId">
			<!-- call-template doesn't support select=. fake it. -->
			<xsl:for-each select="$curr">
				<xsl:call-template name="renderLaterMessage"/>
			</xsl:for-each>
			<xsl:call-template name="renderRest">
				<xsl:with-param name="messages" select="$messages"/>
				<xsl:with-param name="withId" select="$currId"/>
				<xsl:with-param name="position" select="$position + 1"/>
			</xsl:call-template>
		</xsl:if>
	</xsl:template>

	<xsl:template name="renderFirstMessage">
		<xsl:call-template name="renderMessageBody"/>
	</xsl:template>

	<xsl:template name="renderLaterMessage">
		<hr>
			<xsl:attribute name="style">
				border: thin dashed <xsl:value-of select="from/contact/@color"/>;
				margin: 0;
			</xsl:attribute>
		</hr>
		<xsl:call-template name="renderMessageBody"/>
	</xsl:template>

	<xsl:template name="renderMessageBody">
		<xsl:choose>
			<xsl:when test="@type='action'">
				<i><xsl:value-of select="from/contact/contactDisplayName/@text" disable-output-escaping="yes"/><xsl:text> </xsl:text><xsl:value-of select="body" disable-output-escaping="yes"/></i>
			</xsl:when>
			<xsl:when test="@route='internal'">
				<span style="font-weight: bold; font-size: xx-small;">
					<xsl:value-of disable-output-escaping="yes" select="body"/>
				</span>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of disable-output-escaping="yes" select="body"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template name="lightenColour">
		<xsl:param name="colour"/>
		<xsl:value-of select="translate($colour,'0123456789AaBbCcDdEeFf','8899aabbccddddeeeeffff')"/>
	</xsl:template>
</xsl:stylesheet>

