<?php

function _update_feed_api_process_feeds($feeds,&$context) { 
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['feeds'] = $feeds;
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['stepping'] = variable_get('update_feed_api_stepping', 10);
    $context['sandbox']['cur_feed_nr'] = -1;
    $context['sandbox']['processing_feed'] = FALSE;
  }
  // if no feed is procgressedd or the last is finished, start the next one
  if($context['sandbox']['processing_feed'] == FALSE) {
    // check if we are finished
    if($context['sandbox']['cur_feed_nr'] == count($context['sandbox']['feeds'])) {
      $context['finished'] = 1;
      return;
    }
    $context['sandbox']['cur_feed_nr']++;
    // fetch the feed vars
    $class = $context['sandbox']['feeds'][ $context['sandbox']['cur_feed_nr'] ]['class'];
    $host = $context['sandbox']['feeds'][  $context['sandbox']['cur_feed_nr']] ['host'];
    $server_key = $context['sandbox']['feeds'][  $context['sandbox']['cur_feed_nr'] ]['server_key'];
    // create the feed reader
    $context['sandbox']['cur_feed'] = new $class($host, $server_key, $context['sandbox']['stepping']);
    // fetch the complete project list and save it in a temporary file
    try {
      $context['sandbox']['cur_feed']->fetch_project_list();
      $context['sandbox']['processing_feed'] = TRUE;
    }
    catch( Exception $e) {
      drupal_set_message($e->getMessage(),'error');
      // this feed failed, proceed to the next
      $context['sandbox']['processing_feed'] = FALSE;
    }
  }
  
  $status = $context['sandbox']['cur_feed']->fetch_next_projects();
  if($status === TRUE) {
      $context['finished'] = 1;
  }
  // else
  $context['finished'] = (1 / $status);
}

function _update_feed_api_refresh_finished() {
  drupal_set_message('All update-feeds have been updated');
  return '';
}
