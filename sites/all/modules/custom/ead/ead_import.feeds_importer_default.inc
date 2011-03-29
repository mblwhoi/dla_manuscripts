<?php
// $Id$

/**
 * @file
 * Breakout functions for ead_import feeds_importer_default

 * Define Feeds Import configuration for EAD imports.
 */

/**
 * Ead_import feeds_importer_default breakout function.  Defines EAD Feeds importer configuration.
 *
 */
function _ead_import_feeds_importer_default(){

  $export = array();
  $feeds_importer = new stdClass;
  $feeds_importer->disabled = FALSE;
  $feeds_importer->api_version = 1;
  $feeds_importer->id = 'ead_importer';
  $feeds_importer->config = array(
    'name' => 'EAD Importer',
    'description' => 'EAD Importer',
    'fetcher' => array(
      'plugin_key' => 'EadImportFeedsFileFetcher',
      'config' => array(
        'auto_detect_feeds' => FALSE,
      ),
    ),

    'parser' => array(
      'plugin_key' => 'EadImportFeedsParser',
      'config' => array(),
    ),

    'processor' => array(
      'plugin_key' => 'EadImportFeedsCollectionNodeProcessor',
      'config' => array(
        'content_type' => 'feed_item',
        'update_existing' => 0,
        'expire' => '-1',
        'mappings' => array(
          '0' => array(
            'source' => 'title',
            'target' => 'title',
            'unique' => FALSE,
          ),
          '1' => array(
            'source' => 'description',
            'target' => 'body',
            'unique' => FALSE,
          ),
          '2' => array(
            'source' => 'timestamp',
            'target' => 'created',
            'unique' => FALSE,
          ),
          '3' => array(
            'source' => 'url',
            'target' => 'url',
            'unique' => TRUE,
          ),
          '4' => array(
            'source' => 'guid',
            'target' => 'guid',
            'unique' => TRUE,
          ),
        ),
      ),
    ),
    'content_type' => 'feed',
    'update' => 0,
    'import_period' => '1800',
    'expire_period' => 3600,
    'import_on_create' => 1,
  );
  $export['ead_importer'] = $feeds_importer;
  return $export;

  }


