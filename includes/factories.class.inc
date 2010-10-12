<?php
// $Id$

// Copyright (c) 2010 KontextWork
// Author: Eugen Mayer

class Project_factory {
  static public function load_by_pid($pid, $load_release = TRUE) {
    $query = db_select('update_feed_api_projects','ufap');
    $query->condition('ufap.pid', $pid);
    $query->fields('ufap');
    $result = $query->execute();
    
    $result = $result->fetchAssoc();
    if ($result == NULL) {
      return FALSE;
    }
    return self::init_from_db($result, $load_release);
  }
  
  static public function load_project($server_key, $short_name, $api, $load_release = TRUE) {
    $query = db_select('update_feed_api_projects','ufap');
    $query->condition( 'ufap.server_key',$server_key);
    $query->condition( 'ufap.short_name' , $short_name);
    $query->condition( 'ufap.api_version' , $api);
    $query->fields('ufap',array('pid'));
    $result = $query->execute();
    $result = $result->fetchObject();
    if ($result == NULL) {
      return FALSE;
    }
    
    return self::load_by_pid($result->pid, $load_release);
  }
  
  static public function load_projects($server_key, $api, $load_release = TRUE, $search = NULL) {
    $query = db_select('update_feed_api_projects','ufap');
    $query->condition( 'ufap.server_key', $server_key);
    $query->condition( 'ufap.api_version' , $api);
    if($search != NULL && is_string($search)) {
      $con = db_or()
        ->condition( 'ufap.short_name', "%" . db_like($search)  . "%", ' LIKE ')
        ->condition( 'ufap.title', "%" . db_like($search)  . "%", ' LIKE ');
          
      $query->condition( $con );
    }
    $query->fields('ufap',array('pid'));
    $result = $query->execute();
    if ($result == NULL) {
      return FALSE;
    }
    
    $projects = array();
    foreach($result as $record) {
      $project = self::load_by_pid($record->pid, $load_release);
      
      $projects[$project->get_info('short_name')] = $project;
    }
    
    return $projects;
  }
  
  static protected function init_from_db($array, $load_release = TRUE) {
    $terms = unserialize($array['terms']);
    $server_key = $array['server_key'];
    unset($array['terms'], $array['server_key']);
    $info = $array;
    return new Project($info, $terms, $server_key, $load_release);
  }
}

class Release_factory {
  static public function load_by_rid($rid, $project) {
    $query = db_select('update_feed_api_releases','ufar');
    $query->condition('ufar.rid', $rid);
    $query->fields('ufar');
    $result = $query->execute();
    $result = $result->fetchAssoc();
    if ($result == NULL) {
      return FALSE;
    }
    return self::init_from_db($result,$project);
  }
  
  static protected function init_from_db($array, $project) {
    $terms = unserialize($array['terms']);
    unset($array['terms']);
    $info = $array;
    return new Release($info, $terms, $project);
  }
}