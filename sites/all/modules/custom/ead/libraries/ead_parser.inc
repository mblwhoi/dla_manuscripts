<?php
// $Id

/**
 * @file
 *   Parsing functions for EAD files.
 */

/**
 * Parse the feed into a data structure.
 *
 * @param $string
 *  The EAD XML document
 * @return
 *  stdClass The structured data extracted from the feed.
 */


function ead_parser_parse($string) {

  // convert the xml string into a json object and return
  //return json_decode(xml2json::transformXmlStringToJson($string));  

  /*

  if (!defined('LIBXML_VERSION') || (version_compare(phpversion(), '5.1.0', '<'))) {
    @ $ead_xml = simplexml_load_string($string, NULL);
  }
  else {
    @ $ead_xml = simplexml_load_string($string, NULL, LIBXML_NOERROR | LIBXML_NOWARNING | LIBXML_NOCDATA);
  }

  // Got a malformed XML.
  if ($ead_xml === FALSE || is_null($ead_xml)) {
    return FALSE;
  }

  // an array for holding the parsed data
  $parsed_source = array();


  // parse collection data
  $collection_data = array();

  // title
  $collection_data['title'] = (string) array_shift($ead_xml->xpath("../ead/archdesc/did/unittitle"));  


  $parsed_source['collection_data'] = $collection_data;

  // parse items under the dsc tag
  $dsc_path = "../ead/archdesc/dsc";
  $dsc_children = array_shift($ead_xml->xpath($dsc_path))->children();

  $items = array();
  

  // Loop through dsc child elements to look for sub-items.
  foreach($dsc_children as $child_element){
	
      $child_name = $child_element->getName();

      // If child element is an ead item, parse it recursively.
      if (preg_match('/c\d\d/', $child_name)){
	  ead_parser_parse_items_recursively("",&$child_element, &$items);
      }
  }
  
  // Save the items to the parsed_source array
  $parsed_source['items'] = $items;

  return $parsed_source;
}


function ead_parser_parse_items_recursively($json_parent_path, &$item_element, &$items){

    // Get the item tag name
    $item_name = $item_element->getName();

    // Get the item title, date
    $item_title = strip_tags(array_shift($item_element->xpath("did/unittitle"))->asXML());
    $item_date = (string) array_shift($item_element->xpath("did/unitdate"));

    // deserialize the parent path into an array
    $parent_path = json_decode($json_parent_path);

    // append the item title to the path
    $parent_path[] = $item_title;

    // serialize the item path
    $json_item_path = json_encode($parent_path);

    // save the item to the items array
    $items[] = array(
	title => $item_title,
	date => $item_date,
	path => $json_item_path
    );

    // Loop through child elements to look for sub-items.

    $child_elements = $item_element->children();

    foreach($child_elements as $child_element){
	
	$child_name = $child_element->getName();

	// If child element is an ead item, parse it recursively.
	if (preg_match('/c\d\d/', $child_name)){
	    ead_parser_parse_items_recursively($json_item_path,&$child_element,&$items);
	}

    }
    
    return $items;

  */
}
