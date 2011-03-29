<?php
// $Id: fsrange_basic_handler.inc,v 1.1.2.1 2009/09/09 13:21:28 zyxware Exp $

/**
 * @file
 * Defines the fsrange_basic_handler class.
 *
 * Please read API.txt to understand the concepts in this source code.
 */

class fsrange_basic_handler extends fsrange_handler {
  
  /////////////////////////////////////////////////////////////////////////
  //
  // Functions that deal with this handler's options, and with the options form.
  //
  
  /**
   * Overrides base. Provides default options for the handler.
   */
  function get_default_options() {
    $options = parent::get_default_options();
    $options += array(
      'ranges_text' => '',
      'use_php' => 0,
      'ranges_php' => '',
    );
    return $options;
  }
  
  /**
   * Overrides base. Provides a form for setting options.
   */
  function options_form(&$form) {
    $form['ranges_text'] = array(
      '#type' => 'textarea',
      '#title' => t('Ranges'),
      '#default_value' => $this->options['ranges_text'],
      '#description' => t('Ranges of values to present to the user as categories. Enter one range per line, using the syntax "from .. to | label". The syntax to use is demonstrated and explained in the <a href="@readme-url">README.txt</a> file.', array('@readme-url' => _fsrange_get_readme_url())),
    );
    $form['show_leaves'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display the values themselves, as categories, when a deepest-level range is browsed.'),
      '#default_value' => $this->options['show_leaves'],
    );
    $form['advanced'] = array(
      '#type' => 'fieldset',
      '#title' => t('Advanced options'),
      '#collapsible' => TRUE,
      '#collapsed' => !$this->options['use_php'] && !$this->options['accumulate_count'],
    );
    $form['advanced']['accumulate_count'] = array(
      '#type' => 'checkbox',
      '#title' => t('When counting items in a range, add the sum of previous ranges.'),
      '#description' => t('This is sometimes, but not always. desired for ranges that overlap. For example, if you have two date ranges, "Published during last week" and "Published during last month", you will want the number displayed in brackets along the second range to include that of the first range. However, this behavior is not always desired; for example, when your last range is the catchall "-inf..inf | All others", you won\'t want its number to include any of the previous ranges\'. See the most fascinating discussion, "Overlapping Ranges", in the <a href="@readme-url">README.txt</a> file.', array('@readme-url' => _fsrange_get_readme_url())),
      '#default_value' => $this->options['accumulate_count'],
    );
    $form['advanced']['use_php'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use PHP code to build the ranges'),
      '#description' => t('If, and only if, this checkbox is checked, the "Ranges" box above will be ignored and the ranges will be computed by the PHP code you enter in the box below.'),
      '#default_value' => $this->options['use_php'],
    );
    $form['advanced']['ranges_php'] = array(
      '#type' => 'textarea',
      '#title' => t('PHP ranges'),
      '#description' => t('PHP code that returns an array of ranges. Should not include &lt;?php ?&gt; delimiters. The expected format is described in the <a href="@readme-url">README.txt</a> file.', array('@readme-url' => _fsrange_get_readme_url())),
      '#default_value' => $this->options['ranges_php'],
    );
  }
  
  /**
   * Overrides base. Validate the options form.
   */
  function options_validate($form_values) {
    if ($form_values['use_php']) {
      // Validate the PHP ranges.
      if (!$form_values['ranges_php']) {
        form_set_error('ranges_php', t("You haven't specified any PHP code."));
      }
      else {
        // Verify, as much as we can --which isn't very much, the PHP snippet.
        $levels = array(); // the snippet may use this variable.
        ob_start();
        $result = eval($form_values['ranges_php']);
        ob_end_clean();
        if (!is_array($result)) {
          form_set_error('ranges_php', t("Your PHP code doesn't return an array, as it should; or maybe there's a syntax error in it."));
        }
      }
    }
    else {
      // Validate the plain-text ranges.
      $errors = array();
      $ranges = $this->_parse_ranges_text($form_values['ranges_text'], $errors);
      while ($errors) {
        form_set_error('ranges_text', array_shift($errors));
      }
      if (!$ranges) {
        form_set_error('ranges_text', t('No ranges were specified.'));
      }
    }
  }
  
