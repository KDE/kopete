<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns="http://www.w3.org/1999/xhtml">

	<!--
		XHTML 1.0 Strict
	-->

	<xsl:output
		doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN"
		doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
		indent="yes"
		encoding="UTF-8"/>

	<xsl:template match="webpresence">
		<html>
			<head>
				<title>My IM Status</title>
			</head>
			<body>
				<p class="name"><xsl:value-of select="name"/></p>
				<xsl:apply-templates select="accounts"/>
				<hr/>
				<p style="font-size: x-small">
					<xsl:text>Last update at: </xsl:text>
					<xsl:value-of select="listdate"/>
				</p>
			</body>
		</html>
	</xsl:template>

	<xsl:template match="accounts">
		<table>
			<xsl:for-each select="account">
			<tr>
				<td class="protocol">
					<xsl:apply-templates select="protocol"/>
				</td>
				<td class="accountname">
					<xsl:value-of select="accountname"/>
				</td>
				<td class="accountstatus">
					<xsl:apply-templates select="accountstatus"/>
				</td>
				<td class="accountaddress">
					<xsl:value-of select="accountaddress"/>
				</td>
			</tr>
			</xsl:for-each>
		</table>
	</xsl:template>

	<xsl:template match="protocol">
		<xsl:choose>
			<xsl:when test=".='AIMProtocol'">
				<xsl:text>AIM</xsl:text>
			</xsl:when>
			<xsl:when test=".='MSNProtocol'">
				<xsl:text>MSN</xsl:text>
			</xsl:when>
			<xsl:when test=".='ICQProtocol'">
				<xsl:text>ICQ</xsl:text>
			</xsl:when>
			<xsl:when test=".='JabberProtocol'">
				<xsl:text>Jabber</xsl:text>
			</xsl:when>
			<xsl:when test=".='YahooProtocol'">
				<xsl:text>Yahoo</xsl:text>
			</xsl:when>
			<xsl:when test=".='GaduProtocol'">
				<xsl:text>Gadu-Gadu</xsl:text>
			</xsl:when>
			<xsl:when test=".='WPProtocol'">
				<xsl:text>WinPopup</xsl:text>
			</xsl:when>
			<xsl:when test=".='SMSProtocol'">
				<xsl:text>SMS</xsl:text>
			</xsl:when>
			<xsl:when test=".='IRCProtocol'">
				<xsl:text>IRC</xsl:text>
			</xsl:when>
			<xsl:otherwise>
				<xsl:text>Unknown</xsl:text>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template match="accountstatus">
		<xsl:choose>
			<xsl:when test=".='ONLINE'">
				<span style="color: lime">
					<xsl:value-of select="."/>
				</span>
			</xsl:when>
			<xsl:when test=".='OFFLINE'">
				<span style="color: red">
					<xsl:value-of select="."/>
				</span>
			</xsl:when>
			<xsl:when test=".='AWAY'">
				<span style="color: maroon">
					<xsl:value-of select="."/>
				</span>
				<xsl:if test="@statusdescription != 'Away' or @awayreason">
					<xsl:text> (</xsl:text>
				</xsl:if>
				<xsl:if test="@statusdescription != 'Away'">
					<xsl:value-of select="@statusdescription"/>
					<xsl:if test="@awayreason">
						<xsl:text>: </xsl:text>
					</xsl:if>
				</xsl:if>
				<xsl:if test="@awayreason">
					<xsl:value-of select="@awayreason"/>
				</xsl:if>
				<xsl:if test="@statusdescription != 'Away' or @awayreason">
					<xsl:text>)</xsl:text>
				</xsl:if>
			</xsl:when>
			<xsl:when test=".='UNKNOWN'">
				<span style="color: gray">
					<xsl:value-of select="."/>
				</span>
				<xsl:if test="@statusdescription">
					<xsl:text> (</xsl:text>
					<xsl:value-of select="@statusdescription"/>
					<xsl:text>)</xsl:text>
				</xsl:if>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="."/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

</xsl:stylesheet>

<!-- vim: set ts=4 sts=4 sw=4: -->
