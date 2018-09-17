<?php

/**
 * get place id and modified date
 */
function _get_places() {

  $data = fetch_place();

  foreach($data['data'] as $place) {
    $id = $place['id'];
    $last_modified = strtotime($place['attributes']['lastModified']);

    // Load existing.
    $existing = _load_place_by_id($id);

    if (empty($existing)) {
      // set crete new drupal queue
      $queue = DrupalQueue::get('meeting_place_save');
      $queue->createItem($id);
    } elseif ($existing['updated'] < $last_modified) {
      $queue = DrupalQueue::get('meeting_place_update');
      $queue->createItem($id);
    }
  }
}


/**
 * @return string
 * Fetching token.
 */
function _get_token() {

  $token = '';

  // Get token from cache.
  $cache = cache_get('places_jwt_token');
  if (!empty($cache->data)) {
    // Check if token close to expire.
    if ($cache->expire < (REQUEST_TIME - 30)) {
      return $cache->data;
    }
  }

  $options = array(
    'method' => 'GET',
    'timeout' => 15,
  );

  //  $tok_path = 'https://api.toerismewerkt.be/authentication/token?API_KEY=62dade0c-f090-46b4-8f4b-6b5470686038';
  $api_url = variable_get('places_api_url', 'https://api.toerismewerkt.be');
  $tok_path = variable_get('places_api_token_path', '/authentication/token');
  $places_api_key = variable_get('places_api_key', '');

  $path = $api_url . $tok_path . '?API_KEY=' . $places_api_key;

  $tok = drupal_http_request($path, $options);

  $decoded =  drupal_json_decode($tok->data);
  $token  = $decoded['meta']['token'];

  $data = explode('.', $token);

  $unpacked_data = array();
  foreach ($data as $k => $item) {
    $item = drupal_json_decode(base64_decode($item));
    $unpacked_data[$k] = $item;
  }

  // Cache token.
  if (!empty($unpacked_data[1]['exp'])) {
    cache_set('places_jwt_token', $token, 'cache', $unpacked_data[1]['exp']);
  }

  return $token;
}

/**
 * @return array
 * Fetch places from API
 */
function fetch_place() {

  $token =  _get_token();

  $page = array(
    'first' => '&page[offset]=0&list=true',
    'next' => '&page[offset]=25&list=true',
    'last' => '&page[offset]=50&list=true',
  );

  //  $path = 'https://api.toerismewerkt.be/api/v2/touristicproducts/';

  $options = array(
    'method' => 'GET',
    'timeout' => 15,
    'headers' => array(
      'Content-Type' => 'application/json',
      'Authorization' => "Bearer {$token}"
    ),
  );

  $data = array();

  foreach ($page as $pages) {
    $path = variable_get('places_api_path', 'https://api.toerismewerkt.be/api/v2/touristicproducts/?type=mice' . $pages);

    $result = drupal_http_request($path, $options);

    if (!empty($result->code) && !empty($result->data) && $result->code == 200) {
      variable_set('meeting_places_last_fetch', date('Ymd', REQUEST_TIME));
      $_data = drupal_json_decode($result->data);

      // Merge results into one array
      $data = array_merge_recursive($_data ,$data);
    }
    else {
      watchdog('fetch_places_error_message', 'Fetch places error');
      watchdog('fetch_places_error', '<pre>'.print_r($result, 1).'</pre>');
      return array();
    }
  }
  return $data;
}

/**
 * Get single place
 */
function _get_single_place($id) {

  $token = _get_token();

  $options = array(
    'method' => 'GET',
    'timeout' => 15,
    'headers' => array(
      'Content-Type' => 'application/json',
      'Authorization' => "Bearer {$token}"
    ),
  );
//  $path = 'https://api.toerismewerkt.be/api/v2/touristicproducts/' . $id;
  $path = variable_get('places_api_path', 'https://api.toerismewerkt.be/api/v2/touristicproducts/') . $id;

  $results = drupal_http_request($path, $options);

  if (!empty($results->code) && !empty($results->data) && $results->code == 200) {

    return drupal_json_decode($results->data);
  } else {
    watchdog('places_token_error', 'Fetch token error');
    watchdog('token_error', '<pre>'.print_r($results, 1).'</pre>');
  }
}


/**
 * Create meeting places nodes.
 */