  /**
   * @private
   *
   * Parses a ranges text.
   *
   * @param $text
   *   the text containing the ranges definitions. E.g., as entered on the
   *   'options' form.
   * @param &$errors
   *   Log validation error messages onto this array.
   * @return
   *   A nested array of internal range structures.
   */ 
  function _parse_ranges_text($text, &$errors) {
    $ranges = array();
    foreach (explode("\n", $text) as $line_number => $line) {
      $depth = strlen($line) - strlen(ltrim($line, ' '));
      list($range, $label) = array_map('trim', explode('|', $line, 2));
      if (!$range) {
        // Skip empty lines.
        continue;
      }
      if (!$label) {
        // If no label was given, use the range as the label.
        $label = $range;
      }
      $boundaries = $this->_strip_range_boundaries($range);
      list($from, $to) = $this->_parse_range_string($range);
      if ($to == '') {
        $errors[] = t('Endpoint of range is missing at line @line', array('@line' => $line_number + 1));
      }
      if ($from == 'inf') {
        $errors[] = t('The left endpoint, at line @line, may only be "-inf", not "inf".', array('@line' => $line_number + 1));
      }
      if ($to == '-inf') {
        $errors[] = t('The right endpoint, at line @line, may only be "inf", not "-inf".', array('@line' => $line_number + 1));
      }
      if ($from != '' && $from == $to && !($boundaries['from'] == 'closed' && $boundaries['to'] == 'closed')) {
        $errors[] = t('Endpoints of range at line @line contain the same value. You must use a square bracket to include the right endpoint, e.g. "@range]", or else the range will be empty.', array('@line' => $line_number + 1, '@range' => $range));
      }
      $ranges[] = array(
        'from' => $from,
        'to' => $to,
        'boundaries' => $boundaries,
        'label' => $label,
        'depth' => $depth,
        'line_number' => $line_number + 1,
      );
    }
    $ranges = _fsrange_nest($ranges);
    $this->validate_ranges($ranges, NULL, $errors);
    return $ranges;
  }

  function _strip_range_boundaries(&$s) {
    $boundaries = array('from' => 'closed', 'to' => 'open');
    switch ($s{0}) {
      case '(': $boundaries['from'] = 'open';   break;
      case '[': $boundaries['from'] = 'closed'; break;
    }
    switch ($s{strlen($s)-1}) {
      case ')': $boundaries['to'] = 'open';   break;
      case ']': $boundaries['to'] = 'closed'; break;
    }
    $s = trim($s, '([])');
    return $boundaries;
  }

  function _parse_range_string($s) {
    return array_map('trim', explode('..', $s));
  }
  
  /**
   * Validates the parsed ranges tree. Errors are to be logged in $errors.
   */
  function validate_ranges($ranges, $parent_range, &$errors) {
    foreach ($ranges as $range) {
      $this->validate_range($range, $parent_range, $errors);
      if (isset($range['children'])) {
        $this->validate_ranges($range['children'], $range, $errors);
      }
    }
  }

  /**
   * Validates a single range structure. Errors are to be logged in $errors.
   *
   * This is for derived classes to implements, because we ourselves don't
   * know about the nature of the values.
   */
  function validate_range($range, $parent_range, &$errors) {
  }

  /**
   * @private
   *
   * Returns the internal structure describing a certain range.
   */ 
  function _get_range($range_id) {
    $ranges = $this->_get_ranges($this->range_id__get_parent($range_id));
    $levels = $this->range_id__get_levels($range_id);
    $last = array_pop($levels);
    return $ranges[$last];
  }

