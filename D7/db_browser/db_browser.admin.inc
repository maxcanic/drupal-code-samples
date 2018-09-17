<?php


/**
 * Menu callback, list of all tables.
 * @return array
 */
function db_browser_page() {
  $build = array();

  // Get all db tables.
  $sch = drupal_get_complete_schema();
  ksort($sch);

  $build['table_list'] = _db_browser_table_list($sch);

  return $build;
}


/**
 * Generate table with list of all db tables
 * @param $sch
 * @return array
 */
function _db_browser_table_list($sch) {
  $headers = array(
    array('data' => 'Table name'),
    array('data' => 'Module', 'style' => 'width:25%;'),
  );

  $types = array();

  $rows = array();
  foreach ($sch as $table_name => $table_data) {
    foreach($table_data['fields'] as $field => $spec) {
      $types[$spec['type']] = $spec['type'];
    }
    $row = array();

    $row[] = l($table_name, "admin/config/db-browser/{$table_name}");
    $row[] = $table_data['module'];

    $rows[] = $row;
  }

  return array(
    '#theme' => 'table',
    '#header' => $headers,
    '#rows' => $rows,
    '#empty' => t('No tables here.'),
  );
}


/**
 * Menu callback, DB Table results
 * @param $table_data
 * @return array
 */
function db_browser_table_page($table_data) {
  $build = array();

  // Include the CTools tools that we need.
  ctools_include('ajax');
  ctools_include('modal');

  // Add CTools' javascript to the page.
  ctools_modal_add_js();

  // Add filter form.
  $build['filter_form'] = drupal_get_form('db_browser_filter_form', $table_data);

  // Query teble.
  $build['table_results'] = _db_browser_table_results($table_data);

  $build['pager'] = array(
    '#theme' => 'pager',
  );

  return $build;
}



/**
 * Filter form
 * @param $form
 * @param $form_state
 * @param $table_fields
 * @return mixed
 */
function db_browser_filter_form($form, &$form_state, $table_fields) {

  $form['#method'] = 'get';

  $form['filters'] = array(
    '#type' => 'container',
    '#prefix' => '<div class="container-inline">',
    '#suffix' => '</div>',
  );

  // Generate filter fields.
  foreach ($table_fields['fields'] as $field => $spec) {
    if ($spec['type'] != 'blob') {
      $form['filters'][$field] = _db_browse_form_field($field, $spec);
    }
  }

  // Form buttons.
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Apply',
  );
  $form['actions']['reset'] = array(
    '#markup' => l('reset', "admin/config/db-browser/{$table_fields['name']}"),
  );


  return $form;
}


/**
 * DB table results
 * @param $table_data
 * @return array
 */
function _db_browser_table_results($table_data) {
  $table_name = $table_data['name'];

  $header = array();
  foreach ($table_data['fields'] as $field_name => $spec) {
    switch ($spec['type']) {
      case 'serial':
      case 'int':
      case 'float':
      case 'varchar':
      case 'char':
        $header[$field_name] = array('data' => $field_name, 'field' => 'tbl.' . $field_name);
        break;

      default:
        $header[$field_name] = array('data' => $field_name);
    }
  }


  $query = db_select($table_name, 'tbl');
  $query = $query->extend('PagerDefault')->extend('TableSort')->limit(50);
  $query->fields('tbl', array());

  foreach ($table_data['fields'] as $field_name => $spec) {
    if (isset($_GET[$field_name]) && $_GET[$field_name] != '') {
      if ($spec['type'] == 'int' || $spec['type'] == 'serial') {
        $query->condition($field_name, $_GET[$field_name]);
      } else {
        $query->condition($field_name, db_like($_GET[$field_name]) . '%', 'LIKE');
      }
    }
  }
  $query->orderByHeader($header);
  //$query_str = $query->__toString();
  //dpm($query_str);
  $results = $query->execute();



  $rows = array();
  foreach ($results as $res) {
    $row = array();

    $view_field_data = array();

    foreach ($table_data['fields'] as $field_name => $spec) {
      if ($spec['type'] == 'blob' || !empty($spec['serialize'])) {
        $view_field_conditions = base64_encode(json_encode($view_field_data));
        $row[] = l(t('view'), "admin/config/db-browser/{$table_name}/nojs/view/{$field_name}/{$view_field_conditions}", array('attributes' => array('class' => array('ctools-use-modal'))));
      } elseif ($spec['type'] == 'text' || $spec['type'] == 'varchar') {
        $row[] = check_plain($res->{$field_name});
        $view_field_data[$field_name] = $res->{$field_name};
      } else {
        $row[] = $res->{$field_name};
        $view_field_data[$field_name] = $res->{$field_name};
      }
    }
    $rows[] = $row;
  }

  return array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No rows.'),
  );

}

