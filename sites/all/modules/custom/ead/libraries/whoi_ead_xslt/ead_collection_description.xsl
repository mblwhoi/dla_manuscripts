<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

  <!-- collection description template -->
  <xsl:template name="collection_description" match="/ead/archdesc">

    <div class="collection_description">

      <!-- bioghist -->
      <a name="{generate-id(/ead/archdesc/bioghist/head)}"/>
      <xsl:apply-templates select="bioghist"/>
      <hr/>

      <!-- scope and content -->
      <a name="{generate-id(/ead/archdesc/scopecontent/head)}"/>
      <xsl:apply-templates select="scopecontent"/>
      <hr/>

      <!-- administrative information -->
      <a name="adminlink"/>
      <xsl:call-template name="archdesc-admininfo"/>
      <hr/>

      <!-- arrangement -->
      <a name="{generate-id(/ead/archdesc/arrangement/head)}"/>
      <xsl:if test="string(arrangement)">
	<xsl:apply-templates select="arrangement"/>
      </xsl:if>

      <!-- odd -->
      <xsl:if test="string(odd)">
	<div class="odd">      
	  <xsl:apply-templates select="odd"/>
	</div>
      </xsl:if>

    </div>

  </xsl:template>



  <!--This template formats the repostory, origination, physdesc, abstract, unitid, physloc and materialspec elements of archdesc/did which share a common presentaiton. The sequence of their appearance is governed by the previous template.-->

  <xsl:template match="archdesc/did/repository
		       | archdesc/did/origination
		       | archdesc/did/physdesc
		       | archdesc/did/unitid
		       | archdesc/did/physloc
		       | archdesc/did/abstract
		       | archdesc/did/langmaterial
		       | archdesc/did/materialspec">

    <!--The template tests to see if there is a label attribute, inserting the contents if there is or adding display textif there isn't. The content of the supplied label depends on the element.  To change the supplied label, simply alter the template below.-->

    <xsl:choose>
      <xsl:when test="@label">
	<div>
	  <div>
	    <b>
	      <xsl:value-of select="@label"/>
	    </b>
	  </div>
	  <div>
	    <xsl:apply-templates/>
	  </div>
	</div>
      </xsl:when>

      <xsl:otherwise>
	<div>
	  <div>
	    <b>
	      <xsl:choose>
		<xsl:when test="self::repository">
		  <xsl:text>Repository: </xsl:text>
		</xsl:when>
		<xsl:when test="self::origination">
		  <xsl:text>Creator: </xsl:text>
		</xsl:when>
		<xsl:when test="self::physdesc">
		  <xsl:text>Quantity: </xsl:text>
		</xsl:when>
		<xsl:when test="self::physloc">
		  <xsl:text>Location: </xsl:text>
		</xsl:when>
		<xsl:when test="self::unitid">
		  <xsl:text>Identification: </xsl:text>
		</xsl:when>
		<xsl:when test="self::abstract">
		  <xsl:text>Abstract:</xsl:text>
		</xsl:when>
		<xsl:when test="self::langmaterial">
		  <xsl:text>Language: </xsl:text>
		</xsl:when>
		<xsl:when test="self::materialspec">
		  <xsl:text>Technical: </xsl:text>
		</xsl:when>
	      </xsl:choose>
	    </b>
	  </div>
	  <div>
	    <xsl:apply-templates/>
	  </div>
	</div>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>



  <!--This templates consolidates all the other administrative information elements into one block under a common heading.  It formats these elements regardless of which of three encodings has been utilized.  They may be children of archdesc, admininfo, or descgrp. It begins by testing to see if there are any elements of this type with content.-->
  <xsl:template name="archdesc-admininfo">

    <xsl:if test="string(admininfo/custodhist/*)
		  or string(altformavailable/*)
		  or string(prefercite/*)
		  or string(acqinfo/*)
		  or string(processinfo/*)
		  or string(appraisal/*)
		  or string(accruals/*)
		  or string(userestrict/*)
		  or string(accessrestrict/*)
		  or string(relatedmaterial/*)
		  or string(separatedmaterial/*)
		  or string(custodhist/*)">

      <div class="admininfo">

	<a name="adminlink"></a>

	<div class="head">
	  <xsl:text>Administrative Information</xsl:text>
	</div>

	<div class="content">

	  <xsl:apply-templates select="custodhist"/>
	  <xsl:apply-templates select="altformavailable"/>
	  <xsl:apply-templates select="prefercite"/>
	  <xsl:apply-templates select="acqinfo"/>
	  <xsl:apply-templates select="processinfo"/>
	  <xsl:apply-templates select="appraisal"/>
	  <xsl:apply-templates select="accruals"/>
	  <xsl:apply-templates select="accessrestrict"/>
	  <xsl:apply-templates select="userestrict"/>
	  <xsl:apply-templates select="relatedmaterial"/>
	  <xsl:apply-templates select="separatedmaterial"/>
	  <xsl:apply-templates select="controlaccess"/>

	</div>

      </div>

    </xsl:if>
  </xsl:template>


  <!--This template formats the top-level controlaccess element. It begins by testing to see if there is any controlled access element with content. It then invokes one of two templates for the children of controlaccess.  -->

  <xsl:template match="archdesc/controlaccess">

    <!-- if there are children continue -->
    <xsl:if test="string(child::*)">

      <div>

	<!-- create a link for the head -->
	<a name="{generate-id(head)}">
	  <xsl:apply-templates select="head"/>
	</a>

	<!-- display text and notes -->
	<div>
	  <xsl:apply-templates select="p|note"/>
	</div>
      </div>

      <!-- test for whether there are nested controlaccess elements -->
      <xsl:choose>
	<!-- if element is a nested controlaccess item, apply template in recursive mode -->
	<xsl:when test="controlaccess">
	  <xsl:apply-templates mode="recursive" select="."/>
	</xsl:when>
	
	<!-- otherwise apply template in direct mode -->
	<xsl:otherwise>
	  <xsl:apply-templates mode="direct" select="."/>
	</xsl:otherwise>
      </xsl:choose>

    </xsl:if>
  </xsl:template>

  <!--This template formats controlled terms that are entered directly under the controlaccess element.  Elements are alphabetized.-->

  <xsl:template mode="direct" match="archdesc/controlaccess">
    <xsl:for-each select="subject |corpname | famname | persname | genreform | title | geogname | occupation">
      <xsl:sort select="." data-type="text" order="ascending"/>
      <div>
	<xsl:apply-templates/>
      </div>
    </xsl:for-each>
  </xsl:template>

  <!--This template formats controlled terms that are nested under the top-level controlaccess element.-->

  <xsl:template mode="recursive" match="archdesc/controlaccess">
    <xsl:apply-templates select="controlaccess"/>
  </xsl:template>

  <!--This template formats controlled terms that are nested within recursive controlaccess elements. Terms are alphabetized within each grouping.-->

  <xsl:template match="archdesc/controlaccess/controlaccess">
    <div>
      <xsl:apply-templates select="head"/>
    </div>
    <xsl:for-each select="subject |corpname | famname | persname | genreform | title | geogname | occupation">
      <xsl:sort select="." data-type="text" order="ascending"/>
      <div style="margin-left:15pt">
	<xsl:apply-templates/>
      </div>
    </xsl:for-each>
  </xsl:template>


</xsl:stylesheet>
