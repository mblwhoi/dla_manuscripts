<?php
// $Id: term_fields.admin.inc,v 1.4.2.9 2010/10/28 11:06:22 bprod Exp $

/**
 * Administration page for Term Fields. Contains all the forms for editing fields.
 */
function term_fields_admin() {
  $form = array();
  $vocabs = taxonomy_get_vocabularies();
  $vocabularies = array();
  foreach ($vocabs as $vid => $vocab) {
    $vocabularies[$vid] = $vocab->name;
  }

  $result = pager_query("SELECT tf.fid AS fid, tf.title AS title, tf.description AS description, v.name AS name
    FROM {term_fields} tf
    LEFT JOIN {vocabulary} v ON tf.vid = v.vid
    ORDER BY v.name, tf.title",
    variable_get('term_fields_per_page', 25));

  if ($fields = term_fields_fetch_fields($result)) {
    $header = array(t('Field'), t('Vocabulary'), t('Description'), t('Operations'));
    $output .= theme('table', $header, $fields, array('id' => 'term-fields-admin'));
    $output .= theme('pager');
    return $output;
  }
  $output = t('There are no fields defined yet.');

  return $output;
}

/**
 * Administration page for Term Fields. Provides form for creating new fields.
 */
function term_fields_admin_new() {
  $form = array();
  $vocabs = taxonomy_get_vocabularies();

  if (!empty($vocabs)) {
    $vocabs_select = array();
    foreach ($vocabs as $key => $value) {
      $vocabs_select[$key] = $value->name;
    }
    $form['vid'] = array(
      '#type' => 'select',
      '#title' => t('Vocabulary'),
      '#options' => $vocabs_select,
      '#description' => t('Choose a vocabulary to assign this field to. Only terms in this vocabulary will be able to use this field. Cannot be changed later.'),
    );
    $form['fid'] = array(
      '#type' => 'textfield',
      '#title' => t('Field ID'),
      '#description' => t('The machine-readable name of the field. This name must contain only lowercase letters, numbers, and underscores. This name cannot be changed.'),
      '#required' => TRUE,
      '#maxlength' => 32,
    );
    $form['title'] = array(
      '#type' => 'textfield',
      '#title' => t('Title'),
      '#description' => t('A human-readable name to be used as the label for this field.'),
      '#required' => TRUE,
      '#element_validate' => array('title_validate'),
      '#maxlength' => 50,
    );
    $form['description'] = array(
      '#type' => 'textfield',
      '#title' => t('Description'),
      '#description' => t('Enter a short description of this field to explain what the field is used for.'),
      '#maxlength' => 100,
    );
    $form['type'] = array(
      '#type' => 'radios',
      '#title' => t('Field type'),
      '#options' => array(
        'checkbox' => t('Checkbox'),
        'date' => t('Date'),
        'file' => t('File'),
        'numeric' => t('Integer'),
        'select' => t('Select list'),
        'textarea' => t('Text area (multiple rows)'),
        'textfield' => t('Text field'),
      ),
      '#description' => t('Define what type of field to use. Single line textboxes should be used for fields such as names and dates, while multi line textareas should be used for longer fields, such as biographies. Cannot be changed later.'),
      '#required' => TRUE
    );
    $form['weight'] = array(
      '#type' => 'weight',
      '#title' => t('Weight'),
      '#default_value' => 0,
      '#description' => t('Heavier weighted items get put at the bottom of lists.')
    );
    $form['fields']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
    );
  }
  else {
    $form['empty'] = array(
      '#type' => 'item',
      '#value' => t('You have to <a href="@add-vocabulary">add a vocabulary</a> before you can add fields.', array('@add-vocabulary' => url('admin/content/taxonomy/add/vocabulary'))),
    );
  }

  return $form;
}

/**
 * Validate handler for term_fields_admin_new().
 */
function term_fields_admin_new_validate($form, &$form_state) {
  $values = $form_state['values'];
  if (!preg_match('~^[a-z]([_\w]*[a-z0-9]$|$)~', $values['fid'])) {
    form_set_error('fid', t('The field ID must contain only lowercase letters, numbers, and underscores. It should also begin by an alphabetic character and cannot finish by an underscore.'));
  }
  $bad_fids = array('name', 'description', 'numeric');
  $fids = term_fields_get_fids();
  if (!empty($fids)) {
    $bad_fids = $bad_fids + $fids;
  }
  if (in_array(strtolower($values['fid']), $bad_fids)) {
    form_set_error('fid', t('The field ID %fid cannot be used. It is either already being used by another field, or is a reserved internal name.', array('%fid' => $values['fid'])));
  }
}

/**
 * Submit handler for term_fields_admin_new().
 */
