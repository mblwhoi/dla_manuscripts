<?php
// $Id$

/**
 * Implementation of hook_views_query_alter().
 * Modify query to sum ancestor's search scores for ead search, to improve search accuracy
 */

function ead_import_views_query_alter(&$view, &$query){
    if ($view->name === "collection_search"){

	// create inner join on the child-ancestor table
	// to get all ancestors for a node
	$join = new views_join();
	$join->table = 'ead_import_child_ancestor';
	$join->field = 'child_nid';
	$join->left_table = 'node';
	$join->left_field = 'nid';
	$join->type = 'INNER';

	// add the join to the query
	$alias = $query->ensure_table("ead_import_child_ancestor", NULL, $join);

    }
}

