<?php
// $Id$

/**
 * @file
 * A mapping from EAD field names to value functions, drupal field names,
 * and drupal field definitions.
 *
 * This function is used by several EAD modules to create EAD content types
 * and map values from parsed EAD xml files to Drupal EAD nodes.
 */

// Create mapping arrays for each content type
$ead_collection_drupal_mapping = array(

    // Define content type metadata
    content_type => array(
    label => "EAD Collection",
    type => "ead_collection",
    description => "EAD Collection"
    ),

    // Define field mappings, keyed by field name
    fields => array(

    // Title.  Required.
    title => array(
    value_function => "",
    definition => array(
    'label' => "Collection Name"
    ),
    ),

    // Title.  Required.
    body => array(
    value_function => "",
    definition => array(
    'label' => "Body"
    ),
    )
    
    )
);