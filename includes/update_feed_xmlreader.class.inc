<?php
/*
 * Parses feeds using the XML reader
 */
class Update_feed_xmlreader extends Update_feed {
  public function __construct($host, $server_key, $stepping = 5, $projects_count = NULL, $allowed_api_versions = array('7.x', '6.x', '5.x')) {
    parent::__construct($host, $server_key, $stepping, $projects_count, $allowed_api_versions);
  }
  /**
   * @param bool $fetch_releases
   * @param bool $save
   * @return bool
   * with the sax parser we dont support ranged queries, so we have to fetch it all in one step
   */
  public function fetch_next_projects($fetch_releases = TRUE, $save = TRUE) {
    // as we dont support fetching and not saving, we force this
    $this->fetch_and_save_all_projects($fetch_releases, TRUE);
    return TRUE;
  }

  /**
   * @param array $project_shortinfo
   * @param bool $fetch_releases
   * @param bool $save
   * @override
   * @return array
   * fetches the detailed project feeds and if needed, releases
   */
  protected function fetch_project_details($project_shortinfo, $fetch_releases = TRUE, $save = FALSE) {
    $projects_by_api = array();
    if (!array_key_exists('api_versions', $project_shortinfo)) {
      // TODO: what to do here?When a project does not have an API, we cant fetch anything 
      return array();
    }
    foreach ($project_shortinfo['api_versions'] as $api) {
      $releases = NULL;
      $url = $this->host . $this->project_data_url();
      // replace tokens
      $url = str_replace('%p', $project_shortinfo['short_name'], $url);
      $url = str_replace('%api', $api, $url);
      $url = str_replace('%type', $project_shortinfo['type'], $url);

      $reader = new XMLReader();
      $reader->open($url);
      $errors = array();
      while ($reader->read()) {
        if ($reader->nodeType == XMLReader::ELEMENT && $reader->name == 'project') {
          $project = $this->xml_handle_project($reader);
          // else just save a dummy for every project          
          $terms = $project['terms'];
          if (!empty($project['releases']) && is_array($project['releases']) && count($project['releases']) > 0) {
            $releases = $project['releases'];
          }
          unset($project['api_versions'], $project['terms'], $project['releases']);
          try {
            $project_obj = new Project($project, $terms, $this->server_key);
          }
          catch (Exception $e) {
            drupal_set_message('Could not create project ' . $infos['short_name'] . ' due to missing fields. Error: ' . $e->getMessage(), 'warning');
            // Could not create release, most probably validation failed, so fields missing1            
            //TODO: log this?
            continue;
          }
          if ($fetch_releases && $releases != NULL) {
            $this->add_releases($releases, $project_obj);
            // update full_fetched, set it to the current date
            $project_obj->set_info('full_fetched', time());
          }

          if ($save) {
            try {
              $project_obj->save();
            }
            catch (Exception $e) {
              drupal_set_message('Could not save (' . $project['short_name'] . ') due to errors (dublicate?) in the DB. ' . $e, 'error');
            }
          }
          $projects_by_api[$api] = $project_obj;
        }
      }
    }
    return $projects_by_api;
  }

  /**
   * @param array $releases assoc array of releases
   * @param Project $project_obj
   */
  protected function add_releases($releases, &$project_obj) {
    foreach ($releases as $release) {
      $terms = $release['terms'];
      unset($release['terms']);
      $project_obj->add_release($release, $terms);
    }
  }

