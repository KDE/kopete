<?xml version="1.0" encoding="UTF-8"?>
<!--
  'Kopete' theme for Kopete
  Copyright (C) 2005:
  - Johann Ollivier Lapeyre <johann.ollivierlapeyre@gmail.com>
  - Original "Clean" theme, Jonas Lihnell <gg02-jli@mail2.nti.se> 

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="html"/>
	<xsl:template match="message">

		<div class="KopeteMessage" style="margin:1.4em .2em 0 .2em;">
			<xsl:choose>
				<xsl:when test="@route='inbound'">

					<!-- Incoming normal messages -->
					<xsl:if test="@type='normal'">
							<!-- Header -->
							<div id="MessageHeader" style="">
							<xsl:attribute name="style">
							background-color:#dfedff;
							padding:.1em;
							border:solid;
							border-color:#fafafa #d1dfef #d1dfef #fafafa;
							border-width:2px;
							</xsl:attribute>
								<div>
								<xsl:attribute name="style">
								float:left;
								</xsl:attribute>
								<xsl:if test="from/contact/@userPhoto">
									<img>
										<xsl:attribute name="src">
										data:image/png;base64,<xsl:value-of select="from/contact/@userPhoto"/>;
										</xsl:attribute>
										<xsl:attribute name="style">
										border:1px solid #888;
										height:48px; 
										margin-top: 0.2em;
										margin-left: 0.2em;
										margin-right: 1ex;
										</xsl:attribute>
									</img>
								</xsl:if>
								</div>
								<!-- Protocol Icon -->
								<div style="float: left;">
									<img style="margin-top: 0.1em; margin-right: 1ex;"><xsl:attribute name="src"><xsl:value-of select="from/contact/@protocolIcon"/></xsl:attribute></img>
								</div>
								<div>
								<xsl:attribute name="style">
								float:right;
								</xsl:attribute>
									<xsl:value-of select="@time"/>
								</div>
								<div>
								<xsl:attribute name="style">
								<xsl:if test="from/contact/@userPhoto">
									margin-left:60px;
								</xsl:if>
								</xsl:attribute>
									<span>
									<xsl:attribute name="style">
									font-weight:bold;
									<!-- Set username to leftalign regardless of language-direction -->
									<xsl:if test="body/@dir='ltr'">text-align: left;</xsl:if>
									<xsl:if test="body/@dir='rtl'">text-align: left;</xsl:if>
									</xsl:attribute>
										<xsl:value-of select="from/contact/metaContactDisplayName/@text" />&#160;
									</span>
									
								</div>
							</div>

							<div id="MessageBody">

								<!-- Highlighted Importance -->
								<xsl:attribute name="class">
									<xsl:if test="@importance='2'">highlight</xsl:if>
								</xsl:attribute>

								<xsl:attribute name="style">
									padding-left: 1ex;
									padding-right: 1ex;
									padding-top: 0.25em;
									padding-bottom: 0.5em;
									line-height: 1.2;

									<!-- Align body text according to language direction -->
									<xsl:if test="body/@dir='ltr'">text-align: left;</xsl:if>
									<xsl:if test="body/@dir='rtl'">text-align: right;</xsl:if>

									<!-- Colored Message -->
									<xsl:if test="body/@color">color: <xsl:value-of select="body/@color"/>;</xsl:if>
									<xsl:if test="body/@bgcolor">background-color: <xsl:value-of select="body/@bgcolor"/>;</xsl:if>
									<xsl:if test="body/@font">font: <xsl:value-of select="body/@font"/>;</xsl:if>

									<!-- Normal Importance -->
									<xsl:if test="@importance='1'"></xsl:if>

								</xsl:attribute>

								<xsl:value-of disable-output-escaping="yes" select="body" />
							</div>
					</xsl:if>

					<!-- Incoming actions -->
					<xsl:if test="@type='action'">
							<div id="MessageHeader" style="border-top: 2px solid #dae5f0; 
									border-right: 2px solid #aaccf0;
									border-bottom: 2px solid #aaccf0;
									border-left: 2px solid #dae5f0;
									padding: 0.1em; 
									vertical-align: middle;
									background-color:#c3d9f0;">

								<!-- Action Icon -->
								<div style="float: left;">
									<img style="margin-right: 1ex;"><xsl:attribute name="src"><xsl:value-of select="$appdata"/>Kopete/action.png</xsl:attribute></img>
								</div>

								<!-- MetaContacts Display Name && Messages Timestamp -->
								<div>
									<span style="font-weight: bold;"><xsl:value-of select="from/contact/metaContactDisplayName/@text" /></span>
									<span style="margin-left: 1ex;"><xsl:value-of disable-output-escaping="yes" select="body" /></span>
									<div style="float: right; font-weight: normal;"><xsl:value-of select="@time" /></div>
								</div>
							</div>
					</xsl:if>

				</xsl:when>
				<xsl:when test="@route='outbound'">

					<!-- Outgoing normal messages -->
					<xsl:if test="@type='normal'">

							<div id="MessageHeader" style="">
							<xsl:attribute name="style">
							background-color:#f5f5f5;
							padding:.1em;
							border:solid;
							border-color:#fafafa #e3e3e3 #e3e3e3 #fafafa;
							border-width:2px;
							</xsl:attribute>
								<div>
								<xsl:attribute name="style">
								float:left;
								</xsl:attribute>
								<xsl:if test="from/contact/@userPhoto">
									<img>
										<xsl:attribute name="src">
										data:image/png;base64,<xsl:value-of select="from/contact/@userPhoto"/>;
										</xsl:attribute>
										<xsl:attribute name="style">
										border:1px solid #888;
										height:48px;
										margin-top: 0.2em;
										margin-left: 0.2em;
										margin-right: 1ex;
										</xsl:attribute>
									</img>
								</xsl:if>
								</div>
								<!-- Protocol Icon -->
								<div style="float: left;">
									<img style="margin-top: 0.1em; margin-right: 1ex;"><xsl:attribute name="src"><xsl:value-of select="from/contact/@protocolIcon"/></xsl:attribute></img>
								</div>
								<div>
								<xsl:attribute name="style">
								float:right;
								</xsl:attribute>
									<xsl:value-of select="@time"/>
								</div>
								<div>
								<xsl:attribute name="style">
								<xsl:if test="from/contact/@userPhoto">
									margin-left:60px;
								</xsl:if>
								</xsl:attribute>
									<span>
									<xsl:attribute name="style">
									font-weight:bold;
									<!-- Set username to leftalign regardless of language-direction -->
									<xsl:if test="body/@dir='ltr'">text-align: left;</xsl:if>
									<xsl:if test="body/@dir='rtl'">text-align: left;</xsl:if>
									</xsl:attribute>
										<xsl:value-of select="from/contact/metaContactDisplayName/@text" />&#160;
									</span>
									
								</div>
							</div>

							<div id="MessageBody">

								<!-- Highlighted Importance -->
								<xsl:attribute name="class">
									<xsl:if test="@importance='2'">highlight</xsl:if>
								</xsl:attribute>

								<xsl:attribute name="style">
									padding-left: 1ex;
									padding-right: 1ex;
									padding-top: 0.25em;
									padding-bottom: 0.5em;
									line-height: 1.2;

									<!-- Align body text according to language direction -->
									<xsl:if test="body/@dir='ltr'">text-align: left;</xsl:if>
									<xsl:if test="body/@dir='rtl'">text-align: right;</xsl:if>

									<!-- Colored Message -->
									<xsl:if test="body/@color">color: <xsl:value-of select="body/@color"/>;</xsl:if>
									<xsl:if test="body/@bgcolor">background-color: <xsl:value-of select="body/@bgcolor"/>;</xsl:if>
									<xsl:if test="body/@font">font: <xsl:value-of select="body/@font"/>;</xsl:if>

									<!-- Normal Importance -->
									<xsl:if test="@importance='1'"></xsl:if>

								</xsl:attribute>

								<xsl:value-of disable-output-escaping="yes" select="body" />
							</div>
					</xsl:if>

					<!-- Outgoing actions -->
					<xsl:if test="@type='action'">
							<div id="MessageHeader" style="border-top: 2px solid fafafa #cfcfcf; 
									border-right: 2px solid fafafa #afafaf;
									border-bottom: 2px solid fafafa #afafaf;
									border-left: 2px solid fafafa #cfcfcf;
									padding: 0.1em; 
									vertical-align: middle;
									background-color:#dedede;">

								<!-- Action Icon -->
								<div style="float: left;">
									<img style="margin-right: 1ex;"><xsl:attribute name="src"><xsl:value-of select="$appdata"/>Kopete/action.png</xsl:attribute></img>
								</div>

								<!-- MetaContacts Display Name && Messages Timestamp -->
								<div>
									<span style="font-weight: bold;"><xsl:value-of select="from/contact/metaContactDisplayName/@text" /></span>
									<span style="margin-left: 1ex;"><xsl:value-of disable-output-escaping="yes" select="body" /></span>
									<div style="float: right; font-weight: normal;"><xsl:value-of select="@time" /></div>
								</div>
							</div>
					</xsl:if>

				</xsl:when>

				<xsl:when test="@route='internal'">
					<div id="MessageHeader" style="	border-top: 0.1em dashed #afafaf; 
									border-right: 0.1em dashed #afafaf;
									border-bottom: 0.1em dashed #afafaf;
									border-left: 0.1em dashed #afafaf;
									padding-left: 0.1em; 
									padding-bottom: 0.1em; 
									padding-right: 0.1em; 
									vertical-align: middle;">

						<!-- Internal Icon -->
						<div style="float: left;">
							<img style="margin-right: 1ex;"><xsl:attribute name="src"><xsl:value-of select="$appdata"/>Kopete/system.png</xsl:attribute></img>
						</div>

						<!-- Internal Message Display && Messages Timestamp -->
						<div>
							<span>
							<xsl:attribute name="style">
							text-align: left; font-size: 10px; font-weight: bold; color: #808080;
							</xsl:attribute>
							<xsl:value-of disable-output-escaping="yes" select="body" /></span>
							<div style="float: right; font-weight: normal;"><xsl:value-of select="@time" /></div>
						</div>
					</div>
				</xsl:when>

			</xsl:choose>
		</div>

	</xsl:template>
</xsl:stylesheet>

