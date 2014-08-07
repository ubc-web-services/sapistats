<?php

/**
 * @file
 * Administrative callbacks and functions for the sapistats module.
 */

/**
 * Build the settings form.
 */
function sapistats_admin($form, &$form_state) {
  $form['sapistats_enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable Seatch API query log'),
    '#default_value' => variable_get('sapistats_enabled', 0),
    '#description' => t('Enable information collection about all queries issued via the Search API.'),
  );

  $periods_flush = drupal_map_assoc(array(1209600, 2592000, 7776000, 15552000, 31104000, 63072000), 'format_interval');
  $form['sapistats_flush_log_timer'] = array(
    '#type' => 'select',
    '#title' => t('Store logs for'),
    '#default_value' => variable_get('sapistats_flush_log_timer', 15552000),
    '#options' => $periods_flush,
    '#description' => t('Older query log entries will be automatically discarded. (Requires a correctly configured <a href="@cron">cron maintenance task</a>.)', array('@cron' => url('admin/reports/status'))),
  );

  return system_settings_form($form);

}

/**
 * Creates form page for filter logging settings.
 */
function sapistats_choose_fields_form($form, &$form_state) {
  $indexes = array();
  $tmp = search_api_index_load_multiple(FALSE);
  $settings = variable_get('sapistats', array());
  foreach ($tmp as $index) {
    $indexes[$index->machine_name] = $index->machine_name;
  }
  $form['sapistats_indexes'] = array(
    '#type' => 'select',
    '#title' => t('Select search index'),
    '#options' => $indexes,
    '#default_value' => reset($indexes),
    '#description' => t('Select a search index to view statistics for'),

    '#ajax' => array(
      'callback' => 'sapistats_ajax_callback',
      'wrapper' => 'indexesfields',
      'method' => 'replace',
      'effect' => 'fade',
    ),
  );
  $id = !empty($form_state['values']['sapistats_indexes']) ? $form_state['values']['sapistats_indexes'] : reset($indexes);
  $index = search_api_index_load($id);
  $tmp = $index->getFields(TRUE);
  $logged = $settings['fields'][$id];
  $fields = array();
  foreach ($tmp as $name => $field) {
    $fields[$name] = $name;
  }

  $form['sapistats_index_fields_title'] = array(
    '#title' => t('Select fields to log statistics for:'),
    '#type' => 'item',
  );
  $form['sapistats_index_fields'] = array(
    '#weight' => '1',
    '#multiple' => '1',
    '#options' => $fields,
    '#default_value' => $logged,
    '#type' => 'checkboxes',
    '#return_value' => 1,
    '#prefix' => '<div id="indexesfields">',
    '#suffix' => '</div>',
  );
  $form['#submit'][] = 'sapistats_choose_fields_form_submit';
  return system_settings_form($form);
}


/**
 * Creates form page for blacklist settings.
 */
function sapistats_blacklist_searchids_form($form, &$form_state) {

  $form = array();
  $settings = variable_get('sapistats', array());
  $form['sapistats_blacklist_search_ids'] = array(
    '#title' => t('State search ids that should be ignored:'),
    '#default_value' => (implode("\n", $settings['blacklist']['searchids'])) . ', ',
    '#weight' => '0',
    '#type' => 'textarea',
    '#description' => t('Write each Search id in a new line'),
  );
  $form['#submit'][] = 'sapistats_blacklist_searchids_form_submit';
  return system_settings_form($form);
}

/**
 * Submits the blacklist form.
 */
function sapistats_blacklist_searchids_form_submit($form, &$form_state) {
  if (isset($form_state['values']['sapistats_blacklist_search_ids'])) {
    $settings = variable_get('sapistats', array());
    $settings['blacklist']['searchids'] = explode("\n", $form_state['values']['sapistats_blacklist_search_ids']);
    variable_set('sapistats', $settings);
  };
}

/**
 * Submits the fields whitelist form.
 */
function sapistats_choose_fields_form_submit($form, &$form_state) {
  $path = drupal_get_path('module', 'sapistats');
  require_once $path . '/sapistats.inc';
  $checked = $form_state['values']['sapistats_index_fields'];
  $curindex = $form_state['values']['sapistats_indexes'];
  $settings = variable_get('sapistats', array());
  $settings['fields'][$curindex] = sapistats_get_checked_checkboxes($checked);
  variable_set('sapistats', $settings);
}

/**
 * Ajax callback to reaload a part of a form.
 */
function sapistats_ajax_callback($form) {
  return $form['sapistats_index_fields'];
}
