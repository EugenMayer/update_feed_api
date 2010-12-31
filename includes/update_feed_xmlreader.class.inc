<?php
/*
 * Parses feeds using the XML reader
 */
class Update_feed_xmlreader extends Update_feed {
  public function __construct($host, $server_key, $stepping = 5, $projects_count = NULL, $allowed_api_versions = array('7.x', '6.x', '5.x')) {
    parent::__construct($host, $server_key, $stepping, $projects_count, $allowed_api_versions);
  }
  
  // with the sax parser we dont support ranged queries, so we have to fetch it all in one step
  public function fetch_next_projects($fetch_releases = TRUE, $save = TRUE) {
    // as we dont support fetching and not saving, we force this
    $this->fetch_and_save_all_projects($fetch_releases, TRUE);
    return TRUE;
  }  
  
  protected function fetch_and_save_all_projects($fetch_releases = TRUE, $save = TRUE){
    $reader = new XMLReader();
    $reader->open($this->project_list_store);
    $errors = array();
    while ($reader->read()) {
      // When we close a project tag, add it to the database.
      if ($reader->nodeType == XMLReader::END_ELEMENT && $reader->name == 'projects') {
        break;
      }
      if ($reader->nodeType == XMLReader::ELEMENT && $reader->name == 'project') {
        // If it isn't a type we know how to process, then skip it.
        $project = $this->xml_handle_project($reader);
      }
      if ($reader->nodeType == XMLReader::END_ELEMENT && $reader->name == 'project') {
        if(!$this->validate_project_infos($project)) {
          $errors[] = $project['short_name'];
          // fields missing
          continue;
        }
        // if we have to fetch the release, just use this helper function
        // it will fetch all releases for every api version of this project
        if($fetch_releases) {
          $this->fetch_project_details($project, TRUE, $save);
          continue;
        }
        // else just save a dummy for every project
        $apis = $project['api_versions'];
        $terms = $project['terms'];
        unset($project['api_versions'], $project['terms']);
        $infos = $project;
        foreach($apis as $api) {
          $infos['api_version'] = $api;
          $p = new Project($infos, $terms, $this->server_key);
          try {
            $p->save();
          }
          catch(Exception $e) {
            drupal_set_message('Could not save '.$p->get_info('short_name').' into database', 'error');
          }
        }
        continue;
      }
    }
    
    if(count($errors) > 0) {
      drupal_set_message('Not fechted projects('.count($errors).'), most probably no releases yet(missing fields): '.join(', ',$errors),'status');
    }
  }
  
  protected function validate_project_infos($project) { 
    if(!array_key_exists('api_versions', $project) && count($project['api_versions']) == 0) {
      return FALSE;
    }
    
    if(!array_key_exists('terms', $project) && count($project['terms']) == 0) {
      return FALSE;
    }
    
    return TRUE;
  }
  
  protected function xml_handle_project($reader) {
    $project = array();
    while ($reader->read()) {
      // When we close a project tag, add it to the database.
      if ($reader->nodeType == XMLReader::END_ELEMENT && $reader->name == 'project') {
        return $project;
      }
      
      if ($reader->nodeType == XMLReader::ELEMENT && in_array($reader->name, array('title', 'short_name', 'link','project_status'))) {
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
    }
    return $project;
  }
  
  protected function xml_handle_terms($reader) {
    $terms = array();
    while ($reader->read()) {
      // When we close a project tag, add it to the database.
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
    return $terms;
  }
  
  protected function xml_handle_api_versions($reader) {
    $apis = array();
    while ($reader->read()) {
      // When we close a project tag, add it to the database.
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
    return $apis;
  }
}