function _save_place($id = '') {

  $place = _get_single_place($id);

  if (!empty($place) && $place['data']['attributes']['address']['postalCode'] == 3500) {

    $last_update = strtotime($place['data']['attributes']['lastModified']);

    $node = new stdClass();
    $node->title = $place['data']['attributes']['name'];
    $node->type = 'meeting_place';
    node_object_prepare($node); // Sets some defaults. Invokes hook_prepare() and hook_node_prepare().
    $node->language = 'nl'; // Or e.g. 'en' if locale is enabled
    $node->uid = 1;
    $node->status = 1; //(1 or 0): published or not
    $node->promote = 0; //(1 or 0): promoted to front page
    $node->comment = 0; // 0 = comments disabled, 1 = read only, 2 = read/write
    $node->changed = strtotime($place['data']['attributes']['lastModified']);

    $node->field_meeting_description[LANGUAGE_NONE][0]['format'] = 'filtered_html';
    $node->field_meeting_description[LANGUAGE_NONE][0]['value'] = $place['data']['attributes']['description'];

    if (!empty($place['data']['attributes']['maximumCapacity'])) {
      $node->field_seats[LANGUAGE_NONE][0]['value'] = $place['data']['attributes']['maximumCapacity'];
    } else {
      $node->field_seats[LANGUAGE_NONE][0]['value'] = 0;
    }

    if (!empty($place['data']['attributes']['emailAddress'])) {
      $node->field_contact[LANGUAGE_NONE][0]['email'] = $place['data']['attributes']['emailAddress'];
    }

    $image_main  = _fetch_images($place['data']['attributes']['images'], TRUE);

    if (!empty($image_main)) {
      $node->field_main_image[LANGUAGE_NONE][0]['fid'] = $image_main[0]->fid;
    }


    $images  = _fetch_images($place['data']['attributes']['images']);
    foreach ($images as $image) {
      $node->field_images[LANGUAGE_NONE][]['fid'] = $image->fid;
    }


    $node->field_location[LANGUAGE_NONE][] = array(
      'target_id' => _find_locations($place['data']['attributes']['name'], $place['data']['attributes']['address'], $place['data']['attributes']['location']),
      'target_type' => 'node',
    );

    $terms = array();
    foreach($place['data']['attributes']['miceCategories'] as $cat_name) {
      $terms[] = _find_taxonomy($cat_name);
    }

    $terms = array_filter($terms);

    foreach($terms as $tid) {
      $node->field_activity_type['und'][]['tid'] = $tid;
    }


    $node = node_submit($node); // Prepare node for saving
    node_save($node);

    db_insert('meeting_places_node')
      ->fields(array(
        'nid' => $node->nid,
        'place_id' => $place['data']['id'],
        'updated' => $last_update,
      ))->execute();
  }
}

/**
 * @param string $id
 * Update meeting place node
 */
function _update_place($id = '') {
  $place = _get_single_place($id);

  if (!empty($place)) {

    // Load existing ...
    $existing = _load_place_by_id($id);

    $node = node_load($existing['nid']);


    $last_update = strtotime($place['data']['attributes']['lastModified']);

    $node->title = $place['data']['attributes']['name'];

    $node->changed = strtotime($place['data']['attributes']['lastModified']);

    $node->field_meeting_description[LANGUAGE_NONE][0]['format'] = 'filtered_html';
    $node->field_meeting_description[LANGUAGE_NONE][0]['value'] = $place['data']['attributes']['description'];

    if (!empty($place['data']['attributes']['maximumCapacity'])) {
      $node->field_seats[LANGUAGE_NONE][0]['value'] = $place['data']['attributes']['maximumCapacity'];
    } else {
      $node->field_seats[LANGUAGE_NONE][0]['value'] = 0;
    }

    if (!empty($place['data']['attributes']['emailAddress'])) {
      $node->field_contact[LANGUAGE_NONE][0]['email'] = $place['data']['attributes']['emailAddress'];
    }

    $image_main  = _fetch_images($place['data']['attributes']['images'], TRUE);

    if (!empty($image_main)) {
      $node->field_main_image[LANGUAGE_NONE][0]['fid'] = $image_main[0]->fid;
    }


    $images  = _fetch_images($place['data']['attributes']['images']);
    $node->field_images[LANGUAGE_NONE] = array();
    foreach ($images as $image) {
      $node->field_images[LANGUAGE_NONE][]['fid'] = $image->fid;
    }


    $node->field_location[LANGUAGE_NONE][0] = array(
      'target_id' => _find_locations($place['data']['attributes']['name'], $place['data']['attributes']['address'], $place['data']['attributes']['location']),
      'target_type' => 'node',
    );

    $terms = array();
    foreach($place['data']['attributes']['miceCategories'] as $cat_name) {
      $terms[] = _find_taxonomy($cat_name);
    }

    $terms = array_filter($terms);

    $node->field_activity_type = array();

    foreach($terms as $tid) {
      $node->field_activity_type['und'][]['tid'] = $tid;
    }

    node_save($node);

    db_update('meeting_places_node')
      ->fields(array(
        'updated' => $last_update,
      ))->condition('nid', $node->nid)
      ->execute();
  }

}

