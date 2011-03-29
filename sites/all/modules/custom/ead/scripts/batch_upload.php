#!/usr/bin/php -q
<?php

  /*
This is seriously hacky and ugly.  Need to fix this later.  Beacuse there are issues w/ iterative calls to drupal_execute we pass to a secondary script that creates a new drupal environment for each file.  Seems to work...
   */

$max_memory = "256M";

//ini_set('memory_limit', $max_memory_mb . "M");

$upload_script = "/home/adorsk/projects/finding_aids/drupal/sites/all/modules/custom/ead/scripts/upload_ead.php";

$search_index_script = "/home/adorsk/projects/finding_aids/drupal/sites/all/modules/custom/ead/scripts/update_search_index.php";
 
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

//require_once("execute_fix.php");

/*
 * function to create collection node from an xml file
 * copies file to the feeds file directory, creates node from file
 */
function create_ead_node_from_file($ead_file, $system_file_dir_path, $drupal_file_dir_path) {

  // get the ead file name
  $ead_file_basename = basename($ead_file);

  // copy file to drupal's file directory on the system
  copy($ead_file, $system_file_dir_path . "/" . $ead_file_basename);

  $form_state = array();

  //$node = new stdClass();

  //$form_state['values']['type'] = 'story'; // the type of the node to be created
  $form_state['values']['type'] = 'ead_collection'; // the type of the node to be created
  $form_state['values']['status'] = 1; // set the node's status to Published, or set to 0 for unpublished
  $form_state['values']['title'] = 'prgrm collection test';   // the node's title
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
  drupal_execute('ead_collection_node_form', $form_state);

  $errors = form_get_errors();

  if (count($errors)) {
    foreach ($errors as $error){
      print "error is: $error\n";
    }
  }

  // Flush static storage.
  drupal_validate_form('', NULL, $form_state);

}



/*
 * Helper function to get a list of files in a directory
 */
function get_files_list($dir)
{
  $files = array();
  if($handler = opendir($dir)) {
    while (($sub = readdir($handler)) !== FALSE) {
      if ($sub != "." && $sub != ".." && $sub != "Thumb.db") {
        if(is_file($dir."/".$sub)) {
          $files[] = $dir . "/" . $sub;
        }
      }
    }   
    closedir($handler);
  }
  return $files;   
} 


/*
 * batch callback to run drupal_execute
 */
function batch_callback($ead_file, $system_file_dir, $drupal_file_dir){

  print "processing file:\n\t $ead_file\n\n";
  create_ead_node_from_file($ead_file, $system_file_dir, $drupal_file_dir);
}

/*
 * function to do next iteration of batch
*/
function batch_iterate(){
  global $counter;
  global $ead_files;
  
  // stop if we've gone through all the files
  if ($counter == count($ead_files)){
    print "\n\nDone!\n";
    return;
  }
  // otherwise start the next batch w/ the current ead file
  else{

    print "iterating...\n";

    $ead_file = $ead_files[$counter];

    $counter++;

    // set the ead file dir
    $drupal_file_dir = "sites/default/files/ead_files";

    $system_file_dir = "/home/adorsk/projects/finding_aids/drupal/$drupal_file_dir";

    $batch = array(
      'title' => 'Creating Collection Programatically',
      'operations' => array(
        array('batch_callback', array($ead_file, $system_file_dir, $drupal_file_dir)),
      ),
      'finished' => 'batch_iterate',
      'progress_message' => '',
      );
    batch_set($batch);    
  }
}


// global counter
$counter = 0;

// get a list of files from a directory
$ead_dir = '/home/adorsk/projects/finding_aids/findingaids/Administrative Collections/EAD Finding Aids/';

global $ead_files;
$ead_files = get_files_list($ead_dir);

// TESTING!!
#$ead_files = array($ead_files[0], $ead_files[1]);
//$ead_files = array($ead_files[2], $ead_files[1]);

// start batch
//batch_iterate();


// set ead file dir paths
$drupal_file_dir = "sites/default/files/ead_files";
$system_file_dir = "/home/adorsk/projects/finding_aids/drupal/$drupal_file_dir";

// upload a bunch of files

foreach($ead_files as $ead_file){
  print "processing file:\n\t $ead_file\n\n";

  //create_ead_node_from_file($ead_file, $system_file_dir, $drupal_file_dir);
  // hack to get around drupal issues w/ drupal_execute
  #$cmd = "php -d memory_limit=$max_memory $upload_script \"$ead_file\" \"$system_file_dir\" \"$drupal_file_dir\""; 
  $cmd = "php $upload_script \"$ead_file\" \"$system_file_dir\" \"$drupal_file_dir\""; 
  system($cmd);

  print "batch upload memory usage is: " . memory_get_usage(true) . "\n";

}

// update search index for new nodes
// we call an external script to get around memory leak issues w/
// CLI and drupal indexing
#while ($num_unindexed_nodes = db_fetch_object(db_query_range("SELECT count(n.nid) FROM {node} n LEFT JOIN {search_dataset} d ON d.type = 'node' AND d.sid = n.nid WHERE d.sid IS NULL OR d.reindex <> 0 ORDER BY d.reindex ASC, n.nid ASC", 0, 100))) {
$i = 0;
//while( ($num_unindexed_nodes = db_fetch_object(db_query_range("SELECT count(n.nid) FROM {node} n LEFT JOIN {search_dataset} d ON d.type = 'node' AND d.sid = n.nid WHERE d.sid IS NULL OR d.reindex <> 0 ORDER BY d.reindex ASC, n.nid ASC", 0, 100))->{'count(n.nid)'}) && ($i < 2)){
while($num_unindexed_nodes = db_fetch_object(db_query_range("SELECT count(n.nid) FROM {node} n LEFT JOIN {search_dataset} d ON d.type = 'node' AND d.sid = n.nid WHERE d.sid IS NULL OR d.reindex <> 0 ORDER BY d.reindex ASC, n.nid ASC", 0, 100))->{'count(n.nid)'}){

    print "Number of unindexed nodes remaining: $num_unindexed_nodes\n";
    $cmd = "php $search_index_script";
    system($cmd);
    print "\tmemory usage for indexing is: " . memory_get_usage(true) . "\n";
    $i++;
}


