<?php
// $Id$

// Copyright (c) 2010 KontextWork
// Author: Eugen Mayer

function ufa_test() {
  //$p = update_feed_api_get_project('drupal_org', 'ctools', '6.x', TRUE);
  $p = Project_factory::load_project('kontextwork','daisydiff', '6.x', TRUE, 'module');
  dsm($p->releases_to_array());
  return '';
}

function update_feed_api_admin_settings() {
  $form['feeds'] = array(
    '#type' => 'fieldset',
    '#title' => t('Feeds'),
    '#collapsed' => FALSE,
    '#collapsible' => FALSE,
    '#weight' => -2,
  );
  
  $form['feeds']['update_feed_api_feeds'] = array(
    '#type' => 'textarea',
    '#title' => t('Update-feeds'),
    '#description' => t('Define which updat-feeds to fetch from where. Format: host|machine_name|class. Example: <em>"http://updates.drupal.org/release-history|drupal_org|Update_feed_xmlreader"</em>. For each feed use on separate line.'),
    '#default_value' => variable_get('update_feed_api_feeds', 'http://updates.drupal.org/release-history|drupal_org|Update_feed_xmlreader')
  );
  
  $form['feeds']['update_feed_api_stepping'] = array(
    '#type' => 'textfield',
    '#title' => t('Stepping'),
    '#description' => t('How many projects should be processed at once. Keep this a small number on small servers'),
    '#default_value' => variable_get('update_feed_api_stepping', 10)
  );
  $apis = array( '8.x','7.x','6.x','5.x','4.7.x','4.6.x','4.5.x','4.4.x','4.3.x','4.2.x','4.1.x','4.0.x',);
  $form['feeds']['apis'] = array(
    '#type' => 'fieldset',
    '#title' => t('Apis'),
    '#description' => t('Select the allowed API version'),
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
    '#weight' => -2,
  );
  $form['feeds']['apis']['update_feed_api_allowed_apis'] = array(
    '#type' => 'checkboxes',
    '#title' => t('API Versions'),
    '#options' => drupal_map_assoc($apis),
    '#description' => t('Only the selected apis are fetched from the API servers'),
    '#default_value' => variable_get( 'update_feed_api_allowed_apis', array('7.x','6.x','5.x') ),
  );
   $form['feeds']['cron'] = array(
    '#type' => 'fieldset',
    '#title' => t('Cron runs'),
    '#description' => t('Select the allowed API version'),
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
    '#weight' => -2,
  );
  
  $form['feeds']['cron']['udpate_feed_api_cron'] = array(
    '#type' => 'checkbox',
    '#title' => t('Auto-update'),
    '#description' => t('Automaticly update feeds'),
    '#default_value' => variable_get( 'udpate_feed_api_cron', FALSE),
  );
  $form['feeds']['cron']['udpate_feed_api_cron_fetch_releases'] = array(
    '#type' => 'checkbox',
    '#title' => t('Fetch release on automatic cron runs'),
    '#description' => t('During automatic update runs runs, should the project releases be fetched?'),
    '#default_value' => variable_get( 'udpate_feed_api_cron_fetch_releases', FALSE),
  );
  $form['feeds']['cron']['udpate_feed_api_cron_schedule'] = array(
    '#type' => 'textfield',
    '#title' => t('Hours'),
    '#description' => t('Set on which interval the updates should be performed. (Every X hours)'),
    '#default_value' => variable_get( 'udpate_feed_api_cron_schedule', 24),
  ); 
  
  return system_settings_form($form);
}

function update_feed_api_refresh_form(&$form_state) {
  $form['update_feed_container'] = array(
   '#type' => 'fieldset',
   '#title' => 'Update update-feeds',
   '#description' => 'Select which update feeds to refresh. Beware, all selected feeds will be removed from the current database first.',
   '#collapsible' => TRUE,
   '#collapsed' => FALSE,
  );
  $form['update_feed_container']['feeds_container'] = array(
   '#type' => 'fieldset',
   '#title' => 'Configured feeds',
   '#collapsible' => TRUE,
   '#collapsed' => FALSE,
  );
  $options = array();
  $feeds = _update_feed_api_get_feeds();
  foreach($feeds as $feed) {
    $options[ $feed['server_key'] ] = $feed['host'].' ('.$feed['server_key'].')';
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
  $feeds = array_filter($feeds,'_update_feed_api_checkbox_filter');
  if(count($feeds) == 0) {
    form_set_error('feeds', 'You need to select at least one feed');
  }
}


function update_feed_api_refresh_form_submit($form, &$form_state) {
  require_once 'update_feed_api.batch.inc';
  $fetch_releases = $form_state['values']['fetch_releases'];
  $feeds = $form_state['values']['feeds'];
  $feeds = array_filter($feeds,'_update_feed_api_checkbox_filter');
  $feeds = array_keys($feeds);
  _update_feed_api_start_refresh_batch($feeds, $fetch_releases);
}

