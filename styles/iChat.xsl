<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="html"/>
  <xsl:param name="appdata"/>
  
  <!-- You need a trailing slash on directories for the following 5 options -->
  <!-- Change iChat to iChat-Trans for the transparent images -->
  <xsl:variable name="image-location"><xsl:value-of select="$appdata"/>/iChat-Trans/</xsl:variable>
  <xsl:variable name="from-color-scheme">graphite/</xsl:variable>
  <xsl:variable name="to-color-scheme">blue/</xsl:variable>
  <xsl:variable name="action-color-scheme">purple/</xsl:variable>
  <xsl:variable name="system-color-scheme">clear/</xsl:variable>
  <xsl:variable name="highlight-color-scheme">yellow/</xsl:variable>
  
  <xsl:variable name="allow-user-colors">yes</xsl:variable>
  <xsl:variable name="public-names">yes</xsl:variable>
  <xsl:variable name="show-timestamps">yes</xsl:variable>

  <!-- Blank these values and it should use Kopete's settings (temporary workaround) -->
  <xsl:variable name="font-size">12px</xsl:variable>
  <xsl:variable name="font-family">Verdana</xsl:variable>
  <xsl:variable name="name-font-size">10px</xsl:variable>
  <xsl:variable name="timestamp-font-size">10px</xsl:variable>
  <xsl:variable name="timestamp-color">LightSlateGrey</xsl:variable>

  <xsl:template match="message">
    <div class="KopeteMessage" style="padding-bottom:0px;">
      <xsl:attribute name="id">
        <xsl:value-of select="@id"/>
      </xsl:attribute>
      <xsl:choose>
        <xsl:when test="@direction='0'">
          <!-- Incoming Message -->
          <table width="100%" border="0" cellspacing="0" cellpadding="0" style="margin-bottom: 5px;">
            <tr>
              <td colspan="2" align="center">
                <span>
                  <xsl:attribute name="style">
                    <xsl:text>font-family:</xsl:text>
                    <xsl:value-of select="$font-family"/>
                    <xsl:text>;font-size:</xsl:text>
                    <xsl:value-of select="$timestamp-font-size"/>
                    <xsl:text>;color:</xsl:text>
                    <xsl:value-of select="$timestamp-color"/>
                  </xsl:attribute>
                  <xsl:value-of select="@time"/>
                </span>
              </td>
            </tr>
            <tr>
              <td width="32" valign="bottom">
                <img height="32" width="32">
                  <xsl:attribute name="src">
                   <xsl:value-of select="$image-location" />
                   <xsl:value-of select="$from-color-scheme" />
                   <xsl:text>them.png</xsl:text>
                  </xsl:attribute>
                  <xsl:attribute name="title">
                    <xsl:value-of select="from/contact/contactDisplayName/@text"/>
                  </xsl:attribute>
                </img>
              </td>
              <td>
                <table cellpadding="0" cellspacing="0" border="0" align="left">
                  <tr>
                    <td height="12" width="21">
                      <xsl:attribute name="background">
                        <xsl:value-of select="$image-location" />
                        <xsl:choose>
                          <xsl:when test="@importance='2'">
                            <xsl:value-of select="$highlight-color-scheme"/>
                          </xsl:when>
                          <xsl:otherwise>
                            <xsl:value-of select="$from-color-scheme" />
                          </xsl:otherwise>
                        </xsl:choose>
                        <xsl:text>From/tl.png</xsl:text>
                      </xsl:attribute>
                    </td>
                    <td height="12">
                      <xsl:attribute name="background">
                        <xsl:value-of select="$image-location" />
                        <xsl:choose>
                          <xsl:when test="@importance='2'">
                            <xsl:value-of select="$highlight-color-scheme"/>
                          </xsl:when>
                          <xsl:otherwise>
                            <xsl:value-of select="$from-color-scheme" />
                          </xsl:otherwise>
                        </xsl:choose>
                        <xsl:text>From/tm.png</xsl:text>
                      </xsl:attribute>
                    </td>
                    <td height="12" width="17">
                      <xsl:attribute name="background">
                        <xsl:value-of select="$image-location" />
                        <xsl:choose>
                          <xsl:when test="@importance='2'">
                            <xsl:value-of select="$highlight-color-scheme"/>
                          </xsl:when>
                          <xsl:otherwise>
                            <xsl:value-of select="$from-color-scheme" />
                          </xsl:otherwise>
                        </xsl:choose>
                        <xsl:text>From/tr.png</xsl:text>
                      </xsl:attribute>
                    </td>
                  </tr>
                  <tr>
                    <td height="14" width="21">
                      <xsl:attribute name="background">
                        <xsl:value-of select="$image-location" />
                        <xsl:choose>
                          <xsl:when test="@importance='2'">
                            <xsl:value-of select="$highlight-color-scheme"/>
                          </xsl:when>
                          <xsl:otherwise>
                            <xsl:value-of select="$from-color-scheme" />
                          </xsl:otherwise>
                        </xsl:choose>
                        <xsl:text>From/ml.png</xsl:text>
                      </xsl:attribute>
                    </td>
                    <td>
                      <xsl:attribute name="background">
                        <xsl:value-of select="$image-location" />
                        <xsl:choose>
                          <xsl:when test="@importance='2'">
                            <xsl:value-of select="$highlight-color-scheme"/>
                          </xsl:when>
                          <xsl:otherwise>
                            <xsl:value-of select="$from-color-scheme" />
                          </xsl:otherwise>
                        </xsl:choose>
                        <xsl:text>From/mm.png</xsl:text>
                      </xsl:attribute>
                      <span>
                        <xsl:attribute name="dir">
                          <xsl:value-of select="body/@dir"/>
                        </xsl:attribute>
                        <xsl:attribute name="style">
                          <xsl:text>font-size:</xsl:text>
                          <xsl:value-of select="$font-size" />
                          <xsl:text>;</xsl:text>
                          <xsl:choose>
                            <xsl:when test="$allow-user-colors='yes'">
                              <xsl:if test="body/@color">
                                <xsl:text>color:</xsl:text>
                                <xsl:value-of select="body/@color"/>
                                <xsl:text>;</xsl:text>
                              </xsl:if>
                              <xsl:if test="body/@bgcolor">
                                <xsl:text>background-color:</xsl:text>
                                <xsl:value-of select="body/@bgcolor"/>
                                <xsl:text>;</xsl:text>
                              </xsl:if>
                              <xsl:if test="body/@font">
                                <xsl:value-of select="body/@font"/>
                                <xsl:text>; </xsl:text>
                              </xsl:if>
                            </xsl:when>
                            <xsl:otherwise>
                              <xsl:text>font-family:'</xsl:text>
                              <xsl:value-of select="$font-family" />
                              <xsl:text>';</xsl:text>
                              <xsl:text>color:black;</xsl:text>
                              <xsl:text>background-color:transparent;</xsl:text>
                            </xsl:otherwise>
                          </xsl:choose>
                        </xsl:attribute>
                        <xsl:value-of disable-output-escaping="yes" select="body"/>
                      </span>
                    </td>
                    <td height="14" width="17">
                      <xsl:attribute name="background">
                        <xsl:value-of select="$image-location" />
                        <xsl:choose>
                          <xsl:when test="@importance='2'">
                            <xsl:value-of select="$highlight-color-scheme"/>
                          </xsl:when>
                          <xsl:otherwise>
                            <xsl:value-of select="$from-color-scheme" />
                          </xsl:otherwise>
                        </xsl:choose>
                        <xsl:text>From/mr.png</xsl:text>
                      </xsl:attribute>
                    </td>
                  </tr>
                  <tr>
                    <td height="12" width="21">
                      <xsl:attribute name="background">
                        <xsl:value-of select="$image-location" />
                        <xsl:choose>
                          <xsl:when test="@importance='2'">
                            <xsl:value-of select="$highlight-color-scheme"/>
                          </xsl:when>
                          <xsl:otherwise>
                            <xsl:value-of select="$from-color-scheme" />
                          </xsl:otherwise>
                        </xsl:choose>
                        <xsl:text>From/bl.png</xsl:text>
                      </xsl:attribute>
                    </td>
                    <td height="12">
                      <xsl:attribute name="background">
                        <xsl:value-of select="$image-location" />
                        <xsl:choose>
                          <xsl:when test="@importance='2'">
                            <xsl:value-of select="$highlight-color-scheme"/>
                          </xsl:when>
                          <xsl:otherwise>
                            <xsl:value-of select="$from-color-scheme" />
                          </xsl:otherwise>
                        </xsl:choose>
                        <xsl:text>From/bm.png</xsl:text>
                      </xsl:attribute>
                    </td>
                    <td height="12" width="17">
                      <xsl:attribute name="background">
                        <xsl:value-of select="$image-location" />
                        <xsl:choose>
                          <xsl:when test="@importance='2'">
                            <xsl:value-of select="$highlight-color-scheme"/>
                          </xsl:when>
                          <xsl:otherwise>
                            <xsl:value-of select="$from-color-scheme" />
                          </xsl:otherwise>
                        </xsl:choose>
                        <xsl:text>From/br.png</xsl:text>
                      </xsl:attribute>
                    </td>
                  </tr>
                </table>
              </td>
            </tr>
            <xsl:choose>
              <xsl:when test="$public-names='yes'">
                <tr>
                  <td colspan="2">
                    <span>
                      <xsl:attribute name="style">
                        <xsl:text>font-family:</xsl:text>
                        <xsl:value-of select="$font-family"/>
                        <xsl:text>;font-size:</xsl:text>
                        <xsl:value-of select="$name-font-size"/>
                        <xsl:text>;</xsl:text>
                      </xsl:attribute>
                      <xsl:value-of select="from/contact/contactDisplayName/@text"/>
                    </span>
                  </td>
                </tr>
              </xsl:when>
              <xsl:otherwise>
              </xsl:otherwise>
            </xsl:choose>
          </table>
        </xsl:when>
        <xsl:when test="@direction='1'">
          <!-- Outgoing Message -->
          <table width="100%" border="0" cellspacing="0" cellpadding="0" style="margin-bottom: 5px;">
            <tr>
              <td colspan="2" align="center">
                <span>
                  <xsl:attribute name="style">
                    <xsl:text>font-family:</xsl:text>
                    <xsl:value-of select="$font-family"/>
                    <xsl:text>;font-size:</xsl:text>
                    <xsl:value-of select="$timestamp-font-size"/>
                    <xsl:text>;color:</xsl:text>
                    <xsl:value-of select="$timestamp-color"/>
                  </xsl:attribute>
                  <xsl:value-of select="@time"/>
                </span>
              </td>
            </tr>
            <tr>
              <td>
                <table cellpadding="0" cellspacing="0" border="0" align="right">
                  <tr>
                    <td height="12" width="17">
                      <xsl:attribute name="background">
                        <xsl:value-of select="$image-location" />
                        <xsl:value-of select="$to-color-scheme" />
                        <xsl:text>To/tl.png</xsl:text>
                      </xsl:attribute>
                    </td>
                    <td height="12">
                      <xsl:attribute name="background">
                        <xsl:value-of select="$image-location" />
                        <xsl:value-of select="$to-color-scheme" />
                        <xsl:text>To/tm.png</xsl:text>
                      </xsl:attribute>
                    </td>
                    <td height="12" width="21">
                      <xsl:attribute name="background">
                        <xsl:value-of select="$image-location" />
                        <xsl:value-of select="$to-color-scheme" />
                        <xsl:text>To/tr.png</xsl:text>
                      </xsl:attribute>
                    </td>
                  </tr>
                  <tr>
                    <td height="14" width="17">
                      <xsl:attribute name="background">
                       <xsl:value-of select="$image-location" />
                       <xsl:value-of select="$to-color-scheme" />
                       <xsl:text>To/ml.png</xsl:text>
                      </xsl:attribute>
                    </td>
                    <td>
                      <xsl:attribute name="background">
                       <xsl:value-of select="$image-location" />
                       <xsl:value-of select="$to-color-scheme" />
                       <xsl:text>To/mm.png</xsl:text>
                      </xsl:attribute>
                      <span>
                        <xsl:attribute name="dir">
                          <xsl:value-of select="body/@dir"/>
                        </xsl:attribute>
                        <xsl:attribute name="style">
                          <xsl:text>font-size:</xsl:text>
                          <xsl:value-of select="$font-size" />
                          <xsl:text>;</xsl:text>
                          <xsl:choose>
                            <xsl:when test="$allow-user-colors='yes'">
                              <xsl:if test="body/@color">
                                <xsl:text>color:</xsl:text>
                                <xsl:value-of select="body/@color"/>
                                <xsl:text>;</xsl:text>
                              </xsl:if>
                              <xsl:if test="body/@bgcolor">
                                <xsl:text>background-color:</xsl:text>
                                <xsl:value-of select="body/@bgcolor"/>
                                <xsl:text>;</xsl:text>
                              </xsl:if>
                              <xsl:if test="body/@font">
                                <xsl:text>; </xsl:text>
                                <xsl:value-of select="body/@font"/>
                              </xsl:if>
                            </xsl:when>
                            <xsl:otherwise>
                              <xsl:text>font-family:'</xsl:text>
                              <xsl:value-of select="$font-family" />
                              <xsl:text>';</xsl:text>
                              <xsl:text>color:black;</xsl:text>
                              <xsl:text>background-color:transparent;</xsl:text>
                            </xsl:otherwise>
                          </xsl:choose>
                        </xsl:attribute>
                        <xsl:value-of disable-output-escaping="yes" select="body"/>
                      </span>
                    </td>
                    <td height="14" width="21">
                      <xsl:attribute name="background">
                       <xsl:value-of select="$image-location" />
                       <xsl:value-of select="$to-color-scheme" />
                       <xsl:text>To/mr.png</xsl:text>
                      </xsl:attribute>
                    </td>
                  </tr>
                  <tr>
                    <td height="12" width="17">
                      <xsl:attribute name="background">
                       <xsl:value-of select="$image-location" />
                       <xsl:value-of select="$to-color-scheme" />
                       <xsl:text>To/bl.png</xsl:text>
                      </xsl:attribute>
                    </td>
                    <td height="12">
                      <xsl:attribute name="background">
                       <xsl:value-of select="$image-location" />
                       <xsl:value-of select="$to-color-scheme" />
                       <xsl:text>To/bm.png</xsl:text>
                      </xsl:attribute>
                    </td>
                    <td height="12" width="21">
                      <xsl:attribute name="background">
                       <xsl:value-of select="$image-location" />
                       <xsl:value-of select="$to-color-scheme" />
                       <xsl:text>To/br.png</xsl:text>
                      </xsl:attribute>
                    </td>
                  </tr>
                </table>
              </td>
              <td width="32" valign="bottom">
                <img height="32" width="32">
                  <xsl:attribute name="src">
                   <xsl:value-of select="$image-location" />
                   <xsl:value-of select="$to-color-scheme" />
                   <xsl:text>me.png</xsl:text>
                  </xsl:attribute>
                  <xsl:attribute name="title">
                    <xsl:value-of select="from/contact/contactDisplayName/@text"/>
                  </xsl:attribute>
                </img>
              </td>
            </tr>
          </table>
        </xsl:when>
        <xsl:when test="@direction='2'">
          <!-- Internal Message -->
          <table width="100%" border="0" cellspacing="0" cellpadding="0" style="margin-bottom: 5px;">
            <tr>
              <td colspan="2" align="center">
                <span>
                  <xsl:attribute name="style">
                    <xsl:text>font-family:</xsl:text>
                    <xsl:value-of select="$font-family"/>
                    <xsl:text>;font-size:</xsl:text>
                    <xsl:value-of select="$timestamp-font-size"/>
                    <xsl:text>;color:</xsl:text>
                    <xsl:value-of select="$timestamp-color"/>
                  </xsl:attribute>
                  <xsl:value-of select="@time"/>
                </span>
              </td>
            </tr>
            <tr>
              <td>
                <table cellpadding="0" cellspacing="0" border="0" align="center">
                  <tr>
                    <td height="12" width="17">
                      <xsl:attribute name="background">
                       <xsl:value-of select="$image-location" />
                       <xsl:value-of select="$system-color-scheme" />
                       <xsl:text>To/tl.png</xsl:text>
                      </xsl:attribute>
                    </td>
                    <td height="12">
                      <xsl:attribute name="background">
                       <xsl:value-of select="$image-location" />
                       <xsl:value-of select="$system-color-scheme" />
                       <xsl:text>To/tm.png</xsl:text>
                      </xsl:attribute>
                    </td>
                    <td height="12" width="17">
                      <xsl:attribute name="background">
                       <xsl:value-of select="$image-location" />
                       <xsl:value-of select="$system-color-scheme" />
                       <xsl:text>From/tr.png</xsl:text>
                      </xsl:attribute>
                    </td>
                  </tr>
                  <tr>
                    <td height="14" width="17">
                      <xsl:attribute name="background">
                       <xsl:value-of select="$image-location" />
                       <xsl:value-of select="$system-color-scheme" />
                       <xsl:text>To/ml.png</xsl:text>
                      </xsl:attribute>
                    </td>
                    <td>
                      <xsl:attribute name="background">
                       <xsl:value-of select="$image-location" />
                       <xsl:value-of select="$system-color-scheme" />
                       <xsl:text>To/mm.png</xsl:text>
                      </xsl:attribute>
                      <span>
                        <xsl:attribute name="dir">
                          <xsl:value-of select="body/@dir"/>
                        </xsl:attribute>
                        <xsl:attribute name="style">
                          <xsl:text>color:black;</xsl:text>
                          <xsl:text>background-color:transparent;</xsl:text>
                          <xsl:text>font-size:</xsl:text>
                          <xsl:value-of select="$font-size" />
                          <xsl:text>;</xsl:text>
                          <xsl:text>font-family:'</xsl:text>
                          <xsl:value-of select="$font-family" />
                          <xsl:text>';</xsl:text>
                        </xsl:attribute>
                        <xsl:value-of disable-output-escaping="yes" select="body"/>
                      </span>
                    </td>
                    <td height="14" width="17">
                      <xsl:attribute name="background">
                       <xsl:value-of select="$image-location" />
                       <xsl:value-of select="$system-color-scheme" />
                       <xsl:text>From/mr.png</xsl:text>
                      </xsl:attribute>
                    </td>
                  </tr>
                  <tr>
                    <td height="12" width="17">
                      <xsl:attribute name="background">
                       <xsl:value-of select="$image-location" />
                       <xsl:value-of select="$system-color-scheme" />
                       <xsl:text>To/bl.png</xsl:text>
                      </xsl:attribute>
                    </td>
                    <td height="12">
                      <xsl:attribute name="background">
                       <xsl:value-of select="$image-location" />
                       <xsl:value-of select="$system-color-scheme" />
                       <xsl:text>To/bm.png</xsl:text>
                      </xsl:attribute>
                    </td>
                    <td height="12" width="17">
                      <xsl:attribute name="background">
                       <xsl:value-of select="$image-location" />
                       <xsl:value-of select="$system-color-scheme" />
                       <xsl:text>From/br.png</xsl:text>
                      </xsl:attribute>
                    </td>
                  </tr>
                </table>
              </td>
            </tr>
          </table>
        </xsl:when>
        <xsl:when test="@direction='3'">
          <!-- Action Message -->
              <table width="100%" border="0" cellspacing="0" cellpadding="0" style="margin-bottom: 5px;">
                <tr>
                  <td colspan="2" align="center">
                    <span>
                      <xsl:attribute name="style">
                        <xsl:text>font-family:</xsl:text>
                        <xsl:value-of select="$font-family"/>
                        <xsl:text>;font-size:</xsl:text>
                        <xsl:value-of select="$timestamp-font-size"/>
                        <xsl:text>;color:</xsl:text>
                        <xsl:value-of select="$timestamp-color"/>
                      </xsl:attribute>
                      <xsl:value-of select="@time"/>
                    </span>
                  </td>
                </tr>
                <tr>
                  <td width="32" valign="bottom">
                    <img height="32" width="32">
                      <xsl:attribute name="src">
                       <xsl:value-of select="$image-location" />
                       <xsl:value-of select="$from-color-scheme" />
                       <xsl:text>them.png</xsl:text>
                      </xsl:attribute>
                      <xsl:attribute name="title">
                        <xsl:value-of select="from/contact/contactDisplayName/@text"/>
			<xsl:value-of select="from/contact/@color"/>
			<xsl:value-of select="to/contact/@color"/>
                      </xsl:attribute>
                    </img>
                  </td>
                  <td>
                    <table cellpadding="0" cellspacing="0" border="0" align="left">
                      <tr>
                        <td height="12" width="21">
                          <xsl:attribute name="background">
                           <xsl:value-of select="$image-location" />
                           <xsl:value-of select="$action-color-scheme" />
                           <xsl:text>From/tl.png</xsl:text>
                          </xsl:attribute>
                        </td>
                        <td height="12">
                          <xsl:attribute name="background">
                           <xsl:value-of select="$image-location" />
                           <xsl:value-of select="$action-color-scheme" />
                           <xsl:text>From/tm.png</xsl:text>
                          </xsl:attribute>
                        </td>
                        <td height="12" width="17">
                          <xsl:attribute name="background">
                           <xsl:value-of select="$image-location" />
                           <xsl:value-of select="$action-color-scheme" />
                           <xsl:text>From/tr.png</xsl:text>
                          </xsl:attribute>
                        </td>
                      </tr>
                      <tr>
                        <td height="14" width="21">
                          <xsl:attribute name="background">
                           <xsl:value-of select="$image-location" />
                           <xsl:value-of select="$action-color-scheme" />
                           <xsl:text>From/ml.png</xsl:text>
                          </xsl:attribute>
                        </td>
                        <td>
                          <xsl:attribute name="background">
                           <xsl:value-of select="$image-location" />
                           <xsl:value-of select="$action-color-scheme" />
                           <xsl:text>From/mm.png</xsl:text>
                          </xsl:attribute>
                          <span>
                            <xsl:attribute name="dir">
                              <xsl:value-of select="body/@dir"/>
                            </xsl:attribute>
                            <xsl:attribute name="style">
                              <xsl:text>font-size:</xsl:text>
                              <xsl:value-of select="$font-size" />
                              <xsl:text>;</xsl:text>
                              <xsl:choose>
                                <xsl:when test="$allow-user-colors='yes'">
                                  <xsl:if test="body/@color">
                                    <xsl:text>color:</xsl:text>
                                    <xsl:value-of select="body/@color"/>
                                    <xsl:text>;</xsl:text>
                                  </xsl:if>
                                  <xsl:if test="body/@bgcolor">
                                    <xsl:text>background-color:</xsl:text>
                                    <xsl:value-of select="body/@bgcolor"/>
                                    <xsl:text>;</xsl:text>
                                  </xsl:if>
                                  <xsl:if test="body/@font">
                                    <xsl:text>; </xsl:text>
                                    <xsl:value-of select="body/@font"/>
                                  </xsl:if>
                                </xsl:when>
                                <xsl:otherwise>
                                  <xsl:text>font-family:'</xsl:text>
                                  <xsl:value-of select="$font-family" />
                                  <xsl:text>';</xsl:text>
                                  <xsl:text>color:black;</xsl:text>
                                  <xsl:text>background-color:transparent;</xsl:text>
                                </xsl:otherwise>
                              </xsl:choose>
                            </xsl:attribute>
                            <xsl:text>*&#160;</xsl:text>
                            <xsl:value-of disable-output-escaping="yes" select="body"/>
                            <xsl:text>&#160;*</xsl:text>
                          </span>
                        </td>
                        <td height="14" width="17">
                          <xsl:attribute name="background">
                           <xsl:value-of select="$image-location" />
                           <xsl:value-of select="$action-color-scheme" />
                           <xsl:text>From/mr.png</xsl:text>
                          </xsl:attribute>
                        </td>
                      </tr>
                      <tr>
                        <td height="12" width="21">
                          <xsl:attribute name="background">
                           <xsl:value-of select="$image-location" />
                           <xsl:value-of select="$action-color-scheme" />
                           <xsl:text>From/bl.png</xsl:text>
                          </xsl:attribute>
                        </td>
                        <td height="12">
                          <xsl:attribute name="background">
                           <xsl:value-of select="$image-location" />
                           <xsl:value-of select="$action-color-scheme" />
                           <xsl:text>From/bm.png</xsl:text>
                          </xsl:attribute>
                        </td>
                        <td height="12" width="17">
                          <xsl:attribute name="background">
                           <xsl:value-of select="$image-location" />
                           <xsl:value-of select="$action-color-scheme" />
                           <xsl:text>From/br.png</xsl:text>
                          </xsl:attribute>
                        </td>
                      </tr>
                    </table>
                  </td>
                </tr>
                <xsl:choose>
                  <xsl:when test="$public-names='yes'">
                    <tr>
                      <td colspan="2">
                        <span>
                          <xsl:attribute name="style">
                            <xsl:text>font-family:</xsl:text>
                            <xsl:value-of select="$font-family"/>
                            <xsl:text>;font-size:</xsl:text>
                            <xsl:value-of select="$name-font-size"/>
                            <xsl:text>;</xsl:text>
                          </xsl:attribute>
                          <xsl:value-of select="from/contact/contactDisplayName/@text"/>
                        </span>
                      </td>
                    </tr>
                  </xsl:when>
                  <xsl:otherwise>
                  </xsl:otherwise>
                </xsl:choose>
              </table>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of disable-output-escaping="yes" select="body"/>
        </xsl:otherwise>
      </xsl:choose>
    </div>
  </xsl:template>
</xsl:stylesheet>
