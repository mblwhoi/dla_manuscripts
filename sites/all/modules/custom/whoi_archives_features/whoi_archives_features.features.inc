<?php

/**
 * Implementation of hook_content_default_fields().
 */
function whoi_archives_features_content_default_fields() {
  module_load_include('inc', 'whoi_archives_features', 'whoi_archives_features.defaults');
  $args = func_get_args();
  return call_user_func_array('_whoi_archives_features_content_default_fields', $args);
}

/**
 * Implementation of hook_ctools_plugin_api().
 */
function whoi_archives_features_ctools_plugin_api() {
  module_load_include('inc', 'whoi_archives_features', 'whoi_archives_features.defaults');
  $args = func_get_args();
  return call_user_func_array('_whoi_archives_features_ctools_plugin_api', $args);
}

/**
 * Implementation of hook_feeds_importer_default().
 */
function whoi_archives_features_feeds_importer_default() {
  module_load_include('inc', 'whoi_archives_features', 'whoi_archives_features.defaults');
  $args = func_get_args();
  return call_user_func_array('_whoi_archives_features_feeds_importer_default', $args);
}

/**
 * Implementation of hook_node_info().
 */
function whoi_archives_features_node_info() {
  module_load_include('inc', 'whoi_archives_features', 'whoi_archives_features.features.node');
  $args = func_get_args();
  return call_user_func_array('_whoi_archives_features_node_info', $args);
}

/**
 * Implementation of hook_default_page_manager_pages().
 */
function whoi_archives_features_default_page_manager_pages() {
  module_load_include('inc', 'whoi_archives_features', 'whoi_archives_features.defaults');
  $args = func_get_args();
  return call_user_func_array('_whoi_archives_features_default_page_manager_pages', $args);
}

/**
 * Implementation of hook_default_panels_mini().
 */
function whoi_archives_features_default_panels_mini() {
  module_load_include('inc', 'whoi_archives_features', 'whoi_archives_features.defaults');
  $args = func_get_args();
  return call_user_func_array('_whoi_archives_features_default_panels_mini', $args);
}

/**
 * Implementation of hook_strongarm().
 */
function whoi_archives_features_strongarm() {
  module_load_include('inc', 'whoi_archives_features', 'whoi_archives_features.defaults');
  $args = func_get_args();
  return call_user_func_array('_whoi_archives_features_strongarm', $args);
}

/**
 * Implementation of hook_views_default_views().
 */
function whoi_archives_features_views_default_views() {
  module_load_include('inc', 'whoi_archives_features', 'whoi_archives_features.features.views');
  $args = func_get_args();
  return call_user_func_array('_whoi_archives_features_views_default_views', $args);
}
