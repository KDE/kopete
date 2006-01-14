<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="html"/>

<!-- 
Author:   Urs Roesch <urs (a) synthetik dot ch>
Version:  0.5.2
Based on: XChat.xsl (but not much in common at this point)
License:  GPL (http://www.gnu.org/licenses/gpl.txt)

Changelog:
 - 2004-02-29 Urs Roesch
   * Added Changelog :)
   * Rewritten to make use of variables
   * Cleanup
   
 - 2004-03-01 Urs Roesch  
   * template-ized
   * added experimental BiDi support
   * added experimental i18n support
   
 - 2004-03-01 Urs Roesch 
   * Bugfixed contact name   

 - 2004-03-03 Urs Roesch 
   * simplyfied contact name logic
   * added the use-meta-name config option
   
Todo:
   * add more configuration variables
   * test BiDi (need help from 3rd party)
   * test i18n (need help from 3rd party) 

Readme: 
   You can now configure style with xsl variables right below this message.
   Simply change the values to the ones you would like to use and in either
   your favorite text editor or the build in one of Kopete.
   
-->


<!-- User definable variables -->
<xsl:variable name="body-font-weight">bold</xsl:variable>

<!-- set the color of the timestamp -->
<xsl:variable name="time-color">#00FF00</xsl:variable>
<!-- set the font-weight of the timestamp (see the CSS spec for more info) -->
<xsl:variable name="time-font-weight">bold</xsl:variable>

<!-- set the symbol for outgoing messages -->
<xsl:variable name="outgoing-symbol"> $ </xsl:variable>
<!-- set the color of the symbol for outgoing messages -->
<xsl:variable name="outgoing-color">#FF6600</xsl:variable>
<!-- set the font-weight of the symbol for outgoing messages -->
<xsl:variable name="outgoing-font-weight">bold</xsl:variable>

<!-- set the symbol for incoming messages -->
<xsl:variable name="incoming-symbol"> $ </xsl:variable>
<!-- set the color of the symbol for incoming messages --> 
<xsl:variable name="incoming-color">#FFFF00</xsl:variable>
<!-- set the font-weight for the symbol for incoming messages -->
<xsl:variable name="incoming-font-weight">bold</xsl:variable>

<!-- set the symbol for action messages -->
<xsl:variable name="action-symbol"> @ </xsl:variable>
<!-- set the color for the action message symbol -->
<xsl:variable name="action-color">#00FFFF</xsl:variable>
<!-- set the font-weight for the action message symbol -->
<xsl:variable name="action-font-weight">bold</xsl:variable>
<!-- set the font-color of the action messages -->
<xsl:variable name="action-body-color">darkgrey</xsl:variable>

<!-- set the symbol for internal messages -->
<xsl:variable name="internal-symbol"> # </xsl:variable>
<!-- set the color for the internal message symbol -->
<xsl:variable name="internal-color">#FF00FF</xsl:variable>
<!-- set the font-weight for the internal message symbol -->
<xsl:variable name="internal-font-weight">bold</xsl:variable>

<!-- 
	set to yes to display screen name in front of the timestamp seperated 
        by an @ e.g. [screen_name@22:30:59] 
-->
<xsl:variable name="show-screen-name">yes</xsl:variable>
<!-- use meta name instead of screen name -->
<xsl:variable name="use-meta-name">no</xsl:variable>
<!-- set to no to override the kopete background and font-colors -->
<xsl:variable name="use-kopete-colors">yes</xsl:variable>



<!-- eof user definable user variables -->



<!-- ============================================================================================ -->
<!-- Default variables are mine; read keep your greasy paws off ;) -->
<xsl:variable name="default-background">#000000</xsl:variable>
<xsl:variable name="default-foreground">#FFFFFF</xsl:variable>

<xsl:variable name="default-time-color">#00FF00</xsl:variable>
<xsl:variable name="default-time-font-weight">bold</xsl:variable>

<xsl:variable name="default-outgoing-symbol"> $ </xsl:variable>
<xsl:variable name="default-outgoing-color">#FF6600</xsl:variable>
<xsl:variable name="default-outgoing-font-weight">bold</xsl:variable>

<xsl:variable name="default-incoming-symbol"> $ </xsl:variable>
<xsl:variable name="default-incoming-color">#FFFF00</xsl:variable>
<xsl:variable name="default-incoming-font-weight">bold</xsl:variable>

<xsl:variable name="default-action-symbol"> @ </xsl:variable>
<xsl:variable name="default-action-color">#00FFFF</xsl:variable>
<xsl:variable name="default-action-font-weight">bold</xsl:variable>
<xsl:variable name="default-action-body-color">darkgrey</xsl:variable>

<xsl:variable name="default-internal-symbol"> # </xsl:variable>
<xsl:variable name="default-internal-color">#FF00FF</xsl:variable>
<xsl:variable name="default-internal-font-weight">bold</xsl:variable>

