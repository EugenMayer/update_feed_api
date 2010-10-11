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
  }
  
  public function get_release($date = NULL) {
    if($date != NULL) {
      $this->releases[$date];
    }
    // else
    return $this->releases;
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
    $result = array();
    foreach($this->releases as $r) {
      $result[$r->get_info('version')] = $r->get_info($key); 
    }
    
    return $result;
  }
  
  public function full_fetched() {
    return $this->infos['full_fetched'] == 1 ? TRUE : FALSE;
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
      throw new Exception('Not all required infos set from project. Missing: ('.join(',',$diff).')');
    }
  }
  
  protected function alter_db_fields(&$fields) {
    $fields['server_key'] = $this->server_key();
    parent::alter_db_fields($fields);
  }

  protected function load_releases() {
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
  }
}