function term_fields_admin_new_submit($form, &$form_state) {
  $values = $form_state['values'];

  $values['fid'] = strtolower($values['fid']);

  $spec = term_fields_get_db_conf($values['type']);

  // Success
  drupal_set_message(t('The field %title (%fid) has been created.', array('%title' => $values['title'], '%fid' => $values['fid'])));

  // Insert a new record to the term_fields table.
  // Select type fields need a default value for the options column.
  if ($values['type'] != 'select') {
    $sql = "INSERT INTO {term_fields} (fid, vid, title, description, type, weight)
            VALUES ('%s', %d, '%s', '%s', '%s', %d)";
    db_query($sql, $values['fid'], $values['vid'], $values['title'], $values['description'], $values['type'], $values['weight']);
  }
  else {
    $sql = "INSERT INTO {term_fields} (fid, vid, title, description, type, weight, options)
            VALUES ('%s', %d, '%s', '%s', '%s', %d, '%s')";
    db_query($sql, $values['fid'], $values['vid'], $values['title'], $values['description'], $values['type'], $values['weight'], 'a:0:{}');

    // Redirect to the edit page and encourage the user to set the options for the select field.
    drupal_set_message(t('You should now specify some options for your select field.'));
    $form_state['redirect'] = 'admin/content/taxonomy/term_fields/edit/' . $values['fid'];
  }

  // Add the new column to the term_fields_term table.
  db_add_field($ret, 'term_fields_term', $values['fid'], $spec);
}

/**
 * Settings page.
 */
function term_fields_admin_settings() {
  $formats = array(
    'Y-m-d', 'm/d/Y', 'd/m/Y', 'Y/m/d',
    'd.m.Y', 'm/d/Y', 'd/m/Y', 'Y/m/d',
    'M j Y', 'j M Y', 'Y M j',
    'M j Y', 'j M Y', 'Y M j'
  );

  $options = array();
  foreach ($formats as $format) {
    $options[$format] = format_date(time(), 'custom', $format);
  }
  $form['date_format'] = array(
    '#type' => 'select',
    '#title' => t('Date field format'),
    '#description' => t('The format to be used when displaying date fields.'),
    '#default_value' => variable_get('term_fields_date_format', ''),
    '#options' => $options,
  );
  $form['custom'] = array(
    '#type' => 'textfield',
    '#title' => t('Custom date format'),
    '#default_value' => variable_get('term_fields_date_format_custom', ''),
    '#description' => t('A user-defined date format. If set, it will be used instead of what is set in the Date field format field. Leave blank to disable. See the <a href="@url">PHP manual</a> for available options.', array('@url' => 'http://php.net/manual/function.date.php')),
    '#size' => 5,
  );
  $form['per_page'] = array(
    '#type' => 'textfield',
    '#title' => t('Fields per page'),
    '#description' => t('Number of fields to show per page on the fields administration page.'),
    '#default_value' => variable_get('term_fields_per_page', 15),
    '#size' => 3,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Submit handler for settings form.
 */
function term_fields_admin_settings_submit($form, &$form_state) {
  $values = $form_state['values'];
  variable_set('term_fields_date_format', $values['date_format']);
  variable_set('term_fields_date_format_custom', $values['custom']);

  if (is_numeric($values['per_page'])) {
    variable_set('term_fields_per_page', $values['per_page']);
  }
  else {
    form_set_error('per_page', t('The Fields per page value must be numeric.'));
  }
}

/**
 * Administration page for term fields. Provides form for editing.
 */
function term_fields_admin_edit($form_state, $fid) {
  $field = db_fetch_object(db_query("SELECT * FROM {term_fields} WHERE fid = '%s'", $fid));

  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $field->title,
    '#description' => t('You can change the title of this field here.'),
    '#required' => TRUE,
    '#size' => 25,
    '#maxlength' => 50,
  );
  $form['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Description'),
    '#default_value' => $field->description,
    '#description' => t('You can change the description of this field here.'),
  );
  if ($field->type == 'select') {
    $default_value = '';
    if ($field->options && ($options = unserialize($field->options))) {
      $list = array();
      foreach ($options as $key => $value) {
        if ($key == $value) {
          $list[] = $key;
        }
        else {
          $list[] = "$key|$value";
        }
      }
      $default_value = implode("\n", $list);
    }

    $form['options'] = array(
      '#type' => 'textarea',
      '#title' => t('Allowed values list'),
      '#description' => t('The possible values this field can contain. Enter one value per line, in the format key|label. The key is the value that will be stored in the database, and it must match the field storage type (text). The label is optional, and the key will be used as the label if no label is specified.'),
      '#default_value' => $default_value,
    );
  }
  elseif ($field->type == 'file') {
    $options = unserialize($field->options);
    $form['file_allowed_exts'] = array(
      '#type' => 'textfield',
      '#title' => t('Permitted file extensions'),
      '#description' => t('Extensions that users in this role can upload. Separate extensions with a space and do not include the leading dot.'),
      '#default_value' => isset($options['file_allowed_exts']) ? implode(' ', $options['file_allowed_exts']) : 'txt',
      '#size' => 60,
      '#required' => TRUE,
    );
    $form['file_max_size'] = array(
      '#type' => 'textfield',
      '#title' => t('Maximum file size per upload'),
      '#description' => t('The maximum size of a file a user can upload.'),
      '#default_value' => isset($options['file_max_size']) ? $options['file_max_size'] : 1,
      '#size' => 1,
      '#required' => TRUE,
      '#field_suffix' => t('MB'),
    );
    $form['upload_max_size'] = array(
      '#value' => '<p>'. t('Your PHP settings limit the maximum file size per upload to %size.', array('%size' => format_size(file_upload_max_size()))) . '</p>',
    );
  }

  // Show the disabled data type for usability reasons.
  $form['type'] = array(
    '#type' => 'select',
    '#title' => t('Field type'),
    '#default_value' => $field->type,
    '#options' => array(
      'checkbox' => t('Checkbox'),
      'date' => t('Date'),
      'file' => t('File'),
      'numeric' => t('Integer'),
      'select' => t('Select list'),
      'textarea' => t('Text area (multiple rows)'),
      'textfield' => t('Text field'),
    ),
    '#disabled' => TRUE,
  );

  $form['weight'] = array(
    '#type' => 'weight',
    '#title' => t('Weight'),
    '#default_value' => $field->weight,
    '#description' => t('Fields weighted heavier appear at the bottom of lists.'),
  );
  $form['fid'] = array(
    '#type' => 'hidden',
    '#value' => $fid,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
  );

  drupal_set_title(t('Edit %fid field', array('%fid' => $fid)));

  return $form;
}

