<?php

/**
 * @file
 * Admin page settings form for user authorisation module.
 */

/**
 * Settings form.
 */
function pcm_user_authorisation_admin_settings() {
  $form = array();

  $form['pcm_user_authorisation'] = array(
    '#type' => 'textarea',
    '#rows' => 10,
    '#title' => t('Page urls which will shown also for anonymous users'),
    '#default_value' => variable_get('pcm_user_authorisation', ''),
    '#description' => t("A list of urls which are visible for anonymous users."),
    '#required' => TRUE,
  );

  return system_settings_form($form);
}