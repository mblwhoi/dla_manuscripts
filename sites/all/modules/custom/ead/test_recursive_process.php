<?php
require_once("libraries/ead_parser.inc");

class TestProcessor{

  protected $node_id;
  protected $component_id;
  protected $xml;
  protected $real;

  /**
   * Constructor.
   */
  public function __construct($xml) {

    // set xml
    $this->xml = $xml;
    
    // initialize component_id to -1 so that first id is 0
    $this->component_id = -1;

    $this->real = true;
  }

  /**
   * Implementation of FeedsProcessor::process().
   */
  public function process() {

    // Keep track of processed items in this pass.
    $processed = 0;

    // make a DOM object from the batch object's raw xml string
    
    print "creating dom doc\n";
    $sm = memory_get_usage($this->real);
    $domDoc = new DOMDocument();
    if (! $domDoc->loadXML($this->xml) ){
	    return FALSE;      
    }
    $em = memory_get_usage($this->real);
    print "memory used: " . ($em - $sm) . "\n\n";
    

    // make an xpath object to use for xpath queries
    $xp = new DOMXPath($domDoc);

    // xpath expression for selecting only EAD components
    $ead_component_xpath_expression = "(starts-with(name(),'c0') or starts-with(name(),'c1') and string-length(name())=3)";

    // Update collection node here.
    $collection_node = new stdClass();
    $collection_node->nid = $this->get_next_node_id();

    // set its title
    $collection_node->title = $this->html2text($this->getDOMNodeInnerHTML($xp->query("/ead/archdesc/did/unittitle")->item(0) ) );

    // generate its body via an xslt transformation
    print "doing transformation\n";
    $sm = memory_get_usage($this->real);    
    //$collection_node->body = $this->getCollectionNodeBody($this->xml);
    $em = memory_get_usage($this->real);
    print "memory used: " . ($em - $sm) . "\n";


    print "doing recursion\n";
    $sm = memory_get_usage($this->real);

    $sm = memory_get_usage($this->real);    
    $dsc_child_components = $xp->query("/ead/archdesc/dsc/*[$ead_component_xpath_expression]");

    // Loop through dsc child components to look for sub-items.
    foreach($dsc_child_components as $child_component){
      $this->process_ead_component($child_component, $collection_node->nid, array($collection_node->nid), array($collection_term['tid']), $vocabulary_id, $batch, $source, $xp);
    }

    $em = memory_get_usage($this->real);
    print "total recursion memory used: " . ($em - $sm) . "\n";
    print "sm is: $sm, em is $em\n";
    

  }


  /*
   @todo: FUNCTION DOCUMENTATION HERE!!!
  */
  public function process_ead_component($component, $collection_nid, $ancestor_nids, $parent_term_ids, $vocabulary_id, $batch, $source, &$xp){

    $report_str = "-----\nprocess_ead_component, component is: " . $component->tagName . "\n";
    $sm = memory_get_usage($this->real);

    // create a node object for the component
    $node = new stdClass();

    // Populate and prepare node object
    // @todo: Think about node type, how to best code this in.  Depends on existing of ead_component content type
    $node->type = 'ead_component';


    // base xpath for selecting component attributes.
    // This allows us to select immediate attributes without regard to how they are nested,
    // and avoids selecting attributes of child components, 
    // @todo: Combine this with the earlier ead_component_xpath_expression so that it's consolidated
    $ead_component_xpath_expression = "(starts-with(name(),'c0') or starts-with(name(),'c1') and string-length(name())=3)";
    $attribute_xpath_base = "./*[not($ead_component_xpath_expression)]/";

    // set the title to be the unittitle
    $node->title = $this->getTextViaXpathQuery($attribute_xpath_base . "unittitle", $xp, $component);

    // set the body to be the abstract
    $node->body = $this->getTextViaXpathQuery($attribute_xpath_base . "abstract", $xp, $component);

    // Note: CCK fields need the extra field indices and 'value' index
    // set the date to be the unitdate
    $node->field_ead_component_date[0]['value'] = $this->getTextViaXpathQuery($attribute_xpath_base . "unitdate", $xp, $component);

    // set the box to be the container w/ box attribute
    $node->field_ead_component_box[0]['value'] = $this->getTextViaXpathQuery($attribute_xpath_base . "container[@type='Box']", $xp, $component);

    // set the folder to be the container w/ box attribute
    $node->field_ead_component_folder[0]['value'] = $this->getTextViaXpathQuery($attribute_xpath_base . "container[@type='Folder']", $xp, $component);

    // set the folder to be the level attribute value
    $node->field_ead_component_level[0]['value'] = $component->getAttribute("level");

    // set the component id
    $node->field_ead_component_component_id[0]['value'] = $this->get_next_component_id();

    // set the parent collection id
    $node->field_ead_component_collection[0]['value'] = $collection_nid;

    // save the node
    $node->nid = $this->get_next_node_id();

    // get child components
    $child_components = $xp->query("./*[$ead_component_xpath_expression]", $component);    

    //
    // recursively process item's child items if it has any
    //

    foreach($child_components as $child_component){
      $this->process_ead_component($child_component, $collection_nid, array_merge($ancestor_nids, array($node->nid)), array_merge($parent_term_ids, array("term_" . $node->nid) ), $vocabulary_id, $batch, $source, $xp);
    }

    $em = memory_get_usage($this->real);

    if ($component->tagName === "c01"){

      //print "$report_str " . "memory used: " . ($em - $sm) . "\n\n";
      //print "sm was $sm, em was $em\n";

    }

  }