  /**
   * @param bool $fetch_releases
   * @param bool $save
   * fetches the undetailed list of projects and stores them in the db
   */
  protected function fetch_and_save_all_projects($fetch_releases = TRUE, $save = TRUE) {
    $reader = new XMLReader();
    if($this->project_list_store == '' || empty($this->project_list_store)) {
      return;
    }
    $reader->open($this->project_list_store);
    $errors = array();
    while ($reader->read()) {
      if ($reader->nodeType == XMLReader::END_ELEMENT && $reader->name == 'projects') {
        // The end of the outer root - we are done.
        break;
      }
      if ($reader->nodeType == XMLReader::ELEMENT && $reader->name == 'project') {
        $project = $this->xml_handle_project($reader);
        if ($project == FALSE) {
          drupal_set_message('Parsing error (project tag not closed)?', 'error');
        }
        // The project has been fully fetched, so save it into the database
        if (!$this->validate_project_infos($project)) {
          $errors[] = $project['short_name'];
          // fields missing
          continue;
        }
        $project['type'] = Project::terms_to_type($project['terms']);
        // if we have to fetch the release, just use this helper function
        // it will fetch all releases for every api version of this project
        if ($fetch_releases) {
          $this->fetch_project_details($project, TRUE, $save);
          continue;
        }
        // else just save a dummy for every project
        $apis = $project['api_versions'];
        $terms = $project['terms'];
        unset($project['api_versions'], $project['terms']);
        $infos = $project;
        foreach ($apis as $api) {
          $infos['api_version'] = $api;
          $p = new Project($infos, $terms, $this->server_key);
          try {
            $p->save();
          }
          catch (Exception $e) {
            drupal_set_message('Could not save ' . $infos['short_name'] . ' into database', 'error');
          }
        }
      }
    }

    if (count($errors) > 0) {
      drupal_set_message('Not fechted projects(' . count($errors) . '), most probably no releases yet(missing fields): ' . join(', ', $errors), 'status');
    }
  }

  protected function validate_project_infos($project) {
    if (!array_key_exists('api_versions', $project) && count($project['api_versions']) == 0) {
      return FALSE;
    }

    if (!array_key_exists('terms', $project) && count($project['terms']) == 0) {
      return FALSE;
    }

    return TRUE;
  }

  /**
   * @param $reader
   * @return array|bool
   * parses a project xml entry
   */
  protected function xml_handle_project($reader) {
    $project = array();
    while ($reader->read()) {
      if ($reader->nodeType == XMLReader::END_ELEMENT && $reader->name == 'project') {
        // The project has been parsed. return it, so its get saved in the outer method
        return $project;
      }
      if ($reader->nodeType == XMLReader::ELEMENT && in_array($reader->name, array('title', 'short_name', 'link', 'project_status', 'supported_majors', 'recommended_major', 'default_major', 'api_version'))) {
        // if it is one of the default-field
        $name = $reader->name;
        $reader->read();
        $project[$name] = $reader->value;
      }
      if ($reader->nodeType == XMLReader::ELEMENT && $reader->name == 'api_versions') {
        $name = $reader->name;
        $project['api_versions'] = $this->xml_handle_api_versions($reader);
      }
      if ($reader->nodeType == XMLReader::ELEMENT && $reader->name == 'terms') {
        $project['terms'] = array();
        $project['terms'] = $this->xml_handle_terms($reader);
      }
      if ($reader->nodeType == XMLReader::ELEMENT && $reader->name == 'sourcecode') {
        $sourcecode = $this->xml_handle_sourcecode($reader);
        $project['sourcecode_uri'] = $sourcecode['uri'];
        $project['sourcecode_type'] = $sourcecode['type'];
      }
      if ($reader->nodeType == XMLReader::ELEMENT && $reader->name == 'releases') {
        $project['releases'] = array();
        $project['releases'] = $this->xml_handle_releases($reader);
      }
    }
    return FALSE;
  }

  /**
   * @param $reader
   * @return array|bool
   * parsing a SCM xml entry
   */
  protected function xml_handle_sourcecode($reader) {
    $sourcecode = array();
    while ($reader->read()) {
      if ($reader->nodeType == XMLReader::END_ELEMENT && $reader->name == 'sourcecode') {
        return $sourcecode;
      }
      if ($reader->nodeType == XMLReader::ELEMENT && in_array($reader->name, array('uri', 'type'))) {
        // if it is one of the default-field
        $name = $reader->name;
        $reader->read();
        $sourcecode[$name] = $reader->value;
      }
    }
    return FALSE;
  }

