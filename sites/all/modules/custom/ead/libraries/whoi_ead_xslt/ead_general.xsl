<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

  <!-- this stylesheet contains general templates for EAD elements -->

  <xsl:template name="capitalize">

    <xsl:param name="input" />

    <xsl:variable name="lowercase" select="'abcdefghijklmnopqrstuvwxyz'" />
    <xsl:variable name="uppercase" select="'ABCDEFGHIJKLMNOPQRSTUVWXYZ'" />

    <xsl:variable name="first_letter" select="substring($input,1,1)"/>
    <xsl:variable name="remainder" select="substring($input,2)"/>

    <xsl:value-of select="concat(translate($first_letter, $lowercase, $uppercase), $remainder)"/>
   
  </xsl:template>


  <!-- head template -->
  <!-- outputs head as a div -->
  <xsl:template name="head" match="head">
    <div class="head">
      <xsl:apply-templates/>
    </div>

    <xsl:apply-templates select="*[not(name()='head')]"/>
  </xsl:template>


  <!-- generic ead element template -->
  <!-- outputs head, then notes, then processes everything else -->
  <xsl:template name="generic_ead_element" 
		match="bioghist
		       |scopecontent
		       |arrangement
		       |relatedmaterial
		       |controlaccess
		       |odd
		       |custodhist
		       |prefercite
		       |acqinfo
		       |processinfo
		       |appraisal
		       |accruals
		       |userestrict
		       |accessrestrict
		       |relatedmaterial
		       |separatedmaterial">

    <div>
      <xsl:attribute name="class"><xsl:value-of select="name()"/> ead_element</xsl:attribute>

      <xsl:apply-templates select="head"/>
      <xsl:apply-templates select="note"/>

      <div class="content">

	<xsl:for-each select="*">
	  <xsl:choose>
	    <xsl:when test="name()='head'">
	    </xsl:when>

	    <xsl:when test="name()='note'">
	    </xsl:when>

	    <xsl:otherwise>
	      <xsl:apply-templates select="."/>
	    </xsl:otherwise>
	  </xsl:choose>

	</xsl:for-each>

      </div>

    </div>

  </xsl:template>

  <!-- blockquote template -->
  <!-- encase in blockquote tag -->
  <xsl:template name="blockquote" match="blockquote">
    <blockquote>
      <xsl:apply-templates/>
    </blockquote>

  </xsl:template>


  <!-- chronlist templates -->
  <!-- format as table -->
  <xsl:template match="chronlist">
    <table width="100%" style="margin-left:25pt; font-size:10.5pt; font-family:Book Antiqua">
      <tr>
	<td width="5%"> </td>
	<td width="15%"> </td>
	<td width="80%"> </td>
      </tr>
      <xsl:apply-templates/>
    </table>
  </xsl:template>
  
  <xsl:template match="chronlist/head">
    <tr>
      <td colspan="3">
	<h4>
	  <xsl:apply-templates/>
	</h4>
      </td>
    </tr>
  </xsl:template>
  
  <xsl:template match="chronlist/listhead">
    <tr>
      <td> </td>
      <td>
	<b>
	  <xsl:apply-templates select="head01"/>
	</b>
      </td>
      <td>
	<b>
	  <xsl:apply-templates select="head02"/>
	</b>
      </td>
    </tr>
  </xsl:template>

  <xsl:template match="chronitem">
    
    <!--Determine if there are event groups.-->

    <xsl:choose>
      <xsl:when test="eventgrp">

	<!--Put the date and first event on the first line.-->

	<tr>
	  <td> </td>
	  <td valign="top">
	    <xsl:apply-templates select="date"/>
	  </td>
	  <td valign="top">
	    <xsl:apply-templates select="eventgrp/event[position()=1]"/>
	  </td>
	</tr>

	<!--Put each successive event on another line.-->

	<xsl:for-each select="eventgrp/event[not(position()=1)]">
	  <tr>
	    <td> </td>
	    <td> </td>
	    <td valign="top">
	      <xsl:apply-templates select="."/>
	    </td>
	  </tr>
	</xsl:for-each>
      </xsl:when>

      <!--Put the date and event on a single line.-->

      <xsl:otherwise>
	<tr>
	  <td> </td>
	  <td valign="top">
	    <xsl:apply-templates select="date"/>
	  </td>
	  <td valign="top">
	    <xsl:apply-templates select="event"/>
	  </td>
	</tr>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  

  <!-- list template -->
  <!-- make a div for the head if it exists, and a div for the list items -->
  <xsl:template match="list">

    <div class="list">

      <xsl:if test="string(head)">
	<div class="head">
	  <xsl:apply-templates select="head"/>
	</div>
      </xsl:if>

      <ul>
	<xsl:for-each select="item">
	  <li>
	    <xsl:apply-templates/>
	  </li>
	</xsl:for-each>
      </ul>

    </div>

  </xsl:template>

  <!-- p template -->
  <!-- output as p -->
  <xsl:template match="p">
    <p>
      <xsl:apply-templates/>
    </p>
  </xsl:template>


  <!--Formats a simple table. The width of each column is defined by the colwidth attribute in a colspec element.-->
  <xsl:template match="table">
    <table width="75%" style="margin-left: 25pt; font-family:Book Antiqua; font-size:10.5pt">
      <tr>
	<td colspan="3">
	  <h4>
	    <xsl:apply-templates select="head"/>
	  </h4>
	</td>
      </tr>
      <xsl:for-each select="tgroup">
	<tr>
	  <xsl:for-each select="colspec">
	    <td width="{@colwidth}"></td>
	  </xsl:for-each>
	</tr>
	<xsl:for-each select="thead">
	  <xsl:for-each select="row">
	    <tr>
	      <xsl:for-each select="entry">
		<td valign="top">
		  <b>
		    <xsl:apply-templates/>
		  </b>
		</td>
	      </xsl:for-each>
	    </tr>
	  </xsl:for-each>
	</xsl:for-each>

	<xsl:for-each select="tbody">
	  <xsl:for-each select="row">
	    <tr>
	      <xsl:for-each select="entry">
		<td valign="top">
		  <xsl:apply-templates/>
		</td>
	      </xsl:for-each>
	    </tr>
	  </xsl:for-each>
	</xsl:for-each>
      </xsl:for-each>
    </table>
  </xsl:template>



  <!--Formats ref elements: this is a test of git-->
  <!-- NOTE: THIS IS JUST A QUICK FIX FOR WHOI-SPECIFIC EADS.  IT PROBABLY NEEDS SOME MORE LOGIC FOR GENERAL USE -->
  <xsl:template match="ref">

    <!-- save target to a variable-->
    <xsl:variable name="target">
      <xsl:value-of select="@target"/>
    </xsl:variable>


    <!-- try to get an anchor id for the target... -->
    <xsl:variable name="target_id">

      <!-- first get node that corresponds to target... -->
      <xsl:for-each select="/ead//*[@id=$target]">

	<!-- then get the node's id -->
	<xsl:value-of select="generate-id()"/>

      </xsl:for-each>

    </xsl:variable>

    <!-- generate link to node -->
    <!-- note this requires that a corresponding anchor will be created when the node is processed, via generate-id() -->
    <a>
      <xsl:attribute name="href">#<xsl:value-of select="$target_id"/></xsl:attribute>
      <xsl:apply-templates/>
    </a>

  </xsl:template>


</xsl:stylesheet>
