<?php
// $Id$

// Copyright (c) 2010 KontextWork
// Author: Eugen Mayer

/*
 * Search and get projects of a specific api of a specifc server matching the short_name / title
 */
function update_feed_api_search_projects($server_key, $api_version, $search, $type = NULL) {
  $projects = Project_factory::load_projects($server_key, $api_version, FALSE, $search, $type);
  return $projects;
}

/*
 *  Load all projects of a specific api of a specifc server
 */
function update_feed_api_get_projects($server_key, $api_version, $load_releases = TRUE, $type = NULL) {
  $projects = Project_factory::load_projects($server_key, $api_version, $load_releases, NULL, $type);
  return $projects;
}

/*
 *  Load a specific projects of a specific api of a specific server
 */
function update_feed_api_get_project($server_key, $short_name, $api_version, $load_releases = TRUE, $type = 'module') {
  $project = Project_factory::load_project($server_key, $short_name, $api_version, $load_releases, $type);
  return $project;
}

/*
 *  Load  same as update_feed_api_get_project, but loading by pid directly
 */
function update_feed_api_api_get_project_by_pid($pid, $load_releases = TRUE) {
  $project = Project_factory::load_by_pid($pid, $load_releases);
  return $project;
}

/*
 * Get all available api version for this project
 */
function update_feed_api_get_project_apis($server_key, $short_name) {
  $query = db_select('update_feed_api_projects', 'ufap');
  $query->condition('ufap.server_key', $server_key);
  $query->condition('ufap.short_name', $short_name);
  $query->fields('ufap', array('api_version'));
  $result = $query->execute();
  if ($result == NULL) {
    return FALSE;
  }

  $apis = array();
  foreach ($result as $api) {
    $apis[] = $api;
  }
  return $apis;
}

/*
 * Running this for a project, the update-feed for this project is read
 * and all releases are extracted
 */
function update_feed_api_fetch_releases($project, $api_version) {
  $feeds = _update_feed_api_get_feeds(array($project->server_key()));
  $fetcher = new $feeds[0]['class']($feeds[0]['host'], $feeds[0]['server_key'], 10, NULL, _update_feed_api_allowed_apis());
  $projects = $fetcher->fetch_project($project, array($api_version));
  return $projects[$api_version];
}

function update_feed_api_update_feed($feed, $fetch_releases = FALSE) {
  $class = $feed['class'];
  $server_key = $feed['server_key'];
  $host = $feed['host'];
  $feedObj = new $class($host, $server_key, variable_get('update_feed_api_stepping', 10), NULL, _update_feed_api_allowed_apis());
  $feedObj->fetch_project_list();
  while ($status !== TRUE) {
    $status = $feedObj->fetch_next_projects($fetch_releases, TRUE);
  }
}

function _update_feed_api_package_types() {
  return array('module', 'libraries', 'core', 'theme', 'profile', 'theme_engine', 'translation');
}
