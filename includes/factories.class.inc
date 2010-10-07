<?php

class Project_factory {
  static public function load_project_by_pid($pid) {
    $query = db_select('update_feed_api_projects','ufa');
    $query->conditions(array('ufa.pid' => $pid));
    $query->fields('ufa');
    $result = $query->execute();
    if($result == NULL) {
      return FALSE;
    }
    $result = $result->fetchAssoc();
    
    return self::init_from_db($result);
  }
  
  static public function load_project($server_key, $short_name) {
    $query = db_select('update_feed_api_projects','ufa');
    $query->conditions( array('ufa.server_key' => $server_key,'ufa.short_name' => $short_name));
    $query->fields('ufa',array('pid'));
    $result = $query->execute();
    if($result == NULL) {
      return FALSE;
    }
    $result = $result->fetchObject();
    return self::load_project_by_pid($result->pid);
  }
  
  static protected function init_from_db($array) {
    $terms = unserialize($array['terms']);
    $server_key = $array['server_key'];
    unset($array['terms'], $array['server_key']);
    $info = $array;
    return new Project($info, $terms, $server_key);
  }
}