  /**
   * @private
   *
   * Returns an array of internal structures describing ranges.
   *
   * @param $parent_range_id
   *   The parent range ID whose children ranges to return. (Descendant
   *   ranges aren't returned; that is, a flat level is returned.)
   */ 
  function _get_ranges($parent_range_id = NULL) {
    $levels = $this->range_id__get_levels($parent_range_id);

    if (!$this->options['use_php']) {
      // Using plain text
      if (!isset($this->cached_ranges)) {
        $errors = array();
        $this->cached_ranges = $this->_parse_ranges_text($this->options['ranges_text'], $errors);
        // We discard $errors: the input was validated already when the 'options' form was submitted.
      }
      $ranges = $this->cached_ranges;
      while (count($levels) > 0) {
        // Go down the hierarchy...
        $ranges = $ranges[array_shift($levels)]['children'];
      }
      return $ranges;
    }
    else {
      // Using PHP
      if (!isset($this->cached_ranges)) {
        $this->cached_ranges = array();
      }
      $slot = isset($parent_range_id) ? $parent_range_id : 'top level ranges';

      if (!isset($this->cached_ranges[$slot])) {
        ob_start();
        $result = eval($this->options['ranges_php']);
        ob_end_clean();
        if (is_array($result)) {
          $this->cached_ranges[$slot] = $result;
        }
        else {
          drupal_set_message(t("Error: your PHP code for generating the ranges doesn't return an array"), 'error');
          $this->cached_ranges[$slot] = array();
        }
      }
      return $this->cached_ranges[$slot];
    }
  }

  /////////////////////////////////////////////////////////////////////////
  //
  // The Juicy action!
  //

  /**
   * Overrides base. Adds the field's table to the $query.
   *
   * This is a very basic join routine, that optionally consults a 'join_info'
   * structure, and returns a single column. If your fields require a more
   * sophisticated join, or if you need to return several columns, subclass and
   * override this method.
   */
  function join_table(&$query, $unique = FALSE) {
    $table = $this->definition['table'];
    $field = $this->definition['field'];
    if ($table == 'node') {
      // Table already linked.
      return array( "n.$field" );
    }
    else {
      $alias = $table;
      if ($unique) {
        static $unique_key = 1;
        $alias .= '_'. $field .'_'. $unique_key++;
      }
      // If no 'join_info' exists, the default is to join on the 'vid' column.
      // This happens to work for CCK fields as well.
      $this->definition += array('join_info' => array(
        'left'  => array('field' => 'vid'),
        'right' => array('field' => 'vid'),
      ));
      $join_info = $this->definition['join_info'];
      $query->add_table($table, $join_info['right']['field'], 'n', $join_info['left']['field'], $alias);
      return array( $alias .'.'. $field );
    }
  }
  
  /**
   * Overrides base. Constructs the SQL formula that returns a field's range ID.
   */
  function get_range_id_formula($qualified_columns, $parent_range_id = NULL, $desired_range_id = NULL) {
    $field = array_shift($qualified_columns); // operate on the 'main' column.
    $ranges = $this->_get_ranges($parent_range_id);
    if (!$ranges) {
      // If no ranges available, signal that we're done.
      return FALSE;
    }
    $parent_levels = $this->range_id__get_levels($parent_range_id);

    $sql = array();
    $args = array();

    $sql[] = 'CASE';

    foreach ($ranges as $key => $range) {
      $levels = $parent_levels;
      $levels[] = $key;
      $range_id = $this->range_id__from_levels($levels);

      if (!empty($desired_range_id) && $range_id != $desired_range_id) continue;

      $from = $this->prepare_sql_value($range['from']);
      $to   = $this->prepare_sql_value($range['to']);

      // default boundaries:
      $range += array('boundaries' => array('from' => 'closed', 'to' => 'open'));
      $from_op = $range['boundaries']['from'] == 'open' ? '>' : '>=';
      $to_op   = $range['boundaries']['to'  ] == 'open' ? '<' : '<=';

      if ($from == '-inf' && $to == 'inf') {
        $sql[] = "WHEN $field IS NOT NULL THEN '%s'";
        $args[] = $range_id; 
      }
      else if ($from == '-inf') {
        $sql[] = "WHEN $field $to_op '%s' THEN '%s'";
        $args[] = $to;
        $args[] = $range_id; 
      }
      else if ($to == 'inf') {
        $sql[] = "WHEN $field $from_op '%s' THEN '%s'";
        $args[] = $from;
        $args[] = $range_id; 
      }
      else {
        $sql[] = "WHEN $field $from_op '%s' AND $field $to_op '%s' THEN '%s'";
        $args[] = $from;
        $args[] = $to;
        $args[] = $range_id; 
      }
      
      if (!empty($desired_range_id)) break;
    }
    $sql[] = 'ELSE NULL';
    $sql[] = 'END';
    // Unfortunately, $query's add_field() doens't accept placeholder arguments,
    // so we have to embed the arguments ourselves and return a plain string.
    return _fsrange_embed_placeholders(join(' ', $sql), $args);
  }
  
