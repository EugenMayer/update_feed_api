<?php

// A genera Drupal Project "Project" with its meta data
class Project extends FeedEntryContainer {
  protected $releases = array();
  protected $server_key = NULL;
  public function __construct($info, $terms, $server_key) {
    // title, short_name, api_version, recommended_version, supported_major_default_major, project_status, link
    parent::__construct($info, $terms);
    $this->dest_table = 'update_feed_api_projects';
    $this->server_key = $server_key;
  }
  
  public function add_release($info, $terms) {
    $this->releases[$info['date']] = new Release($info, $terms, $this);
  }
  
  public function server_key() {
    return $this->server_key;
  }
  
  protected function validate_terms($terms)  {
    $req = array ( 'Projects' );
    $diff = array_diff($req, array_keys($terms));
    if (count($diff) >0 ) {
      throw new Exception('Not all required terms set from project. Missing: ('.join(',',$diff).')');
    }
  }
  
  protected function validate_info($infos) {
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
    if(array_key_exists('pid', $fields)) {
      return 'pid';
    }
    // else
    return FALSE;
  }
}