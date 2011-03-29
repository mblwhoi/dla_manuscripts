<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

  <!-- collection inventory template -->
  <xsl:template name="collection_inventory" match="/ead/archdesc/dsc">

    <a name="{generate-id(/ead/archdesc/dsc/head)}"/>
    <div class="collection_inventory">

      <!-- output head -->
      <xsl:apply-templates select="head"/>

      <!-- output contents -->
      <div class="content">

	<div class="components">
	  <xsl:apply-templates select="*[not(name()='head')]"/>
	</div>

      </div>
    
    </div>

  </xsl:template>



  <!-- ead components template -->
  <xsl:template name="ead_component" match="*[starts-with(name(),'c0') or starts-with(name(),'c1') and string-length(name())=3]">

    <!-- generate an anchor if the component has an id -->
    <xsl:if test="@id">
      <a name="{generate-id()}"></a>	
    </xsl:if>

  <!-- dispatch to separate templates depending on whether ead component has a container -->
    <xsl:choose>

      <!-- if has a container, call ead_component_has_container template -->
      <xsl:when test="did/container">
	<xsl:call-template name="ead_component_has_container"/>
      </xsl:when>
      
      <!-- otherwise call ead_component ead_component_no_container template -->
      <xsl:otherwise>
	<xsl:call-template name="ead_component_no_container"/>
      </xsl:otherwise>
    </xsl:choose>

  </xsl:template>


  <!-- template for ead components that have containers -->
  <xsl:template name="ead_component_has_container">

    <!-- call generate_ead_component_anchor template -->
    <!--
    <xsl:call-template name="generate_ead_component_anchor"/>
    -->

    <!-- output component div w/ has_container class-->
    <div class="ead_component has_container">

      <xsl:attribute name="id">ead_component_<xsl:number level="any" count="//*[(starts-with(name(),'c0') or starts-with(name(),'c1') and string-length(name())=3)]"/></xsl:attribute>

      <!-- output container in top level -->
      <xsl:call-template name="ead_component_container"/>
      
      <!-- create header div w/ head and clearfix classes -->
      <div class="header clearfix">
	
	<!-- output title -->
	<div class="head">
	  <xsl:apply-templates select="did/unittitle"/>
	</div>

	<!-- output dates  -->
	<xsl:call-template name="ead_component_dates"/>
	      
      </div>


      <!-- output content div -->
      <div class="content">

	<!-- output generic ead component content -->
	<xsl:call-template name="ead_component_generic_content"/>

      </div>

    </div>

  </xsl:template>



  <!-- template for ead components that don't have containers -->
  <xsl:template name="ead_component_no_container">

    <!-- call generate_ead_component_anchor template -->
    <!--
    <xsl:call-template name="generate_ead_component_anchor"/>
    -->

    <!-- output component div-->
    <div class="ead_component no_container">

      <xsl:attribute name="id">ead_component_<xsl:number level="any" count="//*[(starts-with(name(),'c0') or starts-with(name(),'c1') and string-length(name())=3)]"/></xsl:attribute>
      
      <!-- create header div -->
      <div class="header">

	<!-- output title with component level-->
	<div class="head">
	  <xsl:call-template name="capitalize">
	    <xsl:with-param name="input" select="@level" />
	  </xsl:call-template>: <xsl:apply-templates select="did/unittitle"/>
	</div>

      </div>


      <!-- output content div -->
      <div class="content">

	<!-- output dates -->
	<xsl:call-template name="ead_component_dates"/>

	<!-- output generic ead component content -->
	<xsl:call-template name="ead_component_generic_content"/>

      </div>

    </div>

  </xsl:template>





  <!-- template for generating ead component anchors -->
  <xsl:template name="generate_ead_component_anchor">

    <!-- generate anchor for component based on depth-first sequence number-->
    <a><xsl:attribute name="name">ead_component_<xsl:number level="any" count="//*[(starts-with(name(),'c0') or starts-with(name(),'c1') and string-length(name())=3)]"/></xsl:attribute></a>

  </xsl:template>


  <!-- template for processing ead component dates -->
  <xsl:template name="ead_component_dates">

    <!-- output dates if they are present-->
    <xsl:if test="string(did/unittitle/unitdate|did/unitdate)">

      <div class="dates">

	<div class="head">
	  Dates
	</div>

	<div class="content">
	  <ul>
	    <xsl:for-each select="did/unittitle/unitdate|did/unitdate">
	      <li>

		<xsl:apply-templates select="."/>
	      </li>
	    </xsl:for-each>
	  </ul>
	</div>

      </div>

    </xsl:if>

  </xsl:template>

  <!-- template for processing ead component container -->
  <xsl:template name="ead_component_container">

    <!-- test for existence of valid container types -->
    <xsl:if test="did/container[@type='Box']
		  |did/container[@type='Folder']">

      <div class="container">
	
	<xsl:if test="did/container[@type='Box']">
	  <div class="box">
	    <xsl:apply-templates select="did/container[@type='Box']"/>
	  </div>
	</xsl:if>
	
	<xsl:if test="did/container[@type='Folder']">
	  <div class="folder">
	    <xsl:apply-templates select="did/container[@type='Folder']"/>
	  </div>
	</xsl:if>

      </div>

    </xsl:if>
      
  </xsl:template>


  <!-- template for processing generic ead component content -->
  <xsl:template name="ead_component_generic_content">
    
    <!-- output physical description -->
    <xsl:if test="did/physdesc">
      <xsl:apply-templates select="did/physdesc"/>
    </xsl:if>
    
    <!-- output child elements -->
    <xsl:for-each select="scopecontent | bioghist | arrangement |
			  userestrict | accessrestrict | processinfo |
			  acqinfo | custodhist | controlaccess/controlaccess |
			  odd | note | descgrp/*">

      <xsl:apply-templates select="."/>
      
    </xsl:for-each>
    

    <!--output origination and a space if it exists in the markup.-->
    <xsl:if test="did/origination">
      <xsl:apply-templates select="did/origination"/>
      <xsl:text>&#x20;</xsl:text>
    </xsl:if>
    
    <!-- output other did elements -->
    <xsl:for-each select="did">
      
      <!-- output text -->
      <xsl:for-each select="p|note/p">
	<xsl:apply-templates/>
      </xsl:for-each>
      
      <!-- output abtract, text, langmaterial, or materialspec -->
      <xsl:for-each select="abstract | langmaterial | materialspec">
	<div>
	  <xsl:attribute name="class"><xsl:value-of select="name()"/></xsl:attribute>
	  <xsl:apply-templates/>
	</div>
      </xsl:for-each>
      
    </xsl:for-each>

    <!-- process child components -->
    <xsl:apply-templates select="*[starts-with(name(),'c0') or starts-with(name(),'c1') and string-length(name())=3]"/>

  </xsl:template>

</xsl:stylesheet>
