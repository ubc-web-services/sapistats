<?php

/**
 * @file
 * Defines main menu callback for the Search API Statistics module, and some utility
 * functions.
 */


function sapistats_js_callback() {
  if (isset($_POST['data']) && isset($_POST['token'])) {
    $records = $_POST['data'];
   
		//custom code by UBC WS to eliminate string null and NULL mismatch
    array_walk_recursive($records, function(&$item, $key) {
      if ($item == 'null') $item = NULL;
    });
		//end of custom code by UBC WS
    
    $gottoken = $_POST['token'];
    $jsoutput = json_encode($records);
    $token = sapistats_get_token($jsoutput);

    if ($token == $gottoken) {
      sapistats_log_entities($records);
    }
  }
  return '';
}


/**
 * Helper function to get all filter values from the GET-Request in a plain array.
 *
 * @param unknown_type $url
 */
function sapistats_get_filters_from_url($url) {
  $return = array();

  if (isset($url['f'])
  && (is_array($url['f']))) {
    foreach ($url['f'] as $ind => $value) {
      $scpos = strpos($value, ':');
      if ($scpos !== false) {
        // Substitute back escaped semicolon.
        $name = str_replace('%3A', ':', substr($value, 0, $scpos));
        $return[$name] = substr($value, $scpos + 1);
      }
    }
  }
  return $return;
}

/**
 * looks up if search id is blacklisted via settings.
 *
 * @param string $searchid
 */
function sapistats_check_if_blacklisted_searchid($searchid) {
  $settings = variable_get('sapistats', array());
  if (!isset($settings['blacklist']['searchids'])) {
    return FALSE;
  }
  foreach ($settings['blacklist']['searchids'] as $blacklisted) {
    if (strcmp($blacklisted, $searchid) == 0) {
      return TRUE;
    }
  }
  return FALSE;
}

function sapistats_check_if_logged_field($field, $index) {
  $settings = variable_get('sapistats', array());
  if (!isset($settings['fields'][$index])) {
    return FALSE;
  }

  foreach ($settings['fields'][$index] as $logged) {
    if (strcmp($logged, $field) == 0) {
      return TRUE;
    }
  }
  return FALSE;
}


/**
 * Creates a token over a given value, using salt and private key without session information.
 *
 * @param string $value
 */
function sapistats_get_token_orig($value = '') {
  return drupal_hmac_base64($value, drupal_get_private_key() . drupal_get_hash_salt());
}

/* 
 * Custom calls by UBC WS
 */
function sapistats_get_token($value = '') {
  $sanitized = str_replace('"', '', $value);
  return drupal_hmac_base64($sanitized, sapistats_getPrivateKey() . sapistats_getHashSalt());
}
function sapistats_getPrivateKey() {
  return 'lksjdfkjdlfjsdflkjdsf';
}
function sapistats_getHashSalt() {
  return 'ubccstudiesubccstudies';
}
/* 
 * End Of Custom calls by UBC WS
 */


function sapistats_log_entities($records) {
  $e = new stdClass();
  foreach ($records as $record) {
    $e = entity_create('sapistat_log', $record);
    $e->save();
  }
}


/**
 * Converts filter object into an array with contained filters.
 *
 * @param $filters
 *   The Filter object to expose.
 */
function sapistats_getfiltersarray($filters) {
  $array = array();
  foreach ($filters as $filter) {
    if (is_array($filter)) {
      $array[][$filter[0]] = $filter[1];
    }
    else {
      $array = array_merge($array, sapistats_getfiltersarray($filter->getFilters()));
    }
  }
  return $array;
}

/**
 * Helper function.
 * Returns only checked values of a checkbox.
 * @param type $input
 * @return type
 */
function sapistats_get_checked_checkboxes($input) {
  $output = array();
  foreach ($input as $fieldkey => $fieldname) {
    if ($fieldname) {
      $output[] = $fieldname;
    }
  }
  return $output;
}
