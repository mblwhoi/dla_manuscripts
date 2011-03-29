<?php
// $Id: term_fields_handler_field.inc,v 1.1.2.1 2009/12/01 09:33:35 bprod Exp $

/**
 * Handler that allows rendering of fields to be links to the term pages. Taken from views_handler_field_node.inc.
 */
class term_fields_handler_field extends views_handler_field {
  /**
   * Constructor to provide additional field to add.
   */
  function construct() {
    parent::construct();
    $this->additional_fields['tid'] = 'tid';
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['link_to_term'] = array('default' => FALSE);
    return $options;
  }

  /**
   * Provide link to term option.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['link_to_term'] = array(
      '#title' => t('Link this field to its term'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['link_to_term']),
    );
  }

  /**
   * Render whatever the data is as a link to the term.
   *
   * Data should be made XSS safe prior to calling this function.
   */
  function render_link($data, $values) {
    if (!empty($this->options['link_to_term']) && $data !== NULL && $data !== '') {
      return l($data, 'taxonomy/term/' . $values->{$this->aliases['tid']}, array('html' => TRUE));
    }
    else {
      return $data;
    }
  }

  function render($values) {
    return $this->render_link(check_plain($values->{$this->field_alias}), $values);
  }
}