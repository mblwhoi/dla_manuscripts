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


// update 100 nodes at a time
$limit = 10;

#$result = db_query_range("SELECT n.nid FROM {node} n LEFT JOIN {search_dataset} d ON d.type = 'node' AND d.sid = n.nid WHERE d.sid IS NULL OR d.reindex <> 0 ORDER BY d.reindex ASC, n.nid ASC", 0, $limit);

$result = db_query_range("SELECT n.nid FROM {node} n where n.type='ead_component' ORDER BY n.nid ASC", 0, $limit);      

while ($row = db_fetch_object($result)) {
    $node = node_load($row->nid);
    #_node_index_node($node);

        // Build the node body.                                                     
    $node->build_mode = NODE_BUILD_SEARCH_INDEX;
    $node = node_build_content($node, FALSE, FALSE);

    $node->body = drupal_render($node->content);

    $text = '<h1>' . check_plain($node->title) . '</h1>' . $node->body;

    #print "t is: $text\n";

    // Fetch extra data normally not visible                                    
    $extra = node_invoke_nodeapi($node, 'update index');
    foreach ($extra as $t) {
        $text .= $t;
    }

    #print "t: $text\n";


}



