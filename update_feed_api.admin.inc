<?php
// $Id$

// Copyright (c) 2010 KontextWork
// Author: Eugen Mayer

/**
 * Providing a administration interface for tagging.
 */
function update_feed_api_admin_settings() {
  
  $form['feeds'] = array(
    '#type' => 'fieldset',
    '#title' => t('Feeds'),
    '#description' => t('Set the available styles (css) here'),
    '#collapsed' => FALSE,
    '#collapsible' => FALSE,
    '#weight' => -2,
  );
  $form['feeds']['update_feed_api_feeds'] = array(
    '#type' => 'textarea',
    '#title' => t('Update-feeds'),
    '#description' => t('Define which updat-feeds to fetch from where. Format: host|machine_name|class. Example: <em>"http://updates.drupal.org|drupal_org|Update_feed"</em>. For each feed use on separate line.'),
    '#default_value' => variable_get('update_feed_api_feeds', '')
  );
  
  $form['feeds']['update_feed_api_stepping'] = array(
    '#type' => 'textfield',
    '#title' => t('Stepping'),
    '#description' => t('How many projects should be processed at once. Keep this a small number on small servers'),
    '#default_value' => variable_get('update_feed_api_stepping', 10)
  );
  return system_settings_form($form);
}

function update_feed_api_refresh_form(&$form_state) {
   $form['update_feed_container'] = array(
   '#type' => 'fieldset',
   '#title' => 'Update update-feeds',
   '#description' => 'Regenerate all projects and there release. <strong>Be aware, this deletes all old entries!</strong>',
   '#collapsible' => TRUE,
   '#collapsed' => FALSE,
  );
  
  $form['update_feed_container']['start'] = array(
    '#type' => 'submit',
    '#value' => t('Refresh'),
  );
  return $form;
}

function update_feed_api_refresh_form_submit($form, &$form_state) {
  require_once 'update_feed_api.batch.inc';
  _update_feed_api_start_refresh_batch();
}