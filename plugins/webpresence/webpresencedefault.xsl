<?xml version="1.0"?>
<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

	<xsl:template match="contacts">
		<html>
			<head><title>My IM Status</title></head>
			<body>
				<table>
					<xsl:apply-templates select="contact"/>
				</table>
			</body>
		</html>
	</xsl:template>

	<xsl:template match="contact">
		<tr>
			<td>
				<xsl:value-of select="name"/>
			</td>
			<td>
				<xsl:apply-templates select="protocols"/>
			</td>
		</tr>
	</xsl:template>

	<xsl:template match="protocols">
		<table>
			<xsl:apply-templates select="protocol"/>
		</table>
	</xsl:template>

	<xsl:template match="protocol">
		<tr>
			<td>
				<xsl:apply-templates select="protoname"/>
			</td>
			<td>
				<xsl:apply-templates select="protostatus"/>
			</td>
			<td>
				<xsl:value-of select="protoaddress"/>
			</td>
		</tr>
	</xsl:template>
	
	<xsl:template match="protoname">
		<xsl:choose>
			<xsl:when test=".='MSNProtocol'">
				<!-- MSN gubbins here -->
				MSN 
			</xsl:when>
			<xsl:when test=".='ICQProtocol'">
				<!-- ICQ gubbins here -->
				ICQ
			</xsl:when>
			<xsl:when test=".='JabberProtocol'">
				<!-- Jabber gubbins here -->
				Jabber
			</xsl:when>
			<xsl:when test=".='YahooProtocol'">
				<!-- Yahoo gubbins here -->
				Yahoo
			</xsl:when>
			<xsl:when test=".='OscarProtocol'">
				<!-- Oscar gubbins here -->
				AIM
			</xsl:when>
			<xsl:when test=".='GaduProtocol'">
				<!-- Gadu-gadu gubbins here -->
				Gadu-gadu
			</xsl:when>
			<xsl:when test=".='WPProtocol'">
				<!-- WinPopup gubbins here -->
				WinPopup
			</xsl:when>
			<xsl:when test=".='SMSProtocol'">
				<!-- SMS gubbins here -->
				SMS
			</xsl:when>
			<xsl:otherwise>
				<!-- default case -->
				Unknown protocol, amend stylesheet
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template match="protostatus">
		<xsl:choose>
			<xsl:when test=".='ONLINE'">
				<font color="#00FF00">
					<xsl:value-of select="."/>
				</font>
			</xsl:when>
			<xsl:when test=".='OFFLINE'">
				<font color="#FF0000">
					<xsl:value-of select="."/>
				</font>
			</xsl:when>
			<xsl:when test=".='AWAY'">
				<font color="#FFFF00">
					<xsl:value-of select="."/>
				</font>
			</xsl:when>
			<xsl:when test=".='UNKNOWN'">
				<font color="#CCCCCC">
					<xsl:value-of select="."/>
				</font>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="."/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

</xsl:stylesheet>
