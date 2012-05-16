<?php
/**
 * Created for Drupal Wiki.
 * User: eugenmayer
 * Date: 15.05.12
 * Time: 09:55
 */

class Release_list {
  protected $max_count = NULL;
  protected $defaults = array();
  protected $releases = NULL;
  protected $project = NULL;

  /**
   * @param Release[]|null $releases optional list of releases to operate on
   * @param Project $project if set, releases of this project are used as the search bas
   */
  public function __construct(Release $releases = NULL, Project $project = NULL) {
    if ($releases == NULL AND $project == NULL) {
      throw new Exception("Please the the search base ( releases ) or the project for a Release_list");
    }
    $this->project = $project;
    $this->releases = $releases;
  }

  /**
   * @param string $server_key the server key of the project
   * @param string $project_shortname project identifier
   * @param string $api_version api version of the project
   * @return mixed
   */
  protected function ensure_searchbase() {
    if ($this->releases == NULL) { // we ensured in the constructor, that project is set, if release is not
      $this->releases = $this->project->get_releases();
    }
  }

  public function filter_by_major($mj) {
    $this->ensure_searchbase();
    foreach ($this->releases as $version => $r) {
      if ($r->get_info("version_major") != $mj) {
        unset($this->releases[$version]);
      }
    }
    return $this;
  }

  /**
   * @param Release $a
   * @param Release $b
   * @return mixed
   */
  protected function compare_release_versions($a, $b) {
    return $a->compare_versions($b->get_info('version'), $a->get_info('version'), 'mj');
  }

  /**
   * @return Release_list
   * the most recent release will be on index 0, first position
   */
  public function sort_by_version() {
    $this->ensure_searchbase();
    usort($this->releases, array($this, 'compare_release_versions'));
    return $this;
  }

  /**
   * @return Release[]
   */
  public function get_releases() {
    return $this->releases;
  }

  /**
   * @return Release
   */
  public function first() {
    if (is_array($this->releases) && count($this->releases) > 0) {
      return $this->releases[0];
    }
    return NULL;
  }

  /**
   * @return Release
   */
  public function last() {
    if (is_array($this->releases) && count($this->releases) > 0) {
      return $this->releases[count($this->releases) - 1];
    }
    return NULL;
  }
}