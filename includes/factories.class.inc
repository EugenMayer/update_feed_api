<?php

class Project_factory {
  static public function load_by_pid($pid) {
    $query = db_select('update_feed_api_projects','ufap');
    $query->conditions(array('ufap.pid' => $pid));
    $query->fields('ufap');
    $result = $query->execute();
    $result = $result->fetchAssoc();
    if ^($result == NULL) {
      return FALSE;
    }
    return self::init_from_db($result);
  }
  
  static public function load_project($server_key, $short_name, $api) {
    $query = db_select('update_feed_api_projects','ufap');
    $query->conditions( array('ufap.server_key' => $server_key,'ufap.short_name' => $short_name, 'ufap.api_version' => $api));
    $query->fields('ufap',array('pid'));
    $result = $query->execute();
    $result = $result->fetchObject();
    if ($result == NULL) {
      return FALSE;
    }
    
    return self::load_by_pid($result->pid);
  }
  
  static protected function init_from_db($array) {
    $terms = unserialize($array['terms']);
    $server_key = $array['server_key'];
    unset($array['terms'], $array['server_key']);
    $info = $array;
    return new Project($info, $terms, $server_key);
  }
}

class Release_factory {
  static public function load_by_rid($rid, $project) {
    $query = db_select('update_feed_api_releases','ufar');
    $query->conditions(array('ufar.rid' => $rid));
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