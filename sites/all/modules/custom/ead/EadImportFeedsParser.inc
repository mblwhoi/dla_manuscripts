<?php
// $Id: EadImportFeedsParser.inc

/**
 * Parses a given file as an EAD file.
 */
class EadImportFeedsParser extends FeedsParser {

  /**
   * Implementation of FeedsParser::parse().
  */

  public function parse(FeedsImportBatch $batch, FeedsSource $source) {

    // require the ead_parser library
    require_once('./'. drupal_get_path('module', 'ead_import') ."/libraries/ead_parser.inc");

    // Parse.
    // For right now, we don't do anything except set a dummy title if things go wrong.  All parsing happens in the processor.

    $batch->setTitle("untitled");

  }


  /**
   * Return mapping sources.
   *
   */
  public function getMappingSources() {
    return array(
      'title' => array(
        'name' => t('Title'),
        'description' => t('Title of the feed item.'),
      )
    );
  }
  }