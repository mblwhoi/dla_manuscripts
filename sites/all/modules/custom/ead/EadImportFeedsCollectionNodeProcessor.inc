<?php
  // $Id: EadImportFeedsCollectionNodeProcessor.inc

  /**
   * @file
   * Class definition of EadImportFeedsCollectionNodeProcessor.
   */

  // Create or delete FEEDS_NODE_BATCH_SIZE at a time.
define('FEEDS_NODE_BATCH_SIZE', 50);

/**
 * Updates *ead_collection* node from feed content, and creates *ead_item* nodes from feed items.
 * @todo LATER HAVE THIS CALL THE CLEAR FUNCTION?  TO CLEAR OUT WHAT ALREADY EXISTS?
 */
class EadImportFeedsCollectionNodeProcessor extends FeedsProcessor{

  /**
   * Constructor.
   */
  protected function __construct($id) {
    parent::__construct($id);

  }

  /**
   * Implementation of FeedsProcessor::process().
   */
  public function process(FeedsImportBatch $batch, FeedsSource $source){

    // get the collection node
    // First get the parent node to which the importer is attached, via the source.
    $collection_node = node_load($source->feed_nid);


    // re-create the dom object
    // @todo: CLEAN THIS UP!  SHOULD REALLY BE A FIRST-TIME FUNCTION, TO DO INIT
    if (! $this->create_ead_dom($batch) ){
        drupal_set_message("Could not read EAD File.  Nothing has been changed.");
        return FEEDS_BATCH_COMPLETE;
    }


    // if batch->created is null then this is our first processing iteration and we should initalize
    if ($batch->created == null){

      // initialize batch->created
      $batch->created = 0;


      // update the collection node
      // set its title as a combination of the eadid and the EAD title

      $eadid = trim($this->html2text($this->getDOMNodeInnerHTML($batch->ead_xpath->query("/ead/eadheader/eadid")->item(0) ) ) );
      $title = $this->html2text($this->getDOMNodeInnerHTML($batch->ead_xpath->query("/ead/archdesc/did/unittitle")->item(0) ) );
      $collection_node->title = "$eadid: $title";

      // set the collection id
      $collection_node->field_ead_collection_id[0]['value'] = $eadid;
      

      // set the description to be the abstract
      $collection_node->field_ead_abstract[0]['value'] = $this->html2text($this->getDOMNodeInnerHTML($batch->ead_xpath->query("/ead/archdesc/did/abstract")->item(0) ) );

      // set dates if there are normalized iso dates
      $date_elements = $batch->ead_xpath->query("/ead/archdesc/did/unitdate[@normal]");
      if ($date_elements->length > 0){
        $this->node_set_ead_dates($date_elements, $collection_node);
      }


      // generate its body via an xslt transformation
      $collection_node->body = $this->getCollectionNodeBody($batch->getRaw(), $collection_node);

      // save the collection node
      node_save($collection_node);

      // create an entry for it in the child-ancestor table if there is not already a record (e.g. if re-processing)
      $result = db_result(db_query('SELECT ancestor_nid FROM {ead_import_child_ancestor} WHERE child_nid = "%s" AND ancestor_nid = %d', $collection_node->nid, $collection_node->nid));
      if (! $result){
        $record = array('child_nid' => $collection_node->nid, 'ancestor_nid' => $collection_node->nid);
        drupal_write_record('ead_import_child_ancestor', $record);
      }

      // get the collections vocabulary
      $collection_voc = ead_import_get_collection_vocabulary();
      $vocabulary_id = $collection_voc->vid;
      $batch->ead_collection_vocabulary_id = $vocabulary_id;
      
      // create a term for the collection
      $collection_term = array(
        'name' => $collection_node->title,
        'description' => '',
        'weight' => 0,
      );
      
      $collection_term['vid'] = $vocabulary_id;
      $collection_term['parent'] = array();
      $collection_term['relations'] = array();
      $collection_term['synonyms'] = '';


      taxonomy_save_term($collection_term);

      // assign the term to the collection
      taxonomy_node_save($collection_node, array($collection_term));

      // save the the collection path to our custom term path field
      $term_field_values = array('path' => "node/" . $collection_node->nid);
      if (module_exists('term_fields') && function_exists('term_fields_term_create_record')) {
	  term_fields_term_create_record($collection_term['tid'], $term_field_values);
      }


      // save the term id to the batch
      $batch->ead_collection_tid = $collection_term['tid'];
  
      // if we have not yet generated the xpaths-nids array, create it
      if ($batch->ead_xpaths_components == null){
      
        $this->create_ead_xpaths_components($batch);

        // set batch total
        $batch->setTotal(FEEDS_PROCESSING,count($batch->ead_xpaths));
      }


      // index the collection node if not running from command line.
      // Command line has issues w/ memory leaks
      if (php_sapi_name() != 'cli'){
        _node_index_node($collection_node);
      }
      
    }

    // Re-create the dom document. For some reason this needs to be recreated
    // for each iteration, can't just be assigned to batch.  Dont' know why.
    // But the xpath seems to 'forget' its dom document content
    $batch->ead_doc->loadXML($batch->getRaw());
    $batch->ead_xpath = new DOMXPath($batch->ead_doc);


    //
    // process component nodes in order of the xpaths list
    //
    
    // count how many components we process in this iteration
    $processed = 0;



    // resume at the next item to be processed
    for ($i = $batch->created; $i < $batch->getTotal(FEEDS_PROCESSING); $i++){



      // get the component's xpath
      $component_xpath = $batch->ead_xpaths[$i];

      // process the component
      $this->process_ead_component($component_xpath, $collection_node->nid, $batch);

      
      // increment created and processed
      $batch->created++;
      $processed++;

      // return the current progress if go over the batch size
      if ($processed >= variable_get('feeds_node_batch_size', FEEDS_NODE_BATCH_SIZE)) {

        return (1.0 / ($batch->getTotal(FEEDS_PROCESSING) + 1)) * ($batch->created); // Add + 1 to make sure that result is not 1.0 = finished.
      }

    }

    // update search totals
    search_update_totals();


    // Set messages.
    if ($batch->created) {
      drupal_set_message(t('Created !number !type nodes.', array('!number' => $batch->created, '!type' => $this->config['content_type'])));
    }
    else {
      drupal_set_message(t('There is no new content.'));
    }
    
    return FEEDS_BATCH_COMPLETE;

  }
  



