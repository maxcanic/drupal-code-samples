<?php

/**
 * Menu callback - Topic menu tab on a node page
 */
function visithasselt_general_topic_menu($node) {
  if ($node->type != 'landings_page') {
    return;
  }

  $landings_page_type = !empty($node->field_landings_page_type[LANGUAGE_NONE][0]['value']) ? $node->field_landings_page_type[LANGUAGE_NONE][0]['value'] : NULL;
  $landings_page_menu = !empty($node->field_landings_page_menu[LANGUAGE_NONE][0]['value']) ? $node->field_landings_page_menu[LANGUAGE_NONE][0]['value'] : FALSE;
  if ($landings_page_type != 'landings_page' || !$landings_page_menu) {
    return;
  }

  $nid = $node->nid;
  if (isset($node->tnid) && !empty($node->tnid)) {
    $nid = $node->tnid;
  }

  drupal_goto('admin/structure/menu/manage/landings_page_' . $nid);
}