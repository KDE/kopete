<?xml version="1.0"?>
<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<!-- NB You will have to create a dir 'images' at the upload location containing the files named below -->
	<xsl:template match="webpresence">
		<html>
			<head><title>My IM Status with images</title></head>
			<body>
				<xsl:value-of select="name"/>
				<xsl:apply-templates select="accounts"/>
				<hr/>
				<font size="-2">
					Last update at:
					<xsl:value-of select="listdate"/>
				</font>
			</body>
		</html>
	</xsl:template>


	<xsl:template match="accounts">
		<table>
			<xsl:apply-templates select="account"/>
		</table>
	</xsl:template>

	<xsl:template match="account">
		<tr>
			<td>
				<xsl:apply-templates select="protocol"/>
			</td>
			<td>
				<xsl:value-of select="accountname"/>
			</td>
			<td>
				<xsl:apply-templates select="accountstatus"/>
			</td>
			<td>
				<xsl:value-of select="accountaddress"/>
			</td>
		</tr>
	</xsl:template>

	<xsl:template match="protocol">
		<xsl:choose>
			<xsl:when test=".='MSNProtocol'">
				<!-- MSN gubbins here -->
				<img src="images/msn_protocol.png"/>
			</xsl:when>
			<xsl:when test=".='ICQProtocol'">
				<!-- ICQ gubbins here -->
				<img src="images/icq_protocol.png"/>
			</xsl:when>
			<xsl:when test=".='JabberProtocol'">
				<!-- Jabber gubbins here -->
				<img src="images/jabber_protocol_32.png"/>
			</xsl:when>
			<xsl:when test=".='YahooProtocol'">
				<!-- Yahoo gubbins here -->
				<img src="images/yahoo_protocol_32.png"/>
			</xsl:when>
			<xsl:when test=".='AIMProtocol'">
				<!-- Oscar gubbins here -->
				<img src="images/aim_protocol.png"/>
			</xsl:when>
			<xsl:when test=".='IRCProtocol'">
				<!-- IRC gubbins here -->
				<img src="images/irc_protocol.png"/>
			</xsl:when>
			<xsl:when test=".='SMSProtocol'">
				<!-- IRC gubbins here -->
				<img src="images/sms_protocol.png"/>
			</xsl:when>
			<xsl:when test=".='GaduProtocol'">
				<!-- IRC gubbins here -->
				<img src="images/gadu_protocol.png"/>
			</xsl:when>
			<xsl:when test=".='WPProtocol'">
				<!-- WinPopup gubbins here -->
				<img src="images/winpopup_protocol.png"/>
			</xsl:when>
		</xsl:choose>
	</xsl:template>

	<xsl:template match="accountstatus">
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
		</xsl:choose>
	</xsl:template>

</xsl:stylesheet>