/**
 * Submit handler for term_fields_admin_edit().
 */
function term_fields_admin_edit_submit($form, &$form_state) {
  $values = $form_state['values'];

  if ($form_state['clicked_button']['#value'] == t('Delete')) {
    drupal_goto('admin/content/taxonomy/term_fields/delete/' . $values['fid']);
  }
  else {
    $type = db_result(db_query("SELECT type FROM {term_fields} WHERE fid = '%s'", $values['fid']));
    switch ($type) {
      case 'select':
        $options = array();
        // Code taken from content module
        $list = explode("\n", $values['options']);
        $list = array_map('trim', $list);
        $list = array_filter($list, 'strlen');
        foreach ($list as $opt) {
          // Sanitize the user input with a permissive filter.
          $opt = content_filter_xss($opt);
          if (strpos($opt, '|') !== FALSE) {
            list($key, $value) = explode('|', $opt);
            $options[$key] = (isset($value) && $value !=='') ? $value : $key;
          }
          else {
            $options[$opt] = $opt;
          }
        }

        // Check if there are duplicates
        if ($diff = array_diff_key($options, array_unique($options))) {
          $diff = array_unique($diff);
          $message = format_plural(count($diff), 'The value %values appears several times and may confuse the user.', 'The values %values appear several times and may confuse the user.', array('%values' => implode(', ', $diff)));
          drupal_set_message($message, 'warning');
        }

        $sql = "UPDATE {term_fields} SET title = '%s', description = '%s', weight = %d, options = '%s' WHERE fid = '%s'";
        db_query($sql, $values['title'], $values['description'], $values['weight'], serialize($options), $values['fid']);
        break;

      case 'file':
        $options = array();
        $options['file_max_size'] = $values['file_max_size'];
        $options['file_allowed_exts'] = explode(' ', $values['file_allowed_exts']);
        $options = serialize($options);
        $sql = "UPDATE {term_fields} SET title = '%s', description = '%s', weight = %d, options = '%s' WHERE fid = '%s'";
        db_query($sql, $values['title'], $values['description'], $values['weight'], $options, $values['fid']);
        break;

      default:
        $sql = "UPDATE {term_fields} SET title = '%s', description = '%s', weight = %d WHERE fid = '%s'";
        db_query($sql, $values['title'], $values['description'], $values['weight'], $values['fid']);
        break;
    }

    drupal_set_message(t('The field %fid has been updated.', array('%fid' => $values['fid'])));
    $form_state['redirect'] = 'admin/content/taxonomy/term_fields';
  }
}

/**
 * Provide a confirmation screen for deletions.
 */
function term_fields_admin_delete($form_state, $fid) {
  $form['fid'] = array(
    '#type' => 'hidden',
    '#value' => $fid,
  );

  $question = t('Are you sure you want to delete the term field %fid?', array('%fid' => $fid));
  $path = 'admin/content/taxonomy/term_fields';
  $description = t('If you have any content left in this field, it will be lost. This action cannot be undone.');

  return confirm_form($form, $question, $path, $description);
}

/**
 * Term field deletion confirmation screen.
 */
function term_fields_admin_delete_submit($form, &$form_state) {
  $fid = $form_state['values']['fid'];

  // Remove the field definition record from term_fields table.
  db_query("DELETE FROM {term_fields} WHERE fid = '%s'", $fid);

  // Remove the column from term_fields_term table.
  db_drop_field($ret, 'term_fields_term', $fid);

  drupal_set_message(t('The field %fid has been deleted.', array('%fid' => $fid)));
  $form_state['redirect'] = 'admin/content/taxonomy/term_fields';
}