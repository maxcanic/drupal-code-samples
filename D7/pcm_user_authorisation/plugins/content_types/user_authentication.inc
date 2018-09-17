<?php

/**
 * Ctools content type build.
 */
$plugin = array(
  'title' => t('User authentication'),
  'description' => t('Add accept and decline buttons for user authentication'),
  'single' => TRUE,
  'content_types' => array('user_authentication'),
  'render callback' => 'user_authentication_content_type_render',
  'edit form' => 'user_authentication_content_type_edit_form',
  'category' => t('Ctools content type'),
);

/**
 * Ctools edit form.
 */
function user_authentication_content_type_edit_form($form, &$form_state) {
  $conf = $form_state['conf'];

  $form['accept_destination'] = array(
    '#type' => 'textfield',
    '#title' => t('Accept button destination'),
    '#description' => t('Add url that will redirect users that accepted terms and conditions.'),
    '#default_value' => !empty($conf['accept_destination']) ? $conf['accept_destination'] : '',
    '#required' => TRUE,
  );

  $form['decline'] = array(
    '#title' => t('Decline button label'),
    '#type' => 'textfield',
    '#description' => t('Decline button label text field.'),
    '#default_value' => !empty($conf['decline']) ? $conf['decline'] : '',
    '#required' => TRUE,
  );

  $form['decline_destination'] = array(
    '#type' => 'textfield',
    '#title' => t('Decline button destination'),
    '#description' => t('Add url that will redirect users that did not accept terms and conditions.'),
    '#default_value' => !empty($conf['decline_destination']) ? $conf['decline_destination'] : '',
    '#required' => TRUE,
  );

  $form_state['plugin']['defaults'] = array(
    'accept_destination' => '',
    'decline' => '',
    'decline_destination' => '',
  );

  return $form;
}

/**
 * Ctools edit form submit handler.
 */
function user_authentication_content_type_edit_form_submit($form, &$form_state) {
  foreach ($form_state['plugin']['defaults'] as $key => $value) {
    $form_state['conf'][$key] = $form_state['values'][$key];
  }
}

/**
 * Render callback function.
 */
function user_authentication_content_type_render($subtype, $conf, $args, $context) {
  $block = new stdClass();

  $form = drupal_get_form('pcm_accept_form', $conf['accept_destination']);

  $block->content = theme('user_authentication', array(
    'accept' => drupal_render($form),
    'decline' => l($conf['decline'], $conf['decline_destination'], array('attributes' => array('class' => array('Button', 'Button--inactive')))),
  ));

  return $block;
}

/**
 * Accept and continue submit button build.
 */
function pcm_accept_form($form, &$form_state, $accept_destination) {
  $form['accept'] = array(
    '#type' => 'submit',
    '#value' => t('Accept and continue'),
    '#required' => TRUE,
    '#prefix' => "<div class='Button Button--active'>",
    '#suffix' => "</div>"
  );

  $form['accept_destination'] = array(
    '#type' => 'value',
    '#value' => $accept_destination,
  );

  return $form;
}

/**
 * Form submit.
 */
function pcm_accept_form_submit($form, &$form_state) {
  // Set cookie to user if he accepts terms and conditions.
  setrawcookie(PCM_USER_AUTHORISATION_COOKIE, rawurlencode(TRUE), REQUEST_TIME + 31536000, '/');

  $form_state['redirect'] = $form_state['values']['accept_destination'];
}
