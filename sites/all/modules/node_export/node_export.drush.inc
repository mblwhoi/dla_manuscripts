<?php
// $Id: node_export.drush.inc,v 1.1.2.8 2010/01/26 03:03:39 danielb Exp $

/**
 * @file
 *   Drush support for node_export.
 */

/**
 * Implementation of hook_drush_command().
 */
function node_export_drush_command() {
  $items = array();

  $items['node_export'] = array(
    'callback' => 'node_export_drush_callback_export',
    'description' => "Export nodes.",
    'arguments' => array(
      'node ids' => 'A list of node ids to export.',
    ),    
    'options' => array(
      '--file' => "A filename to write the node data to.",
    ),
    'examples' => array(
      'drush node_export 45 46 47 --file=filename' => 
        'Export nodes 45, 46, and 47 to given filename.',
    ),
    'drush node_export 45 46 47 > filename' => 
      'Export nodes 45, 46, and 47 to given filename. (Runs the risk of error messages going to that file.)',
  );
  $items['node_export import'] = array(
    'callback' => 'node_export_drush_callback_import',
    'description' => "Import node code from a previous export.",
    'options' => array(
      '--uid' => "Uid of user to save nodes as. If not given will use 1. You may specify 0 for the Anonymous user.",
    ),
    'examples' => array(
      'drush node_export import < filename' => 
        'Import nodes from given filename.',
      'drush node_export import --uid=2 < filename' => 
        'Import nodes from given filename, saving them with uid 2 as author.',
    ),
  );
  $items['node_export type'] = array(
    'callback' => 'node_export_drush_callback_export_type',
    'description' => "Export all nodes in the given content type.",
    'arguments' => array(
      'type' => 'The content type to export.',
    ),    
    'options' => array(
      '--file' => "A filename to write the node data to.",
    ),
    'examples' => array(
      'drush node_export type story --file=filename' => 
        'Export all nodes of type \'story\' to given filename.',
    ),
    'drush node_export type story > filename' => 
      'Export all nodes of type \'story\' to given filename. (Runs the risk of error messages going to that file.)',
  );

  return $items;
}

/**
 * Implementation of hook_drush_help().
 *
 * This function is called whenever a drush user calls
 * 'drush help <name-of-your-command>'
 *
 * @param
 *   A string with the help section (prepend with 'drush:')
 *
 * @return
 *   A string with the help text for your command.
 */
function node_export_drush_help($section) {
  switch ($section) {
    case 'drush:node_export':
      return dt("Exports nodes. Usage: 'drush node_export > filename'.");
    case 'drush:node_export import':
      return dt("Imports nodes that have been exported. Usage: 'drush node_export import < filename'.");
  }
}

/**
 * Drush command callback.
 *
 * Export nodes.
 */
function node_export_drush_callback_export() {
  $commands = func_get_args();
  
  $nids = array_filter($commands, 'is_numeric');
  
  $data = node_export_node_bulk($nids, TRUE);
  
  $filename = drush_get_option('file');
  
  if ($filename) {
    // Output data to file. Note this only takes a flat filename for the current directory.
    // If file exists, ask for whether to overwrite
    if (file_exists($filename)) {
      if (!drush_confirm(dt('File ' . $filename . ' exists. Do you really want to overwrite?'))) {
        return; 
      }
    }
    // Write the file.
    file_put_contents($filename, $data);
  }
  else {
    // Print to terminal.
    drush_print_r($data);   
  }
}

/**
 * Drush command callback.
 *
 * Import nodes from data.
 */
function node_export_drush_callback_import() {
  $commands = func_get_args();
    
  // Switch to admin or the specified user so imported nodes are not anonymous.
  $uid = drush_get_option('uid');
  // Test on NULL so uid may be given as 0.
  if (is_null($uid)) {
    $uid = 1;
  }
  // uid 0 is already loaded.
  if ($uid != 0) {
    global $user;
    $user = user_load($uid);
  }
  
  $node_code = file_get_contents("php://stdin", "r");

  if (!empty($node_code)) {
    $result = node_export_import($node_code, 'save-edit', FALSE, 'drush_print', 'dt');
    if ($result === FALSE) {
      // There was a problem with types?
      drush_set_error('DRUSH_NOT_COMPLETED', 'Problem found with the import file. Check the node types exist.');
    }
  }

}

/**
 * Drush comand callback.
 *
 * Export nodes of the provided type.
*/
function node_export_drush_callback_export_type($type) {
  if ($type == NULL) {
    drush_set_error('DRUSH_NOT_COMPLETED', 'Please supply the type of nodes you want to export.');
  }
  $file = drush_get_option('file');
  
  $nids = array();
  $sql = "SELECT nid FROM {node} WHERE type = '%s'";
  $rs = db_query($sql, $type);
  
  while ($row = db_fetch_array($rs)) {
    $nids[] = $row['nid'];
  }
  
  if (!empty($nids)) {
    call_user_func_array('node_export_drush_callback_export', $nids);
  }
  else {
    drush_set_error('DRUSH_NOT_COMPLETED', "No nodes found of type $type.");
  }
}

