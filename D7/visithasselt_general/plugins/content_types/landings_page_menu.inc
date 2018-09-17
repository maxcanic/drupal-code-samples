<?php
$plugin = array(
  'single' => TRUE,
  'title' => t('Landings page menu'),
  'description' => t('Renders the landings page specific menu'),
  'category' => t('Helics'),
  'render callback' => 'visithasselt_general_landings_page_menu_render',
  'defaults' => array(),
  'all contexts' => TRUE,
);

/**
 * Render
 *
 * @param $subtype
 * @param $conf
 * @param $args
 * @param $context
 */
function visithasselt_general_landings_page_menu_render($subtype, $conf, $args, $context) {
  $block = new stdClass();
  $block->module = 'visithasselt_general';

  $nid = $args[0];

  $node_context = isset($context['argument_entity_id:node_1']) ? $context['argument_entity_id:node_1'] : NULL;

  if (!empty($node_context) && isset($node_context->data->tnid) && !empty($node_context->data->tnid)) {
    $nid = $node_context->data->tnid;
  }

  

  $landings_page_menu_data = menu_tree_all_data('landings_page_' . $nid);
  $landings_page_menu_output = menu_tree_output($landings_page_menu_data);

  $block->content = drupal_render($landings_page_menu_output);
  return $block;
}