<?php

// $Id: EadImportFeedsFileFetcher.inc

/**
 * @file
 * Home of the EadImportFeedsFileFetcher and related classes.
 */

/**
 * Definition of the EAD import batch object created on the fetching stage by
 * EadImportFeedsFileFetcher.
 */
class EadImportFeedsFileBatch extends FeedsFileBatch {

  // counter for current component id
  public $ead_component_id;

  // ead DOM doc object
  public $ead_doc;

  // xpath for search the ead doc
  public $ead_xpath;

  // vocabulary id of the collection's vocabulary
  public $ead_collection_vocabulary_id;

  // term id of the collection's term
  public $ead_collection_tid;

  // array for ead component xpaths
  // this array is used to resume our place during batch processing
  public $ead_xpaths;

  // array for ead component xpath-component pairs, keyed by xpath.
  // components are arrays representing components (nid, etc...)
  public $ead_xpaths_components;

  // xpath of the current ead parent
  public $ead_current_parent_xpath;


  /**
   * Constructor.
   */
  public function __construct($file_path) {
    parent::__construct($file_path);

    // initialize component id
    $this->ead_component_id = 0;

  }

  }

/**
* Fetches EAD Files
*/
class EadImportFeedsFileFetcher extends FeedsFileFetcher {

  /**
   * Implementation of FeedsFetcher::fetch().
   */
  public function fetch(FeedsSource $source) {
    $source_config = $source->getConfigFor($this);
    return new EadImportFeedsFileBatch($source_config['source']);
  }
}