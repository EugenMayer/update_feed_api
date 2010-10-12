<?php
// $Id$

// Copyright (c) 2010 KontextWork
// Author: Eugen Mayer


/**
 * Implementation of hook_field_info().
 */
function update_feed_cck_field_info() {
  return array(
    'update_feed_cck' => array(
      'label' => t('Update feed'), 
      'description' => t('A field for a module fetched by the update_feed_api'),
    ),
  );
}

/**
 * Implementation of CCK's hook_field().
 */
function update_feed_cck_field($op, $node, $field, &$items, $teaser, $page) {
  module_load_include('inc', 'update_feed_cck', 'update_feed_cck.field');
  $op = str_replace(' ', '_', $op);
  // add filefield specific handlers...
  $function = 'update_feed_cck_field_'. $op;
  if (function_exists($function)) {
    return $function($node, $field, $items, $teaser, $page);
  }
}

function update_feed_cck_field_settings($op, $field) {
  switch ($op) {
    case 'database columns':
      return array(
        'title' => array('type' => 'varchar', 'length' => '255',  'default' => '', 'not null' => TRUE,  'views' => TRUE),
        'server_key' => array('type' => 'varchar', 'length' => '40',  'default' => '', 'not null' => TRUE, 'views' => TRUE),
        'short_name' => array('type' => 'varchar', 'length' => '50',  'default' => '', 'not null' => TRUE, 'views' => TRUE),
        'api_version' => array('type' => 'varchar', 'length' => '10',  'default' => '', 'not null' => TRUE, 'views' => TRUE),
        'module_release' => array('type' => 'varchar', 'length' => '50', 'default' => '', 'not null' => TRUE, 'views' => TRUE),
    );
  }
}

/**
 * 
 */
function update_feed_cck_content_is_empty($item, $field) {
  if (empty($item['short_name']) ||  empty($item['module_release'])) {
    return TRUE;
  }
  return FALSE;
}


/**
 *  Thats our own FAPI element for our CCK widget
 */
function update_feed_cck_elements() {
  return array(
  'update_feed_module' => array(
    '#input' => TRUE,
    '#delta' => 0,
    '#process' => array('update_feed_cck_update_feed_module_process'),
    '#autocomplete_path' => FALSE,
    ),
  );
}

function theme_update_feed_module($element) {
  return theme('form_element', $element, $element['#children']);
}


/**
 * Implementation of CCK's hook_field_settings($op = 'views_data').
 */
function update_feed_cck_settings_views_data($field) {
}


