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
  ctools_add_js('update_feed_cck','update_feed_cck');
  $defaults = $element['#value'];  
  $options = array();
  $feeds = _update_feed_api_get_feeds();
  foreach($feeds as $feed) {
    $options[$feed['server_key']] = $feed['server_key'];
  }
  dsm($element);
  $element['server_key'] = array(
    '#type' => 'select',
    '#title' => t('Server key'),
    '#options' => $options,
    '#default_value' => $defaults['server_key'],
    '#size' => '1',
  );
  $element['short_name'] = array(
    '#type' => 'autocomplete',
    '#title' => t('Module'),
    '#data' => 'update_feed_cck/autocomplete/modules',
    '#plugin_options' => array(
      'cacheLength' => 1,
      'mustMatch' => 1,
      'extraParams' => array(
        'server_key' => array('update_feed_cck_field' => array('id' => 'select#'.$element['#id'].'-server-key') ),
        'api' => array('update_feed_cck_field' => array('id' => 'select#'.$element['#id'].'-api-version') ),
      ),
    ),
    '#function_names' => array( 'update_feed_cck_server_key' ),
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
  );
  $element['release'] = array(
    '#type' => 'autocomplete',
    '#title' => t('Release'),
    '#data' => 'update_feed_cck/autocomplete/releases',
    '#plugin_options' => array(
      'cacheLength' => 1,
      'matchContains' => 0,
      'matchSubset' => FALSE,
      'max' => 0,
      'extraParams' => array(
        'server_key' => array('update_feed_cck_field' => array('id' => 'select#'.$element['#id'].'-server-key') ),
        'api' => array('update_feed_cck_field' => array('id' => 'select#'.$element['#id'].'-api-version') ),
        'module' => array('update_feed_cck_field' => array('id' => 'input#'.$element['#id'].'-short-name') ),
      ),
    ),
    '#default_value' => $defaults['release'],
    '#size' => '30',
  );
  return $element;
}