  /**
   * Implementation of FeedsProcessor::clear().
   */
  public function clear(FeedsBatch $batch, FeedsSource $source) {

    if ($batch->getTotal(FEEDS_CLEARING)) {

      $num_items = db_result(db_query('SELECT COUNT(nid) FROM {feeds_node_item} WHERE id = "%s" AND feed_nid = %d', $source->id, $source->feed_nid));
	    $batch->setTotal(FEEDS_CLEARING, $num_items);
    }

    $result = db_query_range('SELECT nid FROM {feeds_node_item} WHERE id = "%s" AND feed_nid = %d ', $source->id, $source->feed_nid, 0, variable_get('feeds_node_batch_size', FEEDS_NODE_BATCH_SIZE));
    while ($node = db_fetch_object($result)) {

      // CUSTOM!! WATCH OUT!! -A. Dorsk, 2010-04-20
	    _ead_import_node_delete($node->nid);

      // delete entries from the child-ancestor table
      db_query('DELETE FROM {ead_import_child_ancestor} WHERE child_nid = %d', $node->nid);

	    $batch->deleted++;
    }
    if (db_result(db_query_range('SELECT nid FROM {feeds_node_item} WHERE id = "%s" AND feed_nid = %d', $source->id, $source->feed_nid, 0, 1))) {
	    return (1.0 / ($batch->getTotal(FEEDS_CLEARING) + 1)) * $batch->deleted;
    }

    // CUSTOM!!! A. Dorsk 2010-04-20


    // delete collection's entry in the child-ancestor table
    db_query('DELETE FROM {ead_import_child_ancestor} WHERE child_nid = %d', $source->feed_nid);

    // clear the source node's body
    $collection_node = node_load($source->feed_nid);
    $collection_node->body = "";
    node_save($collection_node);

    //
    // Delete collection's vocabulary terms
    
    // Get the collection's term.  It should be the first term.
    $collection_node_terms = taxonomy_node_get_terms($collection_node);

    $collection_node_term = array_shift($collection_node_terms);

    // Delete the term.  This should also delete the term's children.
    taxonomy_del_term($collection_node_term->tid);


    // Set message.
    drupal_get_messages('status');
    if ($batch->deleted) {
	    drupal_set_message(t('Deleted !number nodes.', array('!number' => $batch->deleted)));
    }
    else {
	    drupal_set_message(t('There is no content to be deleted.'));
    }
    return FEEDS_BATCH_COMPLETE;
  }



