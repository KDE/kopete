<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns="http://www.w3.org/1999/xhtml">

	<!--
		XHTML 1.0 Strict - protocol text replaced with icons

		NOTE: You will have to create a dir 'images' at the
		upload location containing the files named below.
	-->

	<!--
		Import the XHTML XSL sheet, and replace the <protocol>
		handling with our own template.
	-->

	<xsl:import href="webpresence_xhtml.xsl"/>

	<!--
		You can change the image directory with this variable.
	-->

	<xsl:variable name="images">images</xsl:variable>

	<xsl:template match="protocol">
		<xsl:choose>
			<xsl:when test=".='MSNProtocol'">
				<img src="{$images}/msn_protocol.png" alt="MSN" title="MSN"/>
			</xsl:when>
			<xsl:when test=".='ICQProtocol'">
				<img src="{$images}/icq_protocol.png" alt="ICQ" title="ICQ"/>
			</xsl:when>
			<xsl:when test=".='JabberProtocol'">
				<img src="{$images}/jabber_protocol.png" alt="Jabber" title="Jabber"/>
			</xsl:when>
			<xsl:when test=".='YahooProtocol'">
				<img src="{$images}/yahoo_protocol.png" alt="Yahoo" title="Yahoo"/>
			</xsl:when>
			<xsl:when test=".='AIMProtocol'">
				<img src="{$images}/aim_protocol.png" alt="AIM" title="AIM"/>
			</xsl:when>
			<xsl:when test=".='IRCProtocol'">
				<img src="{$images}/irc_protocol.png" alt="IRC" title="IRC"/>
			</xsl:when>
			<xsl:when test=".='SMSProtocol'">
				<img src="{$images}/sms_protocol.png" alt="SMS" title="SMS"/>
			</xsl:when>
			<xsl:when test=".='GaduProtocol'">
				<img src="{$images}/gadu_protocol.png" alt="Gadu-Gadu" title="Gadu-Gadu"/>
			</xsl:when>
			<xsl:when test=".='WPProtocol'">
				<img src="{$images}/winpopup_protocol.png" alt="WinPopup" title="WinPopup"/>
			</xsl:when>
		</xsl:choose>
	</xsl:template>

</xsl:stylesheet>

<!-- vim: set ts=4 sts=4 sw=4: -->
