<?php
// $Id$

/**
 * @file
 * Breakout functions for ead_import feeds_plugins

 * Define Feeds Plugin definitions for importing EAD files
 */

/**
 * Ead_import feeds_plugins breakout function.  Defines EAD Feeds plugins.
 *
 */
function _ead_import_feeds_plugins() {


    $info = array();

    // Define EadImportFeedsFileFetcher
    $info['EadImportFeedsFileFetcher'] = array(
        'name' => 'EAD Import File Fetcher',
        'description' => 'Fetches EAD files',
        'handler' => array(
        'parent' => 'FeedsFileFetcher',
        'class' => 'EadImportFeedsFileFetcher',
        'file' => 'EadImportFeedsFileFetcher.inc',
        'path' => drupal_get_path('module', 'ead_import'),
        ),
    );

    // Define parser.
    $info['EadImportFeedsParser'] = array(
        'name' => 'Ead Import Parser',
        'description' => 'Parses EAD Files',
        'handler' => array(
        'parent' => 'FeedsParser',
        'class' => 'EadImportFeedsParser',
        'file' => 'EadImportFeedsParser.inc',
        'path' => drupal_get_path('module', 'ead_import'),
        ),
    );

    // Define processor.
    $info['EadImportFeedsCollectionNodeProcessor'] = array(
        'name' => 'EAD Collection Node Processor',
        'description' => 'Updates an EAD Collection node and creates child EAD item nodes.',
        'handler' => array(
        'parent' => 'FeedsProcessor',
        'class' => 'EadImportFeedsCollectionNodeProcessor',
        'file' => 'EadImportFeedsCollectionNodeProcessor.inc',
        'path' => drupal_get_path('module', 'ead_import'),
        ),
    );

    return $info;

}




