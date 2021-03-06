<?php
// $Id$

// Copyright (c) 2010 KontextWork
// Author: Eugen Mayer

// A genera Drupal Project "Project" with its meta data
class Project extends FeedEntryContainer {
  /**
   * @var Release[]
   */
  protected $releases = array();
  /**
   * @var string
   */
  protected $server_key = NULL;

  public function __construct($infos, $terms, $server_key, $load_release = TRUE) {
    // title, short_name, api_version, recommended_version, supported_major_default_major, project_status, link
    parent::__construct($infos, $terms);
    $this->dest_table = 'update_feed_api_projects';
    $this->prim_key_field = 'pid';
    $this->server_key = $server_key;
    // if we should load release and we have a primary key already
    // (project loaded from the DB) load all releases
    $this->infos['type'] = $this->type();
    if (($load_release) && $this->primary_key_exists($this->infos)) {
      $this->load_releases();
    }
    ;
  }

  /**
   * @param array $infos
   * @param array $terms
   * add a new release to the project. This will not store it in the DB
   */
  public function add_release($infos, $terms) {
    $this->releases[$infos['version']] = new Release($infos, $terms, $this);
    $this->sort_releases();
  }

  /**
   * @return Release[] keyed by the version string
   */
  public function get_releases() {
    return $this->releases;
  }

  /**
   * @param string|null $version
   * @return Release
   * @deprecated
   */
  public function get_release($version = NULL) {
    return $this->get_release_by_version($version);
  }

  /**
   * @param $version
   * @return Release
   * @throws Exception
   * returns a release identified by its version string
   */
  public function get_release_by_version($version) {
    foreach ($this->releases as $release) {
      if ($release->get_info('version') == $version) {
        return $release;
      }
      //else just continue
    }

    throw new Exception('(Project: ' . $this->infos['short_name'] . ' ) No release found with the version ' . $version);
  }

  public function server_key() {
    return $this->server_key;
  }

  /**
   * persists the project and its releases into the database
   */
  public function save() {
    parent::save();
    // and now save all releases
    foreach ($this->releases as $release) {
      $release->save();
    }
  }

  /**
   * @param string $key
   * @return array
   * transforms the releases to ann assoc array
   */
  public function releases_to_array($key = 'version') {
    $this->sort_releases();
    $result = array();
    foreach ($this->releases as $r) {
      $result[$r->get_info('version')] = $r->get_info($key);
    }

    return $result;
  }

  /**
   * @return bool
   * is this project fetched with all meta-informations already? This will set to false
   * if the project-details have yet not beed enriched by the meta-informations found on the specific
   * release feed of a project core version
   */
  public function full_fetched() {
    return $this->infos['full_fetched'] != '' ? TRUE : FALSE;
  }

  public function remove() {
    // if we dont have a prim key yet, its not even in a DB.
    // so we dont need to delete the project
    if ($this->primary_key_exists($this->infos)) {
      db_delete('update_feed_api_projects')
        ->condition('pid', $this->infos[$this->prim_key_field])
        ->execute();
      db_delete('update_feed_api_releases')
        ->condition('pid', $this->infos[$this->prim_key_field])
        ->execute();
    }
  }

  /**
   * @return string type of the project
   */
  public function type() {
    return self::terms_to_type($this->get_terms());
  }


  /**
   * @static
   * @param $terms
   * @return string
   * mapping table for terms to a project type
   */
  static function terms_to_type($terms) {
    $type = $terms['Projects'];
    if (!is_array($type)) {
      $type = array($type);
    }
    foreach ($type as $t) {
      switch ($t) {
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

  /**
   * @param null $label
   * @return mixed
   */
  public function project_link($label = NULL) {
    if (!empty($label)) {
      return l($label, $this->infos['link']);
    }
    // 
    return $this->infos['link'];
  }

  /**
   * @return string
   * returns the SCM type, if available
   */
  public function sourcecode_type() {
    switch ($this->infos['sourcecode_type']) {
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

  /**
   * @return string
   */
  public function sourcecode_uri() {
    return $this->infos['sourcecode_uri'];
  }

  /**
   * sorts releases
   */
  protected function sort_releases() {
    usort($this->releases, array($this, 'compare_releases'));
  }

  /**
   * @param Release $a
   * @param Release $b
   * @return mixed
   */
  protected function compare_releases($a, $b) {
    return $a->compare_versions($b->get_info('version'), $a->get_info('version'), 'mj');
  }

  protected function validate_terms($terms) {
    // TODO: is this required?
    $req = array();
    $diff = array_diff($req, array_keys($terms));
    if (count($diff) > 0) {
      throw new Exception('Not all required terms set from project. Missing: (' . join(',', $diff) . ')');
    }
  }

  protected function validate_infos($infos) {
    $req = array('title', 'short_name', 'api_version', 'link');
    $diff = array_diff($req, array_keys($infos));
    if (count($diff) > 0) {
      throw new Exception('Not all required infos set for project. Missing: (' . join(',', $diff) . ')');
    }
  }

  protected function alter_db_fields(&$fields) {
    $fields['server_key'] = $this->server_key();
    parent::alter_db_fields($fields);
  }

  /**
   * loads the releases and stores them in the object
   */
  protected function load_releases() {
    if (!$this->full_fetched()) {
      update_feed_api_fetch_releases($this, $this->infos['api_version']);
      // our pid has changed, so we need to reload ourselfs completely.
      $query = db_select('update_feed_api_projects', 'ufap');
      $query->condition('ufap.server_key', $this->server_key);
      $query->condition('ufap.short_name', $this->infos['short_name']);
      $query->condition('ufap.api_version', $this->infos['api_version']);
      $query->condition('ufap.type', $this->infos['type']);
      $query->fields('ufap', array('pid'));
      $result = $query->execute();
      $record = $result->fetchObject();

      $query = db_select('update_feed_api_projects', 'ufap');
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
    if ($prim_key == FALSE) {
      // no need to load release, as we did not get saved in the DB yet
      return;
    }
    $query = db_select('update_feed_api_releases', 'ufar');
    $query->condition($prim_key, $this->infos[$prim_key]);
    $query->fields('ufar', array('rid'));
    $result = $query->execute();

    foreach ($result as $record) {
      $release = Release_factory::load_by_rid($record->rid, $this);
      $this->releases[$release->get_info('version')] = $release;
    }

    $this->sort_releases();
  }
}