  /*
   * xpath: the xpath of the ead component
   * @todo: FUNCTION DOCUMENTATION HERE!!!
  */
  public function process_ead_component($xpath, $collection_nid, $batch){

    // shortcut to xpath object
    $xp = $batch->ead_xpath;

    // get the component object from its xpath
    $component = $xp->query("$xpath")->item(0);

    // create a node object for the component
    $node = new stdClass();

    // Populate and prepare node object
    // @todo: Think about node type, how to best code this in.  Depends on existing of ead_component content type
    $node->type = 'ead_component';
    $node->created = FEEDS_REQUEST_TIME;

    $node->feeds_node_item = new stdClass();
    $node->feeds_node_item->id = $this->id;
    $node->feeds_node_item->imported = FEEDS_REQUEST_TIME;
    $node->feeds_node_item->feed_nid = $collection_nid;
	
    static $included;
    if (!$included) {
	    module_load_include('inc', 'node', 'node.pages');
	    $included = TRUE;
    }

    node_object_prepare($node);


    // don't push the node to the front page
    $node->promote = 0;
      
    //
    // assign component's node properties
    //

    // base xpath for selecting component attributes.
    // This allows us to select immediate attributes without regard to how they are nested,
    // and avoids selecting attributes of child components, 
    // @todo: Combine this with the earlier ead_component_xpath_expression so that it's consolidated
    $ead_component_xpath_expression = "(starts-with(name(),'c0') or starts-with(name(),'c1') and string-length(name())=3)";
    $attribute_xpath_base = "./*[not($ead_component_xpath_expression)]/";

    // set the title to be the unittitle
    $node->title = $this->getTextViaXpathQuery("did/unittitle", $xp, $component);

    // set the body and description to be the abstract
    $node->body = $this->getTextViaXpathQuery("did/abstract", $xp, $component);
    $node->field_ead_abstract[0]['value'] = $this->getTextViaXpathQuery("did/abstract", $xp, $component);

    // Note: CCK fields need the extra field indices and 'value' index
    // set the date to be the unitdate

    // set the dates
    // need to parse from the 'normal' attribute, should like yyyy/yyyy.  Can also use dash for middle divider.

    // set date elements if they exist
    //$date_elements = $batch->ead_xpath->query($attribute_xpath_base . "unitdate[@normal]");
    $date_elements = $xp->query("did/unitdate[@normal]", $component);
    if($date_elements->length > 0){
	$this->node_set_ead_dates($date_elements, $node);
    }

    // set the box to be the container w/ box attribute
    $node->field_ead_component_box[0]['value'] = $this->getTextViaXpathQuery("did/container[@type='Box']", $xp, $component);

    // set the folder to be the container w/ box attribute
    $node->field_ead_component_folder[0]['value'] = $this->getTextViaXpathQuery("did/container[@type='Folder']", $xp, $component);

    // set the folder to be the level attribute value
    $node->field_ead_component_level[0]['value'] = $component->getAttribute("level");

    // set the component id to be created counter
    // this assumes that anchors are also being created depth-first
    // in the finding aid collection's body html
    $component_id = $batch->created + 1;
    $node->field_ead_component_component_id[0]['value'] = $component_id;

    // set the collection id
    $node->field_ead_component_collection[0]['nid'] = $collection_nid; // for noderef cck field

    // set the node's path
    $path = "node/$collection_nid#ead_component_$component_id";

    $node->field_ead_component_path[0]['value'] = $path;

    // save the node
    node_save($node);

    // save the component's data to its corresponding object in the $batch object
    $batch->ead_xpaths_components[$xpath]['nid'] = $node->nid;

    // get ancestor nids, if node has ancestors
    // note: this assumes that these ancestors have already been created, which they
    // will have been since we're creating nodes in top-down depth-first order
    $ancestor_nids = array();
    $ancestor_xpaths = $batch->ead_xpaths_components[$xpath]['ancestor_xpaths'];


    // get the number of ancestors
    $num_ancestors = count($ancestor_xpaths);

    // if we have ancestors
    if ( $num_ancestors > 0 ){
      // save the nid of each ancestor
      foreach ($ancestor_xpaths as $ancestor_xpath){
        $ancestor_nids[] = $batch->ead_xpaths_components[$ancestor_xpath]['nid'];
      }
    }

    // add node's id to the ancestor ids and
    // create entries in the child_ancestor table
    // note: we count a node as an ancestor of itself for search purposes
    foreach ($ancestor_nids as $ancestor_nid){
      $record = array('child_nid' => $node->nid, 'ancestor_nid' => $ancestor_nid);
      drupal_write_record('ead_import_child_ancestor', $record);
    }

    
    //
    // tag the component's node
    //

    // start a list of term ids, beginning with the collection's term id
    $tids = array($batch->ead_collection_tid);

    // if component has ancestors...
    if ( $num_ancestors > 0){
      // save term for each ancestor if ancestor has a term
      // note: every ancestor should have a term
      foreach ($ancestor_xpaths as $ancestor_xpath){
          $tids[] = $batch->ead_xpaths_components[$ancestor_xpath]['tid'];
      }
    }
     
    // if the item has child components, create a term for it
    if ($xp->query("./*[$ead_component_xpath_expression]", $component)->length > 0){

      // if component has a parent, get the parent's term in order to place the
      // component's term in the term hierarchy
      // note: parent term will be the last in the list of ancestor terms,

	    $parent_tid = end($tids);

	    // create a term for the item
	    $item_term = array(
        'name' => $node->title,
        'description' => $node->title,
        'weight' => $batch->created + 1,
	    );
	    $item_term['vid'] = $batch->ead_collection_vocabulary_id;
	    $item_term['parent'] = array($parent_tid);
	    $item_term['relations'] = array();
	    $item_term['synonyms'] = '';

      taxonomy_save_term($item_term);

      // set term path field (depends on term field module)
      // should path to the node's path
      $term_field_values = array('path' => $node->field_ead_component_path[0]['value']);
      if (module_exists('term_fields') && function_exists('term_fields_term_create_record')) {
        term_fields_term_create_record($item_term['tid'], $term_field_values);
      }
	    
      // save the item's term id to the list of terms the item will be tagged with
      $term_ids[] = $item_term['tid'];

      // save the component's data to its corresponding object in the $batch object
      $batch->ead_xpaths_components[$xpath]['tid'] = $item_term['tid'];

      // save item's term to the list of term ids
      $tids[] = $item_term['tid'];

    }
	
    // assign terms to the item
    taxonomy_node_save($node, $tids);

    // index the node if not running from command line
    // Command line has memory leaks
    if (php_sapi_name() != 'cli'){
      _node_index_node($node);
    }

  }


