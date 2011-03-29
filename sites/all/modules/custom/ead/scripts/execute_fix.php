<?php

// temp patch


function _drupal_execute($form_id, &$form_state) {
  $args = func_get_args();
  $form = call_user_func_array('drupal_retrieve_form', $args);
  $form['#post'] = $form_state['values'];
 
  // additional patch
  // Reset form validation.
  // if form validation fails once it fails for all subsequent elements!
  $form_state['must_validate'] = TRUE;
  form_set_error(NULL, '', TRUE);  // reset all form errors

  drupal_prepare_form($form_id, $form, $form_state);
  _drupal_process_form($form_id, $form, $form_state);
}

  /**
   * Alternate version of drupal_execute.
   *
   * We need to call an alternative version of drupal_validate_form()
   * because the original uses static data that prevents the same form_id
   * to be validated more than once per execution unit.
   *
   * For additional information, please see:
   * http ://drupal.org/node/260934
   */
/*
function _drupal_execute($form_id, &$form_state) {
    $args = func_get_args();
    $form = call_user_func_array('drupal_retrieve_form', $args);
    $form['#post'] = $form_state['values'];
    drupal_prepare_form($form_id, $form, $form_state);
    _drupal_process_form($form_id, $form, $form_state);
}
*/
function _drupal_process_form($form_id, &$form, &$form_state) {
    $form_state['values'] = array();

    $form = form_builder($form_id, $form, $form_state);
    // Only process the form if it is programmed or the form_id coming
    // from the POST data is set and matches the current form_id.
    if ((!empty($form['#programmed'])) || (!empty($form['#post']) && (isset($form['#post']['form_id']) && ($form['#post']['form_id'] == $form_id)))) {
	_drupal_validate_form($form_id, $form, $form_state);

	// form_clean_id() maintains a cache of element IDs it has seen,
	// so it can prevent duplicates. We want to be sure we reset that
	// cache when a form is processed, so scenerios that result in
	// the form being built behind the scenes and again for the
	// browser don't increment all the element IDs needlessly.
	form_clean_id(NULL, TRUE);

	if ((!empty($form_state['submitted'])) && !form_get_errors() && empty($form_state['rebuild'])) {
	    $form_state['redirect'] = NULL;
	    form_execute_handlers('submit', $form, $form_state);

	    // We'll clear out the cached copies of the form and its stored data
	    // here, as we've finished with them. The in-memory copies are still
	    // here, though.
	    if (variable_get('cache', CACHE_DISABLED) == CACHE_DISABLED && !empty($form_state['values']['form_build_id'])) {
		cache_clear_all('form_'. $form_state['values']['form_build_id'], 'cache_form');
		cache_clear_all('storage_'. $form_state['values']['form_build_id'], 'cache_form');
	    }

	    // If batches were set in the submit handlers, we process them now,
	    // possibly ending execution. We make sure we do not react to the batch
	    // that is already being processed (if a batch operation performs a
	    // drupal_execute).
	    if ($batch =& batch_get() && !isset($batch['current_set'])) {
		// The batch uses its own copies of $form and $form_state for
		// late execution of submit handers and post-batch redirection.
		$batch['form'] = $form;
		$batch['form_state'] = $form_state;
		$batch['progressive'] = !$form['#programmed'];
		batch_process();
		// Execution continues only for programmatic forms.
		// For 'regular' forms, we get redirected to the batch processing
		// page. Form redirection will be handled in _batch_finished(),
		// after the batch is processed.
	    }

	    // If no submit handlers have populated the $form_state['storage']
	    // bundle, and the $form_state['rebuild'] flag has not been set,
	    // we're finished and should redirect to a new destination page
	    // if one has been set (and a fresh, unpopulated copy of the form
	    // if one hasn't). If the form was called by drupal_execute(),
	    // however, we'll skip this and let the calling function examine
	    // the resulting $form_state bundle itself.
	    if (!$form['#programmed'] && empty($form_state['rebuild']) && empty($form_state['storage'])) {
		drupal_redirect_form($form, $form_state['redirect']);
	    }
	}
    }
}
function _drupal_validate_form($form_id, $form, &$form_state) {
    // If the session token was set by drupal_prepare_form(), ensure that it
    // matches the current user's session.
    if (isset($form['#token'])) {
	if (!drupal_valid_token($form_state['values']['form_token'], $form['#token'])) {
	    // Setting this error will cause the form to fail validation.
	    form_set_error('form_token', t('Validation error, please try again. If this error persists, please contact the site administrator.'));
	}
    }

    _form_validate($form, $form_state, $form_id);
}
?>