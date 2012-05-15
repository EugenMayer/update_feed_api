<?php
// $Id$

// Copyright (c) 2010 KontextWork
// Author: Eugen Mayer

// A general Drupal Project release with its meta data
class Release extends FeedEntryContainer {
  protected $parent_project = NULL;

  public function __construct($infos, $terms, $project) {
    parent::__construct($infos, $terms);
    $this->parent_project = $project;
    $this->dest_table = 'update_feed_api_releases';
    $this->prim_key_field = 'rid';
  }

  /**
   * @return string
   * returns the type of the parent-project
   */
  public function type() {
    return $this->parent_project->type();
  }

  /**
   * @return string
   * returns the server identifier the release has been fetched from
   */
  public function server_key() {
    return $this->parent_project->server_key();
  }

  /**
   * @return string
   * module-name (unique)
   */
  public function module_name() {
    return $this->parent_project->get_info('short_name');
  }

  /**
   * @return Project
   */
  public function get_project() {
    return $this->parent_project;
  }

  /**
   * @param $type
   * @return bool|Release
   * returns false, if there is no newer release or the release, if there is one
   */
  public function has_newer_release($type) {
    $releases = $this->parent_project->releases_to_array('version');
    $result = FALSE;
    $cv = $this->infos['version'];
    foreach ($releases as $ver => $r_key) {
      // if current verison is smaller then the checked one
      if ($this->compare_versions($cv, $ver, $type) == -1) {
        // and the curretn result is smaller then the checked one
        if (
          $result == FALSE ||
          $this->compare_versions($result['ver'], $ver, $type) == -1
        ) {
          // set the result
          $result = array('ver' => $ver, 'key' => $r_key);
        }
        ;
      }
    }
    if ($result != FALSE) {
      return $this->parent_project->get_release($result['key']);
    }
    // else
    return FALSE;
  }

  /**
   * @param $cv
   * @param $v
   * @param $type
   * @return int
   * compares to version, see php compare methods
   */
  public function compare_versions($cv, $v, $type) {
    $cv = $this->parse_version($cv);
    $v = $this->parse_version($v);
    switch ($type) {
      case 'mj':
        if ($cv['mj'] < $vv['mj']) {
          return -1;
        }
        else if ($cv['mj'] > $v['mj']) {
          return 1;
        }
      // else just check the minors
      case 'mn':
        if ($cv['mj'] != $v['mj']) {
          return 1;
        }
        // if the minor is small, its smaller :)
        if (is_numeric($v['mn']) && $cv['mn'] < $v['mn']) {
          return -1;
        }
        // the minor is bigger and mj is equal, so its bigger
        else if (is_numeric($v['mn']) && $cv['mn'] > $v['mn']) {
          return 1;
        }
        else if (is_numeric($v['mn']) && $cv['mn'] == $v['mn']) {
          if ($cv['extra'] != '' && $this->compare_version_extra($cv, $v) == -1) {
            return -1;
          }
        }
        break;
    }
    return 1;
  }

  /**
   * @param $a
   * @param $b
   * @return int
   * compares the extra part of a version string
   */
  protected function compare_version_extra($a, $b) {
    $a['extrav'] = $a['extrav'] == '' ? 0 : $a['extrav'];
    $b['extrav'] = $b['extrav'] == '' ? 0 : $b['extrav'];
    $extra = $this->compare_extra_string($a['extra'], $b['extra']);
    if ($extra == 0) {
      return $a['extrav'] < $b['extrav'] ? -1 : 1;
    }
    return $extra;
  }

  protected function compare_extra_string($a, $b) {
    $pattern = array('dev', 'alpha', 'beta', 'rc');
    $replace = array(1, 2, 3, 4);
    $a = str_replace($pattern, $replace, $a);
    $b = str_replace($pattern, $replace, $b);
    if ($a == $b) {
      return 0;
    }
    return $a < $b ? -1 : 1;
  }

  /**
   * @param $version
   * @return array
   * splits an version string into its specific parts (core, mj, mn, extra, extrav)
   */
  public function parse_version($version) {
    // example: 6.x-2.0-rc3
    $pattern = '/^(?<core>\d\.\w)-(?<mj>\d+)\.(?<mn>\w+)-?(?<extra>(rc|dev|beta|alpha))?(?<extrav>\d+)?$/';
    if (preg_match($pattern, $version, $matches)) {
      return $matches;
    }
    ;
    return array();
  }

  /**
   * @param null $label
   * @return string
   */
  public function release_link($label = NULL) {
    if (!empty($label)) {
      return l($label, $this->infos['release_link']);
    }
    // 
    return $this->infos['release_link'];
  }

  protected function validate_terms($terms) {
    $req = array();
    $diff = array_diff($req, array_keys($terms));
    if (count($diff) > 0) {
      throw new Exception('Not all required terms set from release. Missing: (' . join(',', $diff) . ')');
    }
  }

  protected function validate_infos($infos) {
    $req = array('name', 'version', 'version_major',
      'status', 'release_link', 'download_link', 'date', 'mdhash', 'filesize');
    $diff = array_diff($req, array_keys($infos));
    if (count($diff) > 0) {
      throw new Exception('Not all required infos set from release. Missing: (' . join(',', $diff) . ')');
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