<xsl:variable name="test-dir">RTL</xsl:variable>
<xsl:variable name="lc">ltr</xsl:variable>
<xsl:variable name="uc">LTR</xsl:variable>

<!-- let the games begin -->

<xsl:template match="message">


<div class="KopeteMessage">
	<xsl:attribute name="id">
		<xsl:value-of select="@id"/>
	</xsl:attribute>
	<xsl:attribute name="style">
		<xsl:choose>
			<xsl:when test="$use-kopete-colors = 'yes' ">
				<xsl:if test="body/@color">
					color: <xsl:value-of select="body/@color"/>;
				</xsl:if>
  				<xsl:if test="body/@bgcolor">
    					background-color: <xsl:value-of select="body/@bgcolor"/>
  				</xsl:if>
			</xsl:when>
			<xsl:otherwise>
				background-color: <xsl:value-of select="$default-background" />;
				color: <xsl:value-of select="$default-foreground" />;
			</xsl:otherwise>
		</xsl:choose>
  		<xsl:text>
			padding-bottom: 0.2em;
			clear: both;
		</xsl:text>
	</xsl:attribute>
	<!-- 
	<xsl:attribute name="dir">
		<xsl:value-of select="body/@dir" />
	</xsl:attribute>
	-->
	
	
<div> <!-- floating info -->
	<xsl:attribute name="style">
		<xsl:choose>
			<xsl:when test="translate(body/@dir, $uc, $lc) = 'ltr'">
				float: left;
			</xsl:when>
			<xsl:otherwise>
				float: right;
			</xsl:otherwise>
		</xsl:choose>
	</xsl:attribute>
	<!--
	<xsl:attribute name="dir">
		<xsl:value-of select="body/@dir" /> -->
		<!-- <xsl:value-of select="/body/@dir" /> -->
	<!-- </xsl:attribute> -->
	<xsl:if test="translate(body/@dir, $uc, $lc) = 'rtl'">
		<xsl:call-template name="symbol" />
	</xsl:if>
<!-- Time stamp -->
<span><!-- time -->
	<xsl:attribute name="style">
		<xsl:choose>
			<xsl:when test="$time-color">
				color: <xsl:value-of select="$time-color" />;
			</xsl:when>
			<xsl:otherwise>
				color: <xsl:value-of select="$default-time-color" />;
			</xsl:otherwise>
		</xsl:choose>
		<xsl:choose>
			<xsl:when test="$time-font-weight">
				font-weight: <xsl:value-of select="$time-font-weight" />;
			</xsl:when>
			<xsl:otherwise>
				font-weight: <xsl:value-of select="$default-time-font-weight" />;
			</xsl:otherwise>
		</xsl:choose>
	</xsl:attribute>
		<xsl:text>[</xsl:text>
			<xsl:if test="$show-screen-name = 'yes'">
				<xsl:choose>	
					<xsl:when test="$use-meta-name = 'yes'">
						<xsl:value-of select="from/contact/metaContactDisplayName/@text" disable-output-escaping="yes"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:value-of select="from/contact/contactDisplayName/@text" disable-output-escaping="yes"/>
					</xsl:otherwise>
				</xsl:choose>
				<xsl:text>@</xsl:text>
			</xsl:if>
			<xsl:value-of select="@time"/>]
		<!-- <xsl:value-of select="@time"/> -->
	
	<xsl:if test="translate(body/@dir, $uc, $lc) = 'ltr'">
		<xsl:call-template name="symbol" />
	</xsl:if>
</span><!-- eof time -->

<!-- Choose based on message direction -->
</div><!-- eof floating info -->

<div>
	<xsl:attribute name="style">
		<xsl:choose>
			<xsl:when test="translate(body/@dir, $uc, $lc) = 'ltr'">
				text-indent: 0.5em;
				margin-left: 4em;
			</xsl:when>
			<xsl:otherwise>
				text-indent: -0.5em;
				margin-right: 4em;
			</xsl:otherwise>
		</xsl:choose>
	</xsl:attribute>
	<xsl:attribute name="dir">
		<xsl:value-of select="body/@dir" />
		<!-- <xsl:value-of select="/body/@dir" /> -->
	</xsl:attribute>
<span>
	<xsl:attribute name="style">
		<xsl:if test="@direction = '3'"><!-- action message -->
			<xsl:choose>
				<xsl:when test="$action-body-color">
					color: <xsl:value-of select="$action-body-color" />;
				</xsl:when>
				<xsl:otherwise>
					color: <xsl:value-of select="$default-action-body-color" />;
				</xsl:otherwise>
			</xsl:choose>
		</xsl:if>
	</xsl:attribute>
	<xsl:if test="@importance = '2'">
		<xsl:attribute name="class"><xsl:text>KopeteMessage highlight</xsl:text></xsl:attribute>
	</xsl:if>
	<xsl:if test="@direction='3'"><!-- Action -->
		<xsl:value-of select="from/contact/contactDisplayname/@text" disable-output-escaping="yes"/>
	</xsl:if>
	<xsl:value-of disable-output-escaping="yes" select="body"/>
