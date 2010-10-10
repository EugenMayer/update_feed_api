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
  $defaults = $element['#value'];
  dsm($element);
  $element['server_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Server key'),
    '#default_value' => $defaults['server_key'],
    '#size' => '40',
  );
  $element['short_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Module'),
    '#default_value' => $defaults['short_name'],
    '#size' => '30',
  );
  $element['api_version'] = array(
    '#type' => 'textfield',
    '#title' => t('API'),
    '#default_value' => $defaults['api_version'],
    '#size' => 5,
    '#maxlength' => 5
  );  
  $element['release'] = array(
    '#type' => 'textfield',
    '#title' => t('Release'),
    '#default_value' => $defaults['release'],
    '#size' => '30',
  );
  return $element;
}

