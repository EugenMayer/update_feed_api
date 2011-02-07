<?php
// $Id$

// Copyright (c) 2010 KontextWork
// Author: Eugen Mayer

// A genera Drupal Project "Project" with its meta data
class Project extends FeedEntryContainer {
  protected $releases = array();
  protected $server_key = NULL;
  
  public function __construct($infos, $terms, $server_key, $load_release = TRUE) {
    // title, short_name, api_version, recommended_version, supported_major_default_major, project_status, link
    parent::__construct($infos, $terms);
    $this->dest_table = 'update_feed_api_projects';
    $this->prim_key_field = 'pid';
    $this->server_key = $server_key;
    
    // if we should load release and we have a primary key already
    // (project loaded from the DB) load all releases
    if(($load_release) && $this->primary_key_exists($this->infos)) {
      $this->load_releases();
    };
  }
  
  public function add_release($infos, $terms) {
    
    $this->releases[$infos['date']] = new Release($infos, $terms, $this);
    $this->sort_releases();
  }
  
  public function get_release($date = NULL) {
    if($date != NULL) {
      $this->releases[$date];
    }
    // else
    return $this->releases;
  }
  
  public function get_release_by_version($version) {
    foreach($this->releases as $release) {
      if($release->get_info('version') == $version) {
        return $release;
      }
      //else just continue
    }
    
    throw new Exception('(Project: '.$this->infos['short_name'].' ) No release found with the version '.$version);
  }
  
  public function server_key() {
    return $this->server_key;
  }
  
  public function save() {
    parent::save();
    // and now save all releases
    foreach($this->releases as $release) {
      $release->save();
    }
  }
  
  public function releases_to_array($key = 'version') {
    $this->sort_releases();
    $result = array();
    foreach($this->releases as $r) {
      $result[$r->get_info('version')] = $r->get_info($key); 
    }
    
    return $result;
  }
  
  public function full_fetched() {
    return $this->infos['full_fetched'] != '' ? TRUE : FALSE;
  }
  
  public function remove() {
    // if we dont have a prim key yet, its not even in a DB.
    // so we dont need to delete the project
    if($this->primary_key_exists($this->infos)) {
      db_delete('update_feed_api_projects')
      ->condition('pid',$this->infos[$this->prim_key_field])
      ->execute();
      db_delete('update_feed_api_releases')
      ->condition('pid',$this->infos[$this->prim_key_field])
      ->execute();
    }
  }
  
  public function type() {
    try {
      // Its part of the project meta data, so check there.
      $type = $this->get_term('Projects');
    }
    catch(Exception $e) {
      // as this term is not set, we cant determine the type at all
      return 'UNKNOWN';
    }
    if(!is_array($type)) {
      $type = array($type);
    }
    foreach($type as $t) {
      switch($t) {
        case 'Modules':
          return 'module';
        break;
        case 'Themes':
          return 'theme';
        break;
        case 'Libraries':
          return 'libraries';
        break;
        case 'Installation profiles':
          return 'profile';
        break;
        case 'Theme engines':
          return 'theme_engine'; 
        break;
        case 'Translations':
          return 'translation';
        break;
        case 'Drupal project':        
          return 'core';
        break;        
      }
    }
    return 'UNKNOWN';
  }
  public function project_link($label = NULL) {
     if(!empty($label)) {
      return l($label,$this->infos['link']);
    }
    // 
    return $this->infos['link'];    
  }

  public function sourcecode_type() {
    switch($this->infos['sourcecode_type']) {
      case 'git':
        return 'git';
      break;
      case 'GIT repository tags':
        return 'git';
      break;
      default:
        return $this->infos['sourcecode_type'];
      break;
    }
  }

  public function sourcecode_uri() {
    return $this->infos['sourcecode_uri'];      
  }

  protected function sort_releases() {
    usort($this->releases, array($this, 'compare_releases'));
  }

  protected function compare_releases($a, $b) {
    $a_val = $a->get_info('date');
    $b_val = $b->get_info('date');
    if($a_val == $b_val) {
      return 0;
    }
    
    return $a_val < $b_val ? 1 : -1;
  }
  
  protected function validate_terms($terms)  {
    // TODO: is this required?
    $req = array ();
    $diff = array_diff($req, array_keys($terms));
    if (count($diff) >0 ) {
      throw new Exception('Not all required terms set from project. Missing: ('.join(',',$diff).')');
    }
  }
  
  protected function validate_infos($infos) {
    $req = array ( 'title', 'short_name', 'api_version', 'link');
    $diff = array_diff($req, array_keys($infos));
    if (count($diff) >0 ) {
      throw new Exception('Not all required infos set for project. Missing: ('.join(',',$diff).')');
    }
  }
  
  protected function alter_db_fields(&$fields) {
    $fields['server_key'] = $this->server_key();
    parent::alter_db_fields($fields);
  }

  protected function load_releases() {
    if(!$this->full_fetched()) {      
      update_feed_api_fetch_releases($this, $this->infos['api_version']);
      // our pid has changed, so we need to reload ourselfs completely.
      $query = db_select('update_feed_api_projects','ufap');
      $query->condition( 'ufap.server_key', $this->server_key);
      $query->condition( 'ufap.short_name' , $this->infos['short_name']);
      $query->condition( 'ufap.api_version' , $this->infos['api_version']);
      $query->fields('ufap',array('pid'));
      $result = $query->execute();
      $record = $result->fetchObject();

      $query = db_select('update_feed_api_projects','ufap');
      $query->condition('ufap.pid', $record->pid);
      $query->fields('ufap');
      $result = $query->execute();
      $array = $result->fetchAssoc();
      
      $terms = unserialize($array['terms']);
      $server_key = $array['server_key'];
      unset($array['terms'], $array['server_key']);
      $info = $array;
      // this will rebuild our object and then call load_releases again.
      $this->__construct($info, $terms, $server_key, TRUE);
      return;
    }
  
    $prim_key = $this->primary_key_exists($this->infos);
    if($prim_key == FALSE) {        
      // no need to load release, as we did not get saved in the DB yet
      return;
    }
    $query = db_select('update_feed_api_releases','ufar');
    $query->condition( $prim_key, $this->infos[$prim_key]);
    $query->fields('ufar', array('rid') );
    $result = $query->execute();
    
    foreach($result as $record) {
      $release = Release_factory::load_by_rid($record->rid, $this);      
      $this->releases[$release->get_info('date')] = $release;
    }

    $this->sort_releases();
  }
}
