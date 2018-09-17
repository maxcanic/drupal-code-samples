<?php

/**
 * Settings form for Visit Hasselt.
 */
function visithasselt_settings_form($form, &$form_state) {
  form_load_include($form_state, 'inc', 'visithasselt_general', 'includes/admin');

  $langcode = !empty($_GET['variable_realm_key_language']) ? $_GET['variable_realm_key_language'] : $GLOBALS['language']->language;

  $form['newsletter_block_body'] = array(
    '#type'          => 'textarea',
    '#title'         => t('Newsletter block body'),
    '#default_value' => i18n_variable_get('newsletter_block_body', $langcode),
  );

  return system_settings_form($form);
}

function places_settings_form($form, &$form_state) {

  $form['fetch_places_time'] = array(
    '#type' => 'select',
    '#title' => t('Fetch time'),
    '#options' => drupal_map_assoc(
      range(1,23)
    ),
    '#default_value' => variable_get('fetch_places_time', 1),
  );
  $form['places_api_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Toerisme API url'),
    '#default_value' => variable_get('places_api_url', 'https://api.toerismewerkt.be'),
  );
  $form['places_token_path'] = array(
    '#type' => 'textfield',
    '#title' => t('path to request the JWT token'),
    '#default_value' => variable_get('places_api_token_path', '/authentication/token'),
  );
  $form['places_api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('API key to get access to the Toerismewerkt API'),
    '#default_value' => variable_get('places_api_key'),
  );
  $form['meetinhasselt_home'] = array(
    '#type' => 'textfield',
    '#title' => t('URL of meetinhasselt homepage'),
    '#default_value' => variable_get('meetinhasselt_home'),
  );
  $form['meetinhasselt_search'] = array(
    '#type' => 'textfield',
    '#title' => t('URL of meetinhasselt search results'),
    '#default_value' => variable_get('meetinhasselt_search'),
  );
  $form['meetinhasselt_mail_intro'] = array(
    '#type' => 'textarea',
    '#title' => t('Intro of the mail that will be sent'),
    '#default_value' => variable_get('meetinhasselt_mail_intro'),
  );
  $form['meetinhasselt_mail_outro'] = array(
    '#type' => 'textarea',
    '#title' => t('Outro of the mail that will be sent'),
    '#default_value' => variable_get('meetinhasselt_mail_outro'),
  );
  $form['meetinhasselt_mail_outro'] = array(
    '#type' => 'textfield',
    '#title' => t('E-mail address of the originator of the mails'),
    '#default_value' => variable_get('meetinhasselt_email'),
  );

  return system_settings_form($form);
}

/**
 * @return string
 * Listing sent emails for meeting place.
 */
function places_email_listing() {

  // include common.inc
  require_once DRUPAL_ROOT . '/includes/common.inc';

  // configure the table header columns
  $header = array(
    array('data' => 'Name', 'field' => 'name', 'sort' => 'ASC'),
    array('data' => 'Time', 'field' => 'date', 'sort' => 'ASC'),
  );

  $query = db_select('meeting_places_emails', 'm')
    ->extend('PagerDefault')
    ->extend('TableSort');

  $query->fields('m', array('name', 'date'))
    ->limit(10)
    ->orderByHeader($header)
    ->orderBy('date', 'DESC');

  $results = $query->execute();

  $rows = array();
  foreach ($results as $row) {
    $rows[] = array(
      $row->name,
      format_date($row->date, 'custom', 'Y-m-d'),
    );
  }

  $output = theme('table', array(
    'header' => $header,
    'rows' => $rows,
    )
  );

  // add the pager
  $output .= theme('pager');

  $output .= l('Download CSV', 'admin/config/visithasselt/list_mails_csv/downlaod');

  return $output;
}

/**
 * Export csv of sent mails for meeting place.
 */
function places_email_listing_csv() {

  $query = db_select('meeting_places_emails', 'm');
  $query->fields('m', array('name', 'date'));

  $results = $query->execute()->fetchAll();

  $rows = array();
  $rows[] = array(
    'name' => 'name',
    'date' => 'date'
  );

  foreach ($results as $row) {
    $rows[] = array(
      'name' => $row->name,
      'date' => format_date($row->date, 'custom', 'Y-m-d'),
    );
  }

  array_to_CSV($rows);

  header("Content-type: application/octet-stream");
  header("Content-Disposition: attachment; filename=meeting_place_email.xls");
  header("Pragma: no-cache");
  header("Expires: 0");
  return;
}

function array_to_CSV(&$data) {
  foreach ($data as $row) {
  $outstream = fopen("php://output", 'w');
  fputcsv($outstream, $row, ',', '"');
  rewind($outstream);
  $csv = fgets($outstream);
  fclose($outstream);
  }
}