  /*
   get collection node body text by doing an xslt transform
   @todo: FUNCTION DOCUMENTATION HERE!!!
  */
  function getCollectionNodeBody($xml_string){
     
    // create an XSLT processor
    $processor = new XSLTProcessor();
 
    // load xslt files
    $xsl_dir =  "libraries/whoi_ead_xslt";

    $stylesheets = array(
      //"WHOIdsc.xsl",
      //"WHOIstyleMC.xsl",
      //"WHOIstyleAC.xsl"
      "ead.xsl",
    );

    // load stylesheets
    foreach ($stylesheets as $stylesheet){
      $xsl_doc = new DOMDocument();
      $xsl_doc->load("$xsl_dir/$stylesheet");
      $processor->importStylesheet($xsl_doc);
    }

    // generate a dom object for the source document
    $source_doc = new DOMDocument();
    $source_doc->loadXML($xml_string);

    // run the transformation
    $result_doc = $processor->transformToDoc($source_doc);

    // return the body text

    $body_node = $result_doc->getElementsByTagName('body')->item(0);

    $body_xml = $result_doc->saveXml($body_node);

    return $body_xml;
      
  }



  /**
   * helper function for getting full inner html of a DOM node
   */
  function getDOMNodeInnerHTML($element)
  {
    $innerHTML = "";

    // if the element is empty, return blank
    if (! $element->hasChildNodes()){
      return "";
    }

    $children = $element->childNodes;
    foreach ($children as $child)
      {
        $tmp_dom = new DOMDocument();
        $tmp_dom->appendChild($tmp_dom->importNode($child, true));
        $innerHTML.=$tmp_dom->saveHTML();
      }
    return $innerHTML;
  } 

  /**
   * helper function for getting full inner html of a DOM node as text
   */
  function html2text($html){
    return strip_tags($html);
  }

  /**
   * helper function for getting text from xpath query result
   * @todo: Clean this up!  This is kludgy!
   */
  function getTextViaXpathQuery($query,&$xp, &$dom_node){
    $results = $xp->query($query,$dom_node);

    if($results->length > 0){
      $tmp_node = $results->item(0);
      return $this->html2text($this->getDOMNodeInnerHTML($tmp_node));
    }
    else{
      return "";
    }
  }



  /**
   * helper function to get next component id.  Basically a counter.
   * @todo: Clean this up!  This is kludgy!
   */
  function get_next_component_id(){
    $this->component_id++;
    return $this->component_id;
  }

  /**
   * helper function to get next component id.  Basically a counter.
   * @todo: Clean this up!  This is kludgy!
   */
  function get_next_node_id(){
    $this->node_id++;
    return $this->node_id;
  }

}

// read an xml file into a string
$string = file_get_contents("AC-66_Logbooks.xml");

$t = new TestProcessor($string);

$t->process();