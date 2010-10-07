<?php

// A genera Drupal Project "Project" with its meta data
class Project extends FeedEntryContainer {
  protected $releases = array();
  protected $server_key = NULL;
  public function __construct($infos, $terms, $server_key, $load_release = TRUE) {
    // title, short_name, api_version, recommended_version, supported_major_default_major, project_status, link
    parent::__construct($infos, $terms);
    $this->dest_table = 'update_feed_api_projects';
    $this->server_key = $server_key;
    // if we should load release and we have a primary key already
    // (project loaded from the DB) load all releases
    if(($load_release) && $this->get_primary_key($this->infos)) {
      $this->load_releases();
    };
  }
  
  public function add_release($infos, $terms) {
    $this->releases[$info['date']] = new Release($infos, $terms, $this);
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
  protected function validate_terms($terms)  {
    // TODO: is this required?
    $req = array ( 'Projects' );
    $req = array ();
    $diff = array_diff($req, array_keys($terms));
    if (count($diff) >0 ) {
      throw new Exception('Not all required terms set from project. Missing: ('.join(',',$diff).')');
    }
  }
  
  protected function validate_infos($infos) {
    $req = array ( 'title', 'short_name', 'api_version', 'recommended_major', 'supported_majors', 
                  'default_major', 'project_status', 'link',);
    $diff = array_diff($req, array_keys($infos));
    if (count($diff) >0 ) {
      throw new Exception('Not all required infos set from project. Missing: ('.join(',',$diff).')');
    }
  }
  
  protected function add_db_field(&$fields) {
    $fields['server_key'] = $this->server_key();
    parent::add_db_field($fields);
  }
  
  protected function get_primary_key(&$fields) {
    if (array_key_exists('pid', $fields)) {
      return 'pid';
    }
    // else
    return FALSE;
  }
  
  protected function load_releases() {
    $prim_key = $this->get_primary_key($this->infos);
    
    $query = db_select('update_feed_api_releases','ufar');
    $query->condition( 'server_key', $this->server_key);
    $query->condition( 'project_short_name', $this->infos['short_name'] );
    $query->fields('ufar', array('rid') );
    
    $result = $query->execute();

    //if($result == NULL)
    foreach($result as $record) {
      $release = Release_factory::load_by_rid($record->rid, $this);
      $this->releases[$release->get_info('date')] = $release;
    }
  }
}