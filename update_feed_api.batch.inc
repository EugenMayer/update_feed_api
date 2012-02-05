<?php

// Copyright (c) 2010 KontextWork
// Author: Eugen Mayer

function _update_feed_api_start_refresh_batch($feeds, $fetch_releases = TRUE) {
  // Update the user profiles to add values to newly added profile fields
  $batch = array(
    'title' => t('Updating update-feeds'), // Title to display while running.
    'operations' => array(), // Operations to complete, in order. Defined below.
    'finished' => '_update_feed_api_refresh_finished', // Last function to call.
    'init_message' => t('Initializing...'),
    //'progress_message' => t('Updated @current out of @total.'),
    'error_message' => t('Could not update update-feeds. Mabye connection errors occure or missconfigured feeds.'),
    'file' => drupal_get_path('module', 'update_feed_api') . '/update_feed_api.batch.inc',
  );
  $feeds = _update_feed_api_get_feeds($feeds);
  if(count($feeds) == 0) {
    drupal_set_message('No feeds configured yet? Go to the '.l('update Feed configuration','admin/settings/update_feed_api_settings'),'warning');
    return '';
  }
 
  $batch['operations'][] = array('_update_feed_api_process_feeds', array($feeds, $fetch_releases));
  // clear the tables before we start
  foreach($feeds as $feed) {
    _update_feed_api_clear_tables($feed['server_key']);
  }
  
  batch_set($batch);
}

function _update_feed_api_process_feeds($feeds, $fetch_releases, &$context) {
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['feeds'] = $feeds;
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['stepping'] = variable_get('update_feed_api_stepping', 10);
    $context['sandbox']['cur_feed_nr'] = -1;
    $context['sandbox']['allowed_api_versions'] = _update_feed_api_allowed_apis();
    $context['sandbox']['processing_feed'] = FALSE;
    $context['sandbox']['fetch_releases'] = $fetch_releases;
  }

  // if no feed is processed or the last is finished, start (initialize) the next one
  if ($context['sandbox']['processing_feed'] == FALSE) {
    // check if we are finished overall
    if($context['sandbox']['cur_feed_nr'] == count($context['sandbox']['feeds'])) {
      $context['finished'] = 1;
      return;
    }
    // we are not finished, so proceed to the next index
    $context['sandbox']['cur_feed_nr']++;
    // fetch the feed vars, helper only
    $class = $context['sandbox']['feeds'][ $context['sandbox']['cur_feed_nr'] ]['class'];
    $host = $context['sandbox']['feeds'][  $context['sandbox']['cur_feed_nr']] ['host'];
    $server_key = $context['sandbox']['feeds'][  $context['sandbox']['cur_feed_nr'] ]['server_key'];
    // create the feed reader object used by all the next batch runs until its finished
    
    $context['sandbox']['cur_feed'] = new $class($host, $server_key, $context['sandbox']['stepping'], NULL, $context['sandbox']['allowed_api_versions']);
    // fetch the complete project list and save it in a temporary file
    try {
      $context['sandbox']['cur_feed']->fetch_project_list();
      $context['sandbox']['processing_feed'] = TRUE;
    }
    catch( Exception $e) {
      drupal_set_message($e->getMessage(), 'error');
      // this feed failed, proceed to the next
      $context['sandbox']['processing_feed'] = FALSE;
    }
  }
  
  $status = $context['sandbox']['cur_feed']->fetch_next_projects($context['sandbox']['fetch_releases'], TRUE);
  if($status === TRUE) {
      $context['finished'] = 1;
  }
  // else
  $max_feeds = count($context['sandbox']['feeds']);
  // as we start from zero
  $cur_feed_nr = $context['sandbox']['cur_feed_nr'] +1;
  $cur_projects_nr = $context['sandbox']['cur_feed']->allready_fetched_count();
  $overall_projects_nr = $context['sandbox']['cur_feed']->overall_project_count();
  $server_key = $context['sandbox']['cur_feed']->server_key();
  
  $context['message'] = "Fetching feed($server_key) which {$cur_feed_nr} of {$max_feeds}. Fetched projects $cur_projects_nr of $overall_projects_nr";
  $context['finished'] = (1 / $status);
}

function _update_feed_api_refresh_finished() {
  drupal_set_message('All update-feeds have been updated');
  return '';
}
