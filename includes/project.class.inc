<?php

// A genera Drupal Project "Project" with its meta data
class Project extends FeedEntryContainer {
  protected $releases = array();
  protected $server_key = NULL;
  public function __construct($info, $terms, $server_key) {
    // title, short_name, api_version, recommended_version, supported_major_default_major, project_status, link
    parent::__construct( $info, $terms);
    $this->server_key = $server_key;
  }
  
  public function add_release($info, $terms) {
    if(in_array("Release", class_parents(get_class($release))) {
      $this->releases[$info['date']] = new Release($info, $terms, $this);
    }
    else {
      throw new Exception("Cant add release, does not inherit from Release!");
    }
  }
  
  public function server_key() {
    return $this->server_key;
  }
  
  protected function validate_terms($terms)  {
    $req = array ( 'Projects' );
    if(!$this->array_keys_exists($req, $terms)) {
      throw new Exception('Not all required terms set from project. Required: ('.join(',',$req).')');
    }
  }
  
  protected function validate_info($info) {
    $req = array ( 'title', 'short_name', 'api_version', 'recommended_version', 'supported_majors', 
                  'default_major', 'project_status', 'link',);
    if(!$this->array_keys_exists($req, $info)) {
      throw new Exception('Not all required infos set from project. Required: ('.join(',',$req).')');
    }
  }
  protected function add_db_field(&$fields) {
    $fields['server_key'] = $this->server_key();
    parent::add_db_field($fields);
  }
}