<?php

// A genera Drupal Project "Project" with its meta data
class Project extends FeedEntryContainer {
  protected $releases = array();
  public function __construct($info) {
    // title, short_name, api_version, recommended_version, supported_major_default_major, project_status, link
    $this->meta = $info;
  }
  
  public function add_release($release) {
    if(in_array("Release", class_parents(get_class($release))) {
      $this->releases[$release->get_info('date')] = $release;
    }
    else {
      throw new Exception("Cant add release, does not inherit from Release!");
    }
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
}