  /*
   get collection node body text by doing an xslt transform
   @todo: FUNCTION DOCUMENTATION HERE!!!
  */
  function getCollectionNodeBody($xml_string, &$collection_node){
     
    // create an XSLT processor
    $processor = new XSLTProcessor();
 
    // load xslt files
    $xsl_dir = drupal_get_path('module', 'ead_import') . "/libraries/whoi_ead_xslt";

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

    // set parameters

    // if collection has an image use it, otherwise use the default image
    $collection_image_filepath = $collection_node->field_ead_collection_image[0]['filepath'];
    if (! $collection_image_filepath){
      $field = content_fields('field_ead_collection_image', 'ead_collection');
      $collection_image_filepath = $field['widget']['default_image']['filepath'];
    }

    // create the full image path by appending the base path
    $collection_image_filepath = implode("/", array(base_path(), $collection_image_filepath));

    // set the collection image path parameter for the xslt transform
    $processor->setParameter('', "collection_image_filepath", $collection_image_filepath);

    // run the transformation
    $result_doc = $processor->transformToDoc($source_doc);

    // output html body content w/ head and enclosing tags removed

    $xp = new DOMXPath($result_doc);

    $html_node = $xp->query("/html")->item(0);
    $head_node = $xp->query("/html/head")->item(0);
    $html_node->removeChild($head_node);

    $body_html = preg_replace('/^<!DOCTYPE.+?>|<(\/)?html>|<(\/?)body>/', '', $result_doc->saveHTML());

    return $body_html;
      
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
   * helper function for getting full inner html of a DOM node as UTF-8 text
   */
  function html2text($html){
    return html_entity_decode(strip_tags($html), ENT_COMPAT, 'UTF-8');
  }

  /**
   * helper function for getting text from xpath query result
   * @todo: Clean this up!  This is kludgy!
   */
  function getTextViaXpathQuery($query,$xp, $dom_node){
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
   * helper function to create ead DOM objects
   * @todo: Clean this up!  This is kludgy!
   */
  function create_ead_dom($batch){

    // make a DOM object from the batch object's raw xml string
    $dom_doc = new DOMDocument();

    if (! $dom_doc->loadXML($batch->getRaw()) ){
	    return false;      
    }

    // if we could make a dom document...
    else{
      // save the document to an attribute
      $batch->ead_doc = $dom_doc;

      // make an xpath object to use for xpath queries
      $batch->ead_xpath = new DOMXPath($batch->ead_doc);

    }

    return true;

  }


  /**
   * helper function to create array of ead component nids, keyed by xpaths
   * nids will be added as we process.  In this function we just initialize the xpaths.
   * @todo: Clean this up!  This is kludgy!
   */
  function create_ead_xpaths_components($batch){
    
    // initialize the xpaths-component array
    $batch->ead_xpaths_components = array();

    // initialize the xpaths array
    $batch->ead_xpaths = array();

    // populate array with xpaths, in depth-first order
    // note: php xpath queries appear to return in depth-first, but this may not be always true?

    $ead_component_nodes = $batch->ead_xpath->query("/ead/archdesc/dsc//*[(starts-with(name(),'c0') or starts-with(name(),'c1') and string-length(name())=3)]");

    foreach ($ead_component_nodes as $node){
      $xpath = $node->getNodePath();

      // append to the ead_xpaths ordered list
      $batch->ead_xpaths[] = $xpath;

      // create a component object
      $ead_component = array(
        nid => '', // component's drupal node id, this will be set later
        ancestor_xpaths => $this->getAncestorXpaths($batch, $node), // xpaths of parent components, this may be set later
        tid => '', // component's term id, this may be set later
      );

      // append the component object to the ead_xpaths_components array
      $batch->ead_xpaths_components[$xpath] = $ead_component;
    }

  }

  function getAncestorXpaths($batch, $component_node){
    
    // list of ancestor xpaths to be populate
    $ancestor_xpaths = array();

    // try to get the component's ancestor components
    $ancestor_node_list = $batch->ead_xpath->query("ancestor::*[(starts-with(name(),'c0') or starts-with(name(),'c1') and string-length(name())=3)]", $component_node);

    // if the component has no ancestor components...
    if ($ancestor_node_list->length > 0){

      // add their xpaths to the list
      foreach ($ancestor_node_list as $ancestor_node){
        $ancestor_xpaths[] = $ancestor_node->getNodePath();
      }

    }

    // return the list of ancestor xpaths
    return $ancestor_xpaths;

  }

  /*
   * Helper function to format ISO 8601 date string into drupal-compatible date strings
   * will return a singleton array, or an array (start,end) if the iso date is an interval
   */
  function convert_ISO_date_to_drupal_dates($iso_date) {
    
      // array to hold converted dates
      $utc_dates = array();
      
      // first split the iso date (it could be an interval)
      $iso_dates = explode("/", $iso_date);
      
      foreach ($iso_dates as $iso_date){

	  // skip if not a valid iso date
	  if (! $this->is_valid_iso_date($iso_date)){
	      next;
	  }
	  
	  // if date is just a year, add on month and day so that php 
	  // can recognize it
	  if (preg_match('/^\d{4}$/', $iso_date)){
	      $iso_date = sprintf("%s-01-01", $iso_date);
	  }

	  // convert date into PHP DateTime object
	  $date_obj = new DateTime($iso_date);

	  // Reformat the date into a full ISO date that date_make_date() will accept                 
	  $iso_date_string = date_format_date($date_obj, 'custom', 'Y-m-d\TH:i:s');

	  // Create a DateTime object with UTC timezone
	  $utc_date = date_make_date($iso_date_string, 'UTC', DATE_DATETIME);

	  // Format the UTC version of the DateTime
	  $utc_date_string = date_format_date($utc_date, 'custom', 'Y-m-d\TH:i:s');

	  // save the date
	  $utc_dates[] = $utc_date_string;
      }

      // return the array of dates
      return $utc_dates;

  }

  function is_valid_iso_date($iso_date){
      $validation_regex = '/^([\+-]?\d{4}(?!\d{2}\b))((-?)((0[1-9]|1[0-2])(\3([12]\d|0[1-9]|3[01]))?|W([0-4]\d|5[0-2])(-?[1-7])?|(00[1-9]|0[1-9]\d|[12]\d{2}|3([0-5]\d|6[1-6])))([T\s]((([01]\d|2[0-3])((:?)[0-5]\d)?|24\:?00)([\.,]\d+(?!:))?)?(\17[0-5]\d([\.,]\d+)?)?([zZ]|([\+-])([01]\d|2[0-3]):?([0-5]\d)?)?)?)?$/';

      return preg_match($validation_regex, $iso_date);
  }  

  /*
   * Helper function to set ead dates for an ead node 
   * (collection or component)
   */
  function node_set_ead_dates($date_elements, $node){

      // counter for multiple date elements
      $date_counter = 0;
      
      // process each date element
      foreach ($date_elements as $date_element){

	  // skip if there is a normalized date
	  if (! $date_element->hasAttribute("normal")){
	      next;
	  }

	  // get the normalized iso date string
	  $raw_iso_date_string = $date_element->getAttribute("normal");

	  // convert the raw iso date string into a drupal-compatible date(s)
	  // note: iso dates can specify an interval, and thus this function
	  // can return either a singleton (date) array, or a (start_date, end_date) array
	  $drupal_dates = $this->convert_ISO_date_to_drupal_dates($raw_iso_date_string);

	  // if dates were parsed successfully,
	  // save them to the current ead_date field
	  if ($drupal_dates && count($drupal_dates) > 0){

	      $node->field_ead_dates[$date_counter]['value'] = $drupal_dates[0];
	      
	      // if the date ss an interval and a second date exists, save it 
	      // to the drupal 'to' date
	      if ($drupal_dates[1] != NULL){
		  $node->field_ead_dates[$date_counter]['value2'] = $drupal_dates[1];      
	      }
	  }

	  // increment the date counter
	  $date_counter++;
      }
  }

}




/**
 * Copy of node_delete() that circumvents node_access().
 *
 * Used for batch deletion.
 */
function _ead_import_node_delete($nid) {

  $node = node_load($nid);

  db_query('DELETE FROM {node} WHERE nid = %d', $node->nid);
  db_query('DELETE FROM {node_revisions} WHERE nid = %d', $node->nid);

  // Call the node-specific callback (if any):
  node_invoke($node, 'delete');
  node_invoke_nodeapi($node, 'delete');

  // Clear the page and block caches.
  cache_clear_all();

  // Remove this node from the search index if needed.
  if (function_exists('search_wipe')) {
    search_wipe($node->nid, 'node');
  }
  watchdog('content', '@type: deleted %title.', array('@type' => $node->type, '%title' => $node->title));
  drupal_set_message(t('@type %title has been deleted.', array('@type' => node_get_types('name', $node), '%title' => $node->title)));
}
