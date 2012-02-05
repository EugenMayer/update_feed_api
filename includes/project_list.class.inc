<?php
/*
 * A helper class to maintain lists of projects
 * Be aware that by defintion the default project list cannot be an overlay
 * so it is not possible to have the same modules from 2 sources
 */
class Project_list {
  protected $max_count = NULL;
  protected $defaults = array();
  /**
   * @var Project[]
   */
  protected $projects = array();

  public function __construct($defaults = array(), $max_count = NULL) {
    $this->max_count = $max_count;
    $this->defaults = $defaults;
  }

  public function search($keyword, $server_key = NULL, $api_version = NULL, $type = NULL) {
    // making server_key / api_version an argument is just a helper for later
    // integration of project lists, which support overlays
    if ($api_version == NULL && array_key_exists('api_version', $this->defaults)) {
      $api_version = $this->defaults['api_version'];
    }
    else {
      throw new Exception('please pass the api_version or pass it as default');
    }

    if ($server_key == NULL && array_key_exists('server_key', $this->defaults)) {
      $server_key = $this->defaults['server_key'];
    }
    else {
      throw new Exception('please pass the server_key or pass it as default');
    }
    if ($type == NULL && array_key_exists('type', $this->defaults)) {
      $type = $this->defaults['type'];
    }
    $this->projects = Project_factory::load_projects($server_key, $api_version, FALSE, $keyword, $type);
  }
 /**
  * @return Project[]
  */
  public function get_projects() {
    return $this->projects;
  }

  /**
   * @param Project $p
   */
  public function add_project($p) {
    $this->projects[$p->get_info('short_name') . '_' . $p->get_info('type')] = $p;
  }

  public function delete_project($short_name) {
    if ($this->has_project($short_name)) {
      unset($this->projects[$short_name]);
    }
  }
  /**
   * @param $short_name
   * @param string $type
   * @return bool
   */
  public function has_project($short_name, $type = 'module') {
    return array_key_exists($short_name . "_$type", $this->projects);
  }

  /**
   * @param string $key
   * @return array
   */
  public function to_array($key = 'short_name') {
    $result = array();
    foreach ($this->projects as $p) {
      $result[$p->get_info('short_name') . '_'.$p->type()] = $p->get_info($key);
    }

    return $result;
  }
}