/**
 * @param string $title
 * @param array $address
 * @param array $loc
 * @return null
 * Save Locations.
 */
function _find_locations($title = '', $address = array(), $loc = array()) {

  $nid = NULL;

  $query = db_select('node', 'n');
  $query->fields('n', array('nid'))
    ->condition('n.title',  $title, '=')
    ->condition('n.type', 'locatie', '=');

  $nid = $query->execute()->fetchField();


  if (empty($nid) && !empty($title) && !empty($address)) {

    $node = new stdClass();
    $node->title = $title;
    $node->type = 'locatie';
    node_object_prepare($node); // Sets some defaults. Invokes hook_prepare() and hook_node_prepare().
    $node->language = 'nl'; // Or e.g. 'en' if locale is enabled
    $node->uid = 1;
    $node->status = 1; //(1 or 0): published or not
    $node->promote = 0; //(1 or 0): promoted to front page
    $node->comment = 0; // 0 = comments disabled, 1 = read only, 2 = read/write
    $node->field_address_location[LANGUAGE_NONE][0]['country'] = 'be';

    if (!empty($address['street'])) {
      $node->field_address_location[LANGUAGE_NONE][0]['street'] = $address['street'];
      if (!empty($address['houseNumber'])) {
        $node->field_address_location[LANGUAGE_NONE][0]['street'] .= ' ' . $address['houseNumber'];
      }
    }
    if (!empty($address['postalCode'])) {
      $node->field_address_location[LANGUAGE_NONE][0]['postal_code'] = $address['postalCode'];
    }

    if (!empty($address['municipality'])) {
      $node->field_address_location[LANGUAGE_NONE][0]['city'] = $address['municipality'];
    }
    if (!empty($address['region']['name'])) {
      $node->field_address_location[LANGUAGE_NONE][0]['province'] = $address['region']['name'];
    }
    if (!empty($loc['latitude']) && !empty($loc['longitude'])) {
      $node->field_address_location[LANGUAGE_NONE][0]['latitude'] = $loc['latitude'];
      $node->field_address_location[LANGUAGE_NONE][0]['longitude'] = $loc['longitude'];

      $node->field_address_location[LANGUAGE_NONE][0]['locpick']['user_latitude'] = $loc['latitude'];
      $node->field_address_location[LANGUAGE_NONE][0]['locpick']['user_longitude'] = $loc['longitude'];
    }


    $node = node_submit($node); // Prepare node for saving
    node_save($node);

    if (!empty($node->nid) && !empty($title) && !empty($address)) {
      $nid = $node->nid;
    }
  }

  return $nid;
}

/**
 * @param string $name
 * @param int $vid
 * @return mixed
 * Create new term
 */
function _find_taxonomy($name = '', $vid = 10) {
  $query = db_select('taxonomy_term_data', 't');
  $query->fields('t', array('tid'))
    ->condition('t.name',  $name)
    ->condition('t.vid', $vid);

  $tid = $query->execute()->fetchField();

  if (empty($tid)) {
    $term = new stdClass();
    $term->name = $name;
    $term->vid = $vid;
    taxonomy_term_save($term);

    if (!empty($term->tid)) {
      $tid = $term->tid;
    }
  }

  return $tid;
}

/**
 * @param array $images
 * @param bool $primary
 * @return array
 * Fetch images.
 */
function _fetch_images($images = array(), $primary = FALSE) {
  $out = array();

  foreach ($images as $image) {
    if ($image['isPrimary'] == $primary) {
      $image = @file_get_contents($image['large']);
      if (!empty($image)) {
        $file = file_save_data($image, NULL, FILE_EXISTS_RENAME);
        if (is_object($file)) {
          $file->status = 1;
          $out[] = file_save($file);
        }
      }
    }
  }
  return $out;
}


/**
 * @param string $place_id
 * @param string $changed
 * Load places needed to be updated.
 */
function _load_place_by_id($place_id = '') {

  $query = db_select('meeting_places_node', 'm');
  $query->fields('m')->condition('m.place_id', $place_id);

  return $query->execute()->fetchAssoc();

}
