<?php
// $Id$

// Copyright (c) 2010 KontextWork
// Author: Eugen Mayer

// A general Drupal Project release with its meta data
class Release extends FeedEntryContainer{
  protected $parent_project = NULL;
  public function __construct($infos, $terms, $project) {
    parent::__construct($infos, $terms);
    $this->parent_project = $project;
    $this->dest_table = 'update_feed_api_releases';
    $this->prim_key_field = 'rid';
  }
  
  public function type() {
    return $this->parent_project->type();
  }
  
  public function server_key() {
    return $this->parent_project->server_key();
  }
  
  public function module_name() {
    return $this->parent_project->get_info('short_name');
  }

  public function release_link($label = NULL) {
    if(!empty($label)) {
      return l($label,$this->infos['release_link']);
    }
    // 
   return $this->infos['release_link'];    
  }
  protected function validate_terms($terms)  {
    $req = array (); 
    $diff = array_diff($req, array_keys($terms));
    if (count($diff) >0 ) {
      throw new Exception('Not all required terms set from release. Missing: ('.join(',',$diff).')');
    }
  }
  protected function validate_infos($infos) {
    $req = array ( 'name', 'version','version_major',
                  'status', 'release_link', 'download_link', 'date', 'mdhash', 'filesize' );
    $diff = array_diff($req, array_keys($infos));
    if (count($diff) >0 ) {
      throw new Exception('Not all required infos set from release. Missing: ('.join(',',$diff).')');
    }
  }
  
  protected function alter_db_fields(&$fields) {
    $fields['pid'] = $this->parent_project->get_info('pid');
    $fields['server_key'] = $this->parent_project->server_key();
    parent::alter_db_fields($fields);
  }
  
  protected function get_primary_key(&$fields) {
    if (array_key_exists($this->prim_key_field, $fields)) {
      return $this->prim_key_field;
    }
    // else
    return FALSE;
  }
}