  /**
   * @param $reader
   * @return array|bool
   * parsing a list of releases
   */
  protected function xml_handle_releases($reader) {
    $releases = array();
    while ($reader->read()) {
      if ($reader->nodeType == XMLReader::END_ELEMENT && $reader->name == 'releases') {
        return $releases;
      }
      if ($reader->nodeType == XMLReader::ELEMENT && $reader->name == 'release') {
        $release = $this->xml_handle_release($reader);
        if ($release != FALSE) {
          $releases[] = $release;
        }
      }
    }
    return FALSE;
  }

  /**
   * @param $reader
   * @return array|bool
   * parsing a RELEASE xml entry
   */
  protected function xml_handle_release($reader) {
    $release = array('terms' => array());
    while ($reader->read()) {
      if ($reader->nodeType == XMLReader::END_ELEMENT && $reader->name == 'release') {
        if ($release['status'] == 'published') {
          return $release;
        }
        // else
        return FALSE;
      }
      if ($reader->nodeType == XMLReader::ELEMENT && in_array($reader->name, array('name', 'version', 'tag', 'version_major', 'version_extra', 'version_patch', 'status', 'release_link', 'download_link', 'mdhash', 'date', 'filesize'))) {
        // if it is one of the default-field
        $name = $reader->name;
        $reader->read();
        $release[$name] = $reader->value;
      }
      if ($reader->nodeType == XMLReader::ELEMENT && $reader->name == 'terms') {
        $release['terms'] = $this->xml_handle_release_terms($reader);
      }
    }
    return FALSE;
  }

  /**
   * @param $reader
   * @return array|bool
   * parse the TERM xml entry
   */
  protected function xml_handle_release_terms($reader) {
    $terms = array();
    while ($reader->read()) {
      if ($reader->nodeType == XMLReader::END_ELEMENT && $reader->name == 'terms') {
        return $terms;
      }
      if ($reader->nodeType == XMLReader::ELEMENT && $reader->name == 'term') {
        $term = $this->xml_handle_release_term($reader);
        if ($term != FALSE && $term['name'] != '') {
          $terms[$term['name']][] = $term['value'];
        }
      }
    }
    return FALSE;
  }

  /**
   * @param $reader
   * @return array|bool
   * parse the specific term informations
   */
  protected function xml_handle_release_term($reader) {
    $term = array('name' => '', 'value' => '');
    while ($reader->read()) {
      if ($reader->nodeType == XMLReader::END_ELEMENT && $reader->name == 'term') {
        return $term;
      }
      if ($reader->nodeType == XMLReader::ELEMENT && $reader->name == 'name') {
        $reader->read();
        $term['name'] = $reader->value;
      }
      if ($reader->nodeType == XMLReader::ELEMENT && $reader->name == 'value') {
        $reader->read();
        $value = $reader->value;
        $term['value'] = $value;
      }
    }
    return FALSE;
  }

  protected function xml_handle_terms($reader) {
    $terms = array();
    while ($reader->read()) {
      if ($reader->nodeType == XMLReader::END_ELEMENT && $reader->name == 'terms') {
        return $terms;
      }
      if ($reader->nodeType == XMLReader::ELEMENT && $reader->name == 'name') {
        $reader->read();
        $name = $reader->value;
        $reader->read();
      }
      if ($reader->nodeType == XMLReader::ELEMENT && $reader->name == 'value') {
        $reader->read();
        $value = $reader->value;
        $terms[$name][] = $value;
      }
    }
    return FALSE;
  }

  protected function xml_handle_api_versions($reader) {
    $apis = array();
    while ($reader->read()) {
      if ($reader->nodeType == XMLReader::END_ELEMENT && $reader->name == 'api_versions') {
        // filter all not allowed APIs
        $apis = array_intersect($this->allowed_api_versions, $apis);
        return $apis;
      }
      if ($reader->nodeType == XMLReader::ELEMENT && $reader->name == 'api_version') {
        $reader->read();
        $apis[$reader->value] = $reader->value;
      }
    }
    return FALSE;
  }
}