/**
 * View blob field page callback
 * @param $table_data
 * @param null $js
 * @param $field_name
 * @param $view_blob_spec
 * @return array|string|void
 *  ctools modal of basic page
 */
function db_browser_view_blob($table_data, $js = NULL, $field_name, $view_blob_spec) {

  $body = array();

  $table_name = $table_data['name'];


  if (!empty($field_name) && !empty($view_blob_spec) && !empty($table_data['fields'][$field_name])) {
    $cond = json_decode(base64_decode($view_blob_spec));

    $query = db_select($table_name, 'bc')->fields('bc', array($field_name));

    foreach ($cond as $field => $value) {
      $query->condition($field, $value);
    }
    $result = $query->execute()->fetchField();


    if (!empty($result)) {
      $out = @unserialize($result);
      if ($out !== false) {
        $body = '<pre>'.print_r($out, 1).'</pre>';
      } else {
        $body = '<pre>'.print_r($result, 1).'</pre>';
      }
    }
  }

  // Fallback if js is disabled.
  if (!$js) {
    return $body;
  }

  ctools_include('modal');
  ctools_include('ajax');
  return ctools_modal_render("Table: {$table_name}, Field: {$field_name} ({$table_data['fields'][$field_name]['type']})", $body);
}


/**
 * Filter form fields per type
 * @param $field
 * @param $spec
 * @return array
 */
function _db_browse_form_field($field, $spec) {
  $data = array();

  switch ($spec['type']) {
    case 'serial':
      $data = array(
        '#type' => 'textfield',
        '#title' => "$field ({$spec['type']})",
        '#title_display' => 'invisible',
        '#attributes' => array('placeholder' => "$field ({$spec['type']})"),
        '#default_value' => (isset($_GET[$field])) ? $_GET[$field] : '',
        '#size' => 12,
      );
      break;

    case 'int':
      $data = array(
        '#type' => 'textfield',
        '#title' => "$field ({$spec['type']})",
        '#title_display' => 'invisible',
        '#attributes' => array('placeholder' => "$field ({$spec['type']})"),
        '#default_value' => (isset($_GET[$field])) ? $_GET[$field] : '',
        '#size' => 12,
      );
      break;

    case 'varchar':
      $data = array(
        '#type' => 'textfield',
        '#title' => "$field ({$spec['type']})",
        '#title_display' => 'invisible',
        '#attributes' => array('placeholder' => "$field ({$spec['type']})"),
        '#maxlength' => (!empty($spec['length'])) ? $spec['length'] : 128,
        '#default_value' => (isset($_GET[$field])) ? $_GET[$field] : '',
        '#size' => 24,
      );
      break;

    case 'text':
      $data = array(
        '#type' => 'textfield',
        '#title' => "$field ({$spec['type']})",
        '#title_display' => 'invisible',
        '#attributes' => array('placeholder' => "$field ({$spec['type']})"),
        '#default_value' => (isset($_GET[$field])) ? $_GET[$field] : '',
        '#size' => 24,
      );
      break;

    case 'char':
      $data = array(
        '#type' => 'textfield',
        '#title' => "$field ({$spec['type']})",
        '#title_display' => 'invisible',
        '#attributes' => array('placeholder' => "$field ({$spec['type']})"),
        '#maxlength' => (!empty($spec['length'])) ? $spec['length'] : 128,
        '#default_value' => (isset($_GET[$field])) ? $_GET[$field] : '',
        '#size' => 24,
      );
      break;

    case 'float':
      $data = array(
        '#type' => 'textfield',
        '#title' => "$field ({$spec['type']})",
        '#title_display' => 'invisible',
        '#attributes' => array('placeholder' => "$field ({$spec['type']})"),
        '#default_value' => (isset($_GET[$field])) ? $_GET[$field] : '',
        '#size' => 12,
      );
      break;

  }

  return $data;
}