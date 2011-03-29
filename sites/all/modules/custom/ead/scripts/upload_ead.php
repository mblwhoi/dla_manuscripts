#!/usr/bin/php -q
<?php
 
   // set some server variables so Drupal doesn't freak out
$_SERVER['SCRIPT_NAME'] = '/script.php';
$_SERVER['SCRIPT_FILENAME'] = '/script.php';
$_SERVER['HTTP_HOST'] = 'dev.lighthouse.whoi.edu';
$_SERVER['REMOTE_ADDR'] = '127.0.0.1';
$_SERVER['REQUEST_METHOD'] = 'POST';
 
// act as the first user
global $user;
$user->uid = 1;
 
// change to the Drupal directory
chdir('/home/adorsk/projects/finding_aids/drupal');
 
// Drupal bootstrap throws some errors when run via command line
//  so we tone down error reporting temporarily
error_reporting(E_ERROR | E_PARSE);
 
// run the initial Drupal bootstrap process
require_once('includes/bootstrap.inc');
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
 
// restore error reporting to its normal setting
error_reporting(E_ALL);

// include node file, necessary for node generation
module_load_include('inc', 'node', 'node.pages');

function create_ead_node_from_file($ead_file, $system_file_dir_path, $drupal_file_dir_path) {

  // get the ead file name
  $ead_file_basename = basename($ead_file);

  // copy file to drupal's file directory on the system
  copy($ead_file, $system_file_dir_path . "/" . $ead_file_basename);

  // create $node
  $node = new stdClass();
  $node->type = 'ead_collection';

  $form_state = array();

  //$form_state['values']['type'] = 'story'; // the type of the node to be created
  $form_state['values']['type'] = 'ead_collection'; // the type of the node to be created
  $form_state['values']['status'] = 1; // set the node's status to Published, or set to 0 for unpublished
  //$form_state['values']['title'] = '';   // the node's title
  $form_state['values']['body'] = 'whatever body you might want, or else, leave blank'; // the body, not required

  // create feeds value array
  //$path = "sites/default/files/feeds/AC-66_Logbooks_reduced.xml";
  $feeds = array(
    'EadImportFeedsFileFetcher' => array(
      'source' => $drupal_file_dir_path . "/" . $ead_file_basename,
    ),
  );

  $form_state['values']['feeds'] = $feeds; 
  
  $form_state['values']['name'] = 'admin'; 
  $form_state['values']['op'] = t('Save');  // this seems to be a required value

  // create node using drupal_execute
  drupal_execute('ead_collection_node_form', $form_state, $node);

  $errors = form_get_errors();

  if (count($errors)) {
    foreach ($errors as $error){
      print "error is: $error\n";
    }
  }

}

$ead_file = $argv[1];
$system_file_dir = $argv[2];
$drupal_file_dir = $argv[3];

create_ead_node_from_file($ead_file, $system_file_dir, $drupal_file_dir);

print "upload memory usage is: " . memory_get_usage(true) . "\n";
print "done w/ uplead_ead.php\n";