</span>
</div>
</div><!-- eof message -->
</xsl:template>

<xsl:template name="symbol">
<!-- Choose based on message direction -->
<xsl:choose><!-- symbol -->
	<xsl:when test="@direction = '2'"><!--internal message-->
		<xsl:call-template name="display-symbol">
			<xsl:with-param name="symbol"><xsl:value-of select="$internal-symbol" /></xsl:with-param>
			<xsl:with-param name="color"><xsl:value-of select="$internal-color" /></xsl:with-param>
			<xsl:with-param name="font-weight"><xsl:value-of select="$internal-font-weight" /></xsl:with-param>
			<xsl:with-param name="default-color"><xsl:value-of select="$default-internal-color" /></xsl:with-param>
			<xsl:with-param name="default-font-weight"><xsl:value-of select="$default-internal-font-weight" /></xsl:with-param>
		</xsl:call-template>
	</xsl:when><!-- eof internal message -->
	
	<xsl:when test="@direction = '3'"><!--action message-->
		<xsl:call-template name="display-symbol">
			<xsl:with-param name="symbol"><xsl:value-of select="$action-symbol" /></xsl:with-param>
			<xsl:with-param name="color"><xsl:value-of select="$action-color" /></xsl:with-param>
			<xsl:with-param name="font-weight"><xsl:value-of select="$action-font-weight" /></xsl:with-param>
			<xsl:with-param name="default-color"><xsl:value-of select="$default-action-color" /></xsl:with-param>
			<xsl:with-param name="default-font-weight"><xsl:value-of select="$default-action-font-weight" /></xsl:with-param>
		</xsl:call-template>
	</xsl:when><!-- eof action message -->

	<xsl:when test="@direction = '1'"><!-- outgoing -->
		<xsl:call-template name="display-symbol">
			<xsl:with-param name="symbol"><xsl:value-of select="$outgoing-symbol" /></xsl:with-param>
			<xsl:with-param name="color"><xsl:value-of select="$outgoing-color" /></xsl:with-param>
			<xsl:with-param name="font-weight"><xsl:value-of select="$outgoing-font-weight" /></xsl:with-param>
			<xsl:with-param name="default-color"><xsl:value-of select="$default-outgoing-color" /></xsl:with-param>
			<xsl:with-param name="default-font-weight"><xsl:value-of select="$default-outgoing-font-weight" /></xsl:with-param>
		</xsl:call-template>
	</xsl:when><!-- eof outgoing  -->	
	
	<xsl:otherwise><!-- other messages -->
		<xsl:call-template name="display-symbol">
			<xsl:with-param name="symbol"><xsl:value-of select="$incoming-symbol" /></xsl:with-param>
			<xsl:with-param name="color"><xsl:value-of select="$incoming-color" /></xsl:with-param>
			<xsl:with-param name="font-weight"><xsl:value-of select="$incoming-font-weight" /></xsl:with-param>
			<xsl:with-param name="default-color"><xsl:value-of select="$default-incoming-color" /></xsl:with-param>
			<xsl:with-param name="default-font-weight"><xsl:value-of select="$default-incoming-font-weight" /></xsl:with-param>
		</xsl:call-template>
	</xsl:otherwise><!-- eof other messages -->
</xsl:choose><!-- eof symbol -->
</xsl:template>

<xsl:template name="display-symbol">
	<xsl:param name="symbol"><xsl:value-of select="symbol" /></xsl:param>
	<xsl:param name="color"><xsl:value-of select="color" /></xsl:param>
	<xsl:param name="font-weight"><xsl:value-of select="font-weight" /></xsl:param>
	<xsl:param name="default-color"><xsl:value-of select="default-color" /></xsl:param>
	<xsl:param name="default-font-weight"><xsl:value-of select="default-font-weight" /></xsl:param>
	
	<span>
		<xsl:attribute name="style">
			<xsl:choose>
				<xsl:when test="$color">
					color: <xsl:value-of select="$color" />;
				</xsl:when>
				<xsl:otherwise>
					color: <xsl:value-of select="$default-color" />;
				</xsl:otherwise>
			</xsl:choose>
			<xsl:choose>
				<xsl:when test="$font-weight">
					font-weight: <xsl:value-of select="$font-weight" />;
				</xsl:when>
				<xsl:otherwise>
					font-weight: <xsl:value-of select="$default-font-weight" />;
				</xsl:otherwise>
			</xsl:choose>
		</xsl:attribute>
		<xsl:value-of select="$symbol" />
	</span>
</xsl:template>

</xsl:stylesheet>
