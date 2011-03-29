<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<xsl:template name="collection_inventory">

  <xsl:for-each select="/ead/archdesc/dsc/c01">
    <xsl:call-template name="ead_component">
    </xsl:call-template>     
  </xsl:for-each>
  
</xsl:template>


<!-- template for processing ead components -->
<xsl:template name="ead_component">

<div class="$css_prefix_ead_component">

  <!-- generate anchor for component based on depth-first sequence number-->
  <a><xsl:attribute name="name">ead_component_<xsl:number level="any" count="//*[(starts-with(name(),'c0') or starts-with(name(),'c1') and string-length(name())=3)]"/></xsl:attribute></a>

  <div class="title">title: <xsl:value-of select="./did/unittitle"/></div>

</div>


  <!-- call template for children -->
  <xsl:for-each select="./*[starts-with(name(),'c0') or starts-with(name(),'c1') and string-length(name())=3]">
    <xsl:call-template name="ead_component"/>
  </xsl:for-each>

</xsl:template>



</xsl:stylesheet>
