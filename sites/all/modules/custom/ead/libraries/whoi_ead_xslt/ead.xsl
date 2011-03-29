<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  
  <xsl:output method="html" encoding="ISO-8859-1" doctype-public="-//W3C//DTD HTML 4.0 Transitional//EN"/>

  <!-- parameters to be set at runtime -->
  <xsl:param name="collection_image_filepath">COLLECTION_IMAGE_PATH</xsl:param>

  <!-- Creates the main outline of the finding aid.-->

  <xsl:template match="/ead">
    <html>

      <!-- create EAD head -->
      <head>

	<!-- ead stylesheet -->
	<link rel="stylesheet" href="/finding_aid_resources/css/ead_style.css" type="text/css"/>

      </head>
      
      <!-- create EAD body -->
      <body>

	<div class="ead_finding_aid">

	  <!-- collection header -->
	  <xsl:call-template name="collection_header"/>

	  <hr/>

	  <!-- table of contents -->
	  <xsl:call-template name="toc"/>

	  <hr/>

	  <!-- collection description -->
	  <xsl:apply-templates select="archdesc"/>

	  <hr/>
	  
	  <!-- collection inventory -->
	  <xsl:apply-templates select="archdesc/dsc"/>

	</div>

      </body>

    </html>
  </xsl:template>



  <!-- collection header template -->
  <xsl:template name="collection_header">

    <div class="collection_header clearfix">

      <!-- logo -->
      <div class="logo">
	<img>
	  <xsl:attribute name="src"><xsl:value-of select="$collection_image_filepath"/></xsl:attribute>
	</img>
      </div>
      
      <!-- text -->
      <div class="text">
	<div class="whoi">Woods Hole Oceanographic Institution</div>
	<div class="titleproper">
	  <xsl:value-of select="eadheader/filedesc/titlestmt/titleproper"/>
	</div>
	<div class="unitdate">
	  <xsl:value-of select="archdesc/did/unitdate"/>
	</div>
	
	<!-- brief archive description -->
	<div class="brief_arch_desc">
	  <div class="eadid">Archival Collection <xsl:value-of select="eadheader/eadid"/></div>
	  <div class="physdesc"><xsl:value-of select="archdesc/did/physdesc"/></div>
	</div>
	
      </div>
      
    </div>

  </xsl:template>


  <!-- table of contents template -->
  <!-- The Table of Contents template performs a series of tests to determine which elements will be included in the table of contents.  Each if statement tests the presence of an element in the finding aid.  If the element is present, then we create a link to its anchor.  Anchors for these elements will be generated in other templates  -->
  <xsl:template name="toc">

    <div class="table_of_contents">

      <div class="head">
	Table Of Contents
      </div>

      <div class="content">

	<ul>
	  
	  <!-- bioghist -->
	  <xsl:if test="string(archdesc/bioghist/head)">
	    <li>
	      <a href="#{generate-id(/ead/archdesc/bioghist/head)}">
		<xsl:value-of select="archdesc/bioghist/head"/>
	      </a>
	    </li>
	  </xsl:if>

	  <!-- scope and content note -->
	  <xsl:if test="string(/ead/archdesc/scopecontent/head)">
	    <li>
	      <a href="#{generate-id(archdesc/scopecontent/head)}">
		<xsl:value-of select="archdesc/scopecontent/head"/>
	      </a>
	    </li>
	  </xsl:if>
	  
	  <!-- Administrative History, will link to one of these elements -->
	  <xsl:if test="string(archdesc/acqinfo/*)
			or string(archdesc/processinfo/*)
			or string(archdesc/prefercite/*)
			or string(archdesc/custodialhist/*)
			or string(archdesc/processinfo/*)
			or string(archdesc/appraisal/*)
			or string(archdesc/accruals/*)">

	    <li>
	      <a href="#adminlink">
		<xsl:text>Administrative Information</xsl:text>
	      </a>
	    </li>
	  </xsl:if>

	  <!-- Arrangement -->
	  <xsl:if test="string(archdesc/arrangement/head)">
	    <li>
	      <a href="#{generate-id(/ead/archdesc/arrangement/head)}">
		<xsl:value-of select="archdesc/arrangement/head"/>
	      </a>
	    </li>
	  </xsl:if>

	  <!-- Folder List -->
	  <xsl:if test="string(archdesc/dsc/head)">
	    <li>
	      <a href="#{generate-id(/ead/archdesc/dsc/head)}">
		<xsl:value-of select="archdesc/dsc/head"/>
	      </a>
	    </li>
	  </xsl:if>

	</ul>

      </div>

    </div>

  </xsl:template>


  <!--Include general stylesheet-->
  <xsl:include href="ead_general.xsl"/>

  <!--Include the stylesheet for the collection description template-->
  <xsl:include href="ead_collection_description.xsl"/>

  <!--Include the stylesheet for the collection inventory template-->
  <xsl:include href="ead_collection_inventory.xsl"/>


</xsl:stylesheet>
