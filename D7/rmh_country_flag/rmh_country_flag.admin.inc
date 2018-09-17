<?php

/*
 * Build Country flag page.
 */
function rmh_country_flag_page() {

  $build = array();

  $build['table'] = _rmh_country_list_table();

  $build['add_new'] = array(
    '#prefix' => "<div>",
    '#suffix' => "</div>",
    '#markup' => l('Add new', 'admin/config/system/country_flag/add-new'),
  );

  return $build;

}

/**
 * rmh_country_flag add new form.
 */
function rmh_country_flag_add_form($form, &$form_state) {

  $form['country']['country_flag'] = array(
    '#title' => t('Country Flag'),
    '#type' => 'managed_file',
    '#description' => t('Upload country flag'),
    '#default_value' => '',
    '#upload_location' => 'public://',
    '#required' => TRUE,
  );

  $form['country']['link'] = array(
    '#type' => 'textfield',
    '#title' => t('Url'),
    '#description' => t('External link, has to start with for example https://www.example.com'),
    '#maxlength' => 255,
    '#required' => TRUE,
  );

  $form['country']['order'] = array(
    '#type' => 'select',
    '#title' => t('Order'),
    '#options' => drupal_map_assoc(range(0, 30)),
    '#default_value' => 0,
  );

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;

}

/*
 * Insert Country flags item in table.
 */
function rmh_country_flag_add_form_submit($form, &$form_state) {

  $fid = $form_state['values']['country_flag'];
  $url = $form_state['values']['link'];
  $order = (int)$form_state['values']['order'];


  $file = file_load($fid);

  // Change status to permanent.
  $file->status = FILE_STATUS_PERMANENT;

  file_save($file);

  file_usage_add($file, 'rmh_country_flag', 'country_flag', $file->fid);

  db_insert('rmh_country_flag')
    ->fields(array(
      'fid' => $fid,
      'country_url' => $url,
      'weight' => $order,
    ))->execute();

  $form_state['redirect'] = "admin/config/system/country_flag";

}

/**
 * rmh_country_flag add edit form.
 */
function rmh_country_flag_edit_form($form, &$form_state, $id = NULL) {

  $result = db_select('rmh_country_flag', 'cf')
    ->fields('cf')
    ->condition('cf.id', $id)
    ->execute()->fetchAll();

  $form['result'] = array(
    '#type' => 'value',
    '#value' => $result,
  );

  $form['country']['country_flag'] = array(
    '#title' => t('Country Flag'),
    '#type' => 'managed_file',
    '#description' => t('Upload country flag'),
    '#default_value' => $result[0]->fid,
    '#upload_location' => 'public://',
    '#required' => TRUE,
  );

  $form['country']['link'] = array(
    '#type' => 'textfield',
    '#title' => t('Url'),
    '#maxlength' => 255,
    '#default_value' => $result[0]->country_url,
    '#required' => TRUE,
  );

  $form['country']['order'] = array(
    '#type' => 'select',
    '#title' => t('Order'),
    '#options' => drupal_map_assoc(range(0, 30)),
    '#default_value' => $result[0]->weight,
  );

  $form['actions']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  $form['actions']['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
    '#submit' => array('_rmh_country_list_delete'),
  );

  return $form;
}

/*
 * Insert Country flags item in table.
 */
function rmh_country_flag_edit_form_submit($form, &$form_state) {

  $result = $form_state['values']['result'];

  $fid = $form_state['values']['country_flag'];
  $url = $form_state['values']['link'];
  $order = (int)$form_state['values']['order'];

  if ($result[0]->fid != $fid) {
    $file = file_load($fid);

    // Change status to permanent.
    $file->status = FILE_STATUS_PERMANENT;

    file_save($file);

    file_usage_add($file, 'rmh_country_flag', 'country_flag', $file->fid);

    $delete_file = file_load($result[0]->fid);
    file_delete($delete_file);
  }

  db_update('rmh_country_flag')
    ->fields(array(
      'fid' => $fid,
      'country_url' => $url,
      'weight' => $order,
    ))
    ->condition('id', $result[0]->id)
    ->execute();

  $form_state['redirect'] = "admin/config/system/country_flag";

}

/*
 * Listing of country flag items.
 */
function _rmh_country_list_table() {

  $headers = array(
    array('data' => 'Flag', 'style' => 'width:15%;'),
    array('data' => 'Url', 'style' => 'width:15%;'),
    array('data' => 'Weight', 'style' => 'width:15%;'),
    array('data' => 'Options', 'colspan' => 3, 'style' => 'width:10%;'),
  );

  $results = db_select('rmh_country_flag', 'cf')
    ->fields('cf')
    ->orderBy('weight', 'ASC')
    ->execute()->fetchAll();

  $rows = array();

  foreach ($results as $result) {
    $row = array();

    $image = file_load($result->fid);

    $row[] = theme('image_style', array('style_name' => 'thumbnail', 'path' => $image->uri, 'width' => $image->width, 'height' => $image->height));
    $row[] = $result->country_url;
    $row[] = $result->weight;
    $row[] = l('Edit', "admin/config/system/country_flag/{$result->id}/edit");

    $rows[] = $row;
  }

  return array(
    '#theme' => 'table',
    '#header' => $headers,
    '#rows' => $rows,
    '#empty' => t('No results.'),
  );

}

/*
 * Delete country flag from table.
 */
function _rmh_country_list_delete($form, &$form_state) {

  $result = $form_state['values']['result'];

  db_delete('rmh_country_flag')
    ->condition('id', $result[0]->id)
    ->execute();

  $file = file_load($result[0]->fid);
  file_delete($file);

  $form_state['redirect'] = "admin/config/system/country_flag";

}
