<?php
// A general Drupal Project release with its meta data
class Release extends FeedEntryContainer{
  protected $parent_project = NULL;
  public function __construct($infos, $terms, $project) {
    parent::__construct($infos, $terms);
    $this->parent_project = $project;
    $this->dest_table = 'update_feed_api_releases';
    $this->prim_key_field = 'rid';
  }
  
  protected function validate_terms($terms)  {
    $req = array ( 'Release type' ); 
    $diff = array_diff($req, array_keys($terms));
    if (count($diff) >0 ) {
      throw new Exception('Not all required terms set from release. Missing: ('.join(',',$diff).')');
    }
  }
  protected function validate_infos($infos) {
    $req = array ( 'name', 'version', 'tag', 'version_major', 'version_extra', 
                  'status', 'release_link', 'download_link', 'date', 'mdhash', 'filesize' );
    $diff = array_diff($req, array_keys($infos));
    if (count($diff) >0 ) {
      throw new Exception('Not all required infos set from release. Missing: ('.join(',',$diff).')');
    }
  }
  
  protected function alter_db_fields(&$fields) {
    $fields['pid'] = $this->parent_project->get_info('pid');
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