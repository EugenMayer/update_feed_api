<?php
// $Id$

// Copyright (c) 2010 KontextWork
// Author: Eugen Mayer

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
   '#description' => 'Regenerate all projects and their release. <strong>Be aware, this deletes all old entries!</strong>',
   '#collapsible' => TRUE,
   '#collapsed' => FALSE,
  );
  $form['update_feed_container']['feeds_container'] = array(
   '#type' => 'fieldset',
   '#title' => 'Which feeds to update',
   '#collapsible' => TRUE,
   '#collapsed' => FALSE,
  );
  $options = array();
  $feeds = _update_feed_api_get_feeds();
  foreach($feeds as $feed) {
    $options[ $feed['server_key'] ] = $feed['host'];
  }
  
  $form['update_feed_container']['feeds_container']['feeds'] = array(
   '#type' => 'checkboxes',
   '#title' => 'Feeds',
   '#description' => 'Chose the feeds to update',
   '#options' =>  $options,
   '#default_value' => array(),
  );
  
  $form['update_feed_container']['fetch_releases'] = array(
   '#type' => 'checkbox',
   '#title' => 'Fetch all releases',
   '#description' => 'This means, for every project fetched, all releases are fetched. Takes <strong>long</strong>',
  );
  
  $form['update_feed_container']['start'] = array(
    '#type' => 'submit',
    '#value' => t('Refresh'),
  );
  return $form;
}

function update_feed_api_refresh_form_validate($form, &$form_state) {
  $feeds = $form_state['values']['feeds'];
  $feeds = array_filter($feeds,'_update_feed_api_get_activate_feeds');
  if(count($feeds) == 0) {
    form_set_error('feeds', 'You need to select at least one feed');
  }
}


function update_feed_api_refresh_form_submit($form, &$form_state) {
  require_once 'update_feed_api.batch.inc';
  $fetch_releases = $form_state['values']['fetch_release'];
  $feeds = $form_state['values']['feeds'];
  $feeds = array_filter($feeds,'_update_feed_api_get_activate_feeds');
  $feeds = array_keys($feeds);
  _update_feed_api_start_refresh_batch($feeds, $fetch_releases);
}

function _update_feed_api_get_activate_feeds($i) {
  if($i === 0) {
    return FALSE;
  }
  
  return TRUE;
}