<?php
// $Id$

/**
 * @file
 * Breakout functions for ead_import enable

 * Creates EAD content types from EAD mapping file in top-level EAD module directory
 */

/**
 * Ead_import enable breakout function.  Creates EAD content types.
 *
 */
function _ead_import_enable() {

    //drush_print("yo");

    // create content types
    _create_ead_content_types();

}

/**
 * Helper function for creating ead content types.
 */
function _create_ead_content_types() {

    // Load the ead_drupal_mapping variables by including the mapping file.
    require_once ( dirname(__FILE__) . '/ead_drupal_mapping.inc');

    dvm("mapping is: $ead_collection_drupal_mapping");

    // Use the mapping to create a collection content type definition string
    _create_content_type_definition_from_ead_mapping($ead_collection_drupal_mapping);

// create a content type from the definition string
//_create_content_type_from_definition_file($definition_file_path);

}


/**
 * @param EAD_mapping_array $ead_mapping
 * Helper function for creating a Drupal content type export string from an
 * ead mapping.
 */
function _create_content_type_definition_from_ead_mapping($ead_mapping) {

    // Create the content array
    $content = array();

    // set the basic node properties
    $content['type']  = array (
        'name' => $ead_mapping['content_type']['label'],
        'type' => $ead_mapping['content_type']['type'],
        'description' => $ead_mapping['content_type']['description'],
        'title_label' => $ead_mapping['fields']['title']['definition']['label'],
        'body_label' =>  $ead_mapping['fields']['body']['definition']['label'],
        'min_word_count' => '0',
        'help' => '',
        'node_options' =>
        array (
        'status' => true,
        'promote' => false,
        'sticky' => false,
        'revision' => false,
        ),
        'old_type' => $ead_mapping['content_type']['type'],
        'orig_type' => '',
        'module' => 'node',
        'custom' => '1',
        'modified' => '1',
        'locked' => '0',
        'comment' => '2',
        'comment_default_mode' => '4',
        'comment_default_order' => '1',
        'comment_default_per_page' => '50',
        'comment_controls' => '3',
        'comment_anonymous' => 0,
        'comment_subject_field' => '1',
        'comment_preview' => '1',
        'comment_form_location' => '0',
    );

    // @todo: Do other stuff for CCK fields here.


    // Make the content type definition into a string
    $definition_string = '$content = ' . var_export($content, TRUE) . ';';

    // Create the content type from the definition string.
    _create_content_type_from_definition_string($definition_string);

}


/**
 * @@param string $definition_string
 * Helper function for creating content types from definition strings,
 * as per the content/types/export form.
 */
function _create_content_type_from_definition_string($definition_string) {

    // load the node/content .inc files
    module_load_include('inc', 'node', 'content_types');
    module_load_include('inc', 'content', 'content_admin');

    $form_state = array();
    $form_state['values']['type_name'] = '<create>';
    $form_state['values']['macro'] = $definition_string;

    drupal_execute('content_copy_import_form', $form_state);

    content_clear_type_cache();
}



