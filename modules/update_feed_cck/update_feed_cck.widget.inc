<?php
// $Id$
function update_feed_cck_widget(&$form, &$form_state, $field, $items, $delta = 0) {
  $element= array(
    '#type' => 'update_feed_module',
    '#default_value' => $items[$delta],
  );
  return $element;
}

/**
 * An element #process callback for the update_feed_cck field type.
 *
 * Expands the filefield_widget type to include the upload field, upload and
 * remove buttons, and the description field.
 */
function update_feed_cck_widget_process($element, $edit, &$form_state, $form) {
  dsm($element);
  return $element;
}

/**
 * An element #process callback for the update_feed_cck field type.
 *
 * Expands the filefield_widget type to include the upload field, upload and
 * remove buttons, and the description field.
 */
function update_feed_cck_update_feed_module_process($element, $edit, &$form_state, $form) {
  dsm('here');
  $defaults = $element['#value'];  
  $options = array();
  $feeds = _update_feed_api_get_feeds();
  foreach($feeds as $feed) {
    $options[$feed['server_key']] = $feed['server_key'];
  }
  $element['server_key'] = array(
    '#type' => 'select',
    '#title' => t('Server key'),
    '#options' => $options,
    '#default_value' => $defaults['server_key'],
    '#size' => '1',
  );
  $element['short_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Module'),
    '#default_value' => $defaults['short_name'],
    '#size' => '30',
  );
  $apis = _update_feed_api_allowed_apis();  
  $element['api_version'] = array(
    '#type' => 'select',
    '#title' => t('API'),
    '#options' => drupal_map_assoc($apis),
    '#default_value' => $defaults['api_version'],
    '#size' => 1,
    '#autocomplete_path' => 'update_feed_cck/autocomplete/api_version',
  );  
  $element['release'] = array(
    '#type' => 'textfield',
    '#title' => t('Release'),
    '#default_value' => $defaults['release'],
    '#size' => '30',
  );
  return $element;
}

