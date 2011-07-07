<?php

/**
 * hook_views_view_field() implementation
 * Customizes display of search result title links based on type of node.
 */
function whoi_manuscripts_views_view_field__manuscript_search_results__title($view, $field, $row){

  // Get the unaltered result.
  $result = $view->field[$field->options['id']]->advanced_render($row);

  // If node is of type ead collection, set the collection id to be the nid in the path.
  if ($row->node_type == 'ead_collection'){
    $result = preg_replace('/\/node\/#/', "/node/" . $row->nid . "#", $result);
  }

  return $result;
  
}

?>