  /**
   * Converts a $from or $to value, as typed in by the user, into a format
   * legible to the SQL server. It's for derived classes to override.
   */
  function prepare_sql_value($value) {
    return $value;
  }

  /**
   * Overrides base. Constructs the SQL formula for leaves' range IDs.
   */
  function get_leaves_formula($qualified_columns, $parent_range_id = NULL) {
    $field = array_shift($qualified_columns);
    $sql = "CONCAT('%s#', $field)";
    $args = array($parent_range_id);
    return _fsrange_embed_placeholders($sql, $args);
  }
  
  /////////////////////////////////////////////////////////////////////////
  //
  // Functions that handle rendering.
  //

  /**
   * Overrides base. Renders a non-leaf range.
   */
  function render_range($range_id, $html) {
    $range = $this->_get_range($range_id);
    return $html ? filter_xss_admin($range['label']) : strip_tags($range['label']);
  }

  /**
   * Overrides base. Renders a leaf range.
   */
  function render_leaf($leaf, $html) {
    // This isn't the nicest place to do CCK stuff. It should be factored out.
    // This 'leaf' thingy is a very late addition to the module and I haven't
    // yet refined it.
    if (isset($this->definition['cck_fieldname'])) {
      $field = content_fields($this->definition['cck_fieldname']);
      $db_info = content_database_info($field);
      // of course, this isn't alright; we should load all columns, not just the 'main' one.
      $output = content_format($field, array(key($db_info['columns']) => $leaf));
      return $html ? $output : strip_tags($output);
    }
    else {
      // We don't know about the value, so we must strip html. Derived
      // classes may know more about the value and choose otherwise.
      return strip_tags($leaf);
    }
  }

  /////////////////////////////////////////////////////////////////////////
  //
  // Functions for parsing range IDs.
  //
  // All override base. See documentation there.
  //

  function range_id__get_levels($range_id) {
    if (!isset($range_id)) {
      return array();
    }
    else {
      list($range_id, ) = explode('#', $range_id); // get rid of optional leaf.
      $levels = explode('-', $range_id);
      array_shift($levels); // get rid of the 'R'.
      return array_map('intval', $levels); // get rid of possible zero left padding.
    }
  }

  function range_id__from_levels($levels, $sortable = TRUE) {
    $range_id = 'R';
    foreach ($levels as $idx) {
      $range_id .= '-'. ($sortable ? sprintf('%05d', $idx) : $idx);
    }
    return $range_id;
  }

  function range_id__get_parent($range_id) {
    $levels = $this->range_id__get_levels($range_id);
    array_pop($levels);
    return $levels ? $this->range_id__from_levels($levels) : NULL;
  }

  function range_id__contains_leaf($range_id) {
    return strpos($range_id, '#') !== FALSE;
  }

  function range_id__get_leaf($range_id) {
    list(, $leaf) = explode('#', $range_id, 2);
    return $leaf;
  }

  function range_id__remove_leaf($range_id) {
    list($nonleaf, ) = explode('#', $range_id, 2);
    return $nonleaf;
  }
}

/**
 * A utility function that returns README.txt's URL.
 */
function _fsrange_get_readme_url($file = 'README.txt') {
  return base_path() . drupal_get_path('module', 'fsrange') .'/'. $file;
}

/**
 * Utility function that nests elements.
 *
 * The function gets a flat list of elements and nests them, based on
 * the 'depth' property each element has.
 *
 * @param $elements
 *  flat array of elements
 * @param $children_slot
 *  the name of the property in which to store the children. Defaults
 *  to 'children'.
 * @returns
 *  A nested array.
 */
function _fsrange_nest($elements, $children_slot = 'children') {
  $nested = array();
  while ($elements) {
    $first = array_shift($elements);
    $sub_elements = array();
    // Gather all sub elements
    while (($next = reset($elements)) && $next['depth'] > $first['depth']) {
      $sub_elements[] = $next;
      array_shift($elements);
    }
    if ($sub_elements) {
      $first[$children_slot] = _fsrange_nest($sub_elements);
    }
    $nested[] = $first;
  }
  return $nested;
}

