<?php
// $Id$

// Copyright (c) 2010 KontextWork
// Author: Eugen Mayer

interface IUpdate_feed {
  public function __construct($host, $server_key, $stepping = 5, $projects_count = NULL, $allowed_api_versions = array('7.x', '6.x', '5.x'));
  // should fetch the whole project list. That what fetch_next_projects operates on
  // This should also initialize the count of the projects overall
  public function fetch_project_list();
  // fetches the next set of projects. Should return TRUE if finished
  // or an integer for the turns needed to finish. Mostly needed for batch runs 
  public function fetch_next_projects($fetch_releases = TRUE, $save = TRUE);
  // use this to fetch project informations. This is used for fetching releases after lazy fetching the project list
  // mostly this is done on demand to reduce load
  public function fetch_project($projects);
  public function overall_project_count();
  public function allready_fetched_count();
  public function host();
  public function server_key();
}

/* basic class to fetch a update feed and the release feed
 * to extend this, you most probably only want to override
 * project_list_url
 * project_data_url
 * Of course you can reimplement this and fetch the data from somewhere else, e.g. a database or a file
 */
class Update_feed implements IUpdate_feed{
  protected $host, $server_key, $project_list_store;  
  protected $stepping;
  protected $cur_pos = 0;
  protected $projects_count = NULL;
  protected $allowed_api_versions = array();
  
  public function __construct($host, $server_key, $stepping = 5, $projects_count = NULL, $allowed_api_versions = array('7.x', '6.x', '5.x')) {
    $this->host = $host;
    $this->server_key = $server_key;
    $this->cur_pos = 0;
    $this->stepping = $stepping;
    $this->projects_count = $projects_count;
    $this->allowed_api_versions = $allowed_api_versions;
  }
  
  public function fetch_next_projects($fetch_releases = TRUE, $save = TRUE) {
    if($this->cur_pos < $this->projects_count) {
      // define the border, exclusivly
      $to = $this->cur_pos + $this->stepping;
      
      if($to > $this->projects_count) {
        $to = $this->projects_count;
      }
     
      // finally fetch the projects shortinfos
      $projects_shortinfo = $this->fetch_projects($this->cur_pos,  $to);
      // no get all detailed informations, project by project and save them in the DB
      foreach($projects_shortinfo as $p) {
        // we dont fetch releases yet, just for debuuging
        // the result is an array of projects for this project, as they are seperated by the API key
        if($fetch_releases) {
          $projects = $this->fetch_project_details($p, $fetch_releases, $save);
        } 
        else {
          // lazy fetching
          $apis = $p['api_versions'];
          $terms = $p['terms'];
          unset($p['api_versions'], $p['terms']);
          $infos = $p;
          foreach($apis as $api) {
            $infos['api_version'] = $api;
            $project = new Project($infos, $terms, $this->server_key);
            try {
              $project->save();
            }
            catch(Exception $e) {
              drupal_set_message('Could not save '.$project->get_info('short_name').' into database','error');
            }            
          }
        }
      }
      // we are done, set the new posion and return how many steps we will need to finish all projects
      $this->cur_pos = $to;
      $status = ceil( ($this->projects_count - $this->cur_pos) / $this->stepping ) + 1;
      return $status;
    }

    // if last step
    return TRUE;
  }
  
  public function fetch_project_list() {
    $url = $this->host.$this->project_list_url();
    $result = drupal_http_request($url);
    if($result == FALSE) {
      throw new Exception("Could not fetch projects from $url");
    }
    
    $tmp = file_directory_temp();
    $filename = uniqid().'.xml';
    $path = "$tmp/$filename";
    file_put_contents($path,$result->data);
    $this->project_list_store = $path;
    // count all projects once        
    $doc = new SmartDOMDocument();
    $doc->loadXML($result->data);
    $xpath = new DOMXPath($doc);
    $projects = $xpath->query("//project", $doc);
    
    // if projects_count was set before, its a hard limit init. by the contructor
    // so we dont touch it
    if($this->projects_count == NULL || !is_integer($this->projects_count)) {
      $this->projects_count = $projects->length;
    }
  }
  
  
  
  public function fetch_project($project, $api_versions = NULL) {
    if($api_versions == NULL)  {
      $api_versions = array( $project->get_info('api_version') );
    }
    $project_info = array(
      'short_name' => $project->get_info('short_name'),
      'api_versions' => $api_versions,
    );
    
    // remove the old project from the DB, as we will fetch all details now newly and create 
    // a new recordd for each api related to the releases;
    $project->remove();
    $this->fetch_project_details($project_info, TRUE, TRUE);
  }
  
  protected function fetch_projects($from, $to){
    // We need to read the file each time, as it seem not to be persisten due a session in batch run
    // so we save it in the temporary file
    $doc = new SmartDOMDocument();
    $xml = file_get_contents($this->project_list_store);
    $doc->loadXML($xml);
    $xpath = new DOMXPath($doc);
    // we dont include the right border, as we will start with that element the next time
    $projects = $xpath->query("//project[position() >= $from and position() <= $to]", $doc);
    $result = array();
    foreach($projects as $p) {
      $project_shortinfo = array();
      $project_shortinfo['short_name'] = $xpath->query("./short_name", $p)->item(0)->nodeValue;
      $project_shortinfo['title'] = $xpath->query("./title", $p)->item(0)->nodeValue;
      // do we need the link?
      $project_shortinfo['link'] = $xpath->query("./link", $p)->item(0)->nodeValue;
      $terms = array();
      $ts = $xpath->query("//terms/term", $doc);
      foreach($ts as $term) {
        $name = $xpath->query("./name", $term)->item(0)->nodeValue;
        $value = $xpath->query("./value", $term)->item(0)->nodeValue;
        if(array_key_exists($name,$terms)) {
          if(is_string($terms[$name])) {
            $terms[$name] = array($terms[$name], $value);
          }
          else {
            $terms[$name][] = $value;
          }
        }
        else {
          $terms[$name] = $value;
        }
      }
      $project_shortinfo['terms'] = $terms;
      $apis = $xpath->query("./api_versions/api_version", $p);
      foreach($apis as $api) {
        $project_shortinfo['api_versions'][$api->nodeValue] = $api->nodeValue;
      }
      
      // if there are not apis, dont add that project
      if(!is_array($project_shortinfo['api_versions'])) {
        continue;
      }
      // filter all not allowed APIs
      $project_shortinfo['api_versions'] = array_intersect($this->allowed_api_versions, $project_shortinfo['api_versions']);
      
      // if no apis left, dont add the project
      if(count($project_shortinfo['api_versions']) == 0) {
        continue;
      }
      // on one sever, the short_name is unique
      $result[$project_shortinfo['short_name']] = $project_shortinfo;
    }
    return $result;
  }
  
  // be aware, returns an array. Each item is one of the project with a specfici API key
  protected function fetch_project_details($project_shortinfo, $fetch_releases = TRUE, $save = FALSE) {
    $projects_by_api = array();
    if(!array_key_exists('api_versions',$project_shortinfo)) {
      // TODO: what to do here?When a project does not have an API, we cant fetch anything 
      return array();
    }
    foreach($project_shortinfo['api_versions'] as $api) {
      $url = $this->host.$this->project_data_url();
      // replace tokens
      $url =  str_replace('%p', $project_shortinfo['short_name'], $url);
      $url =  str_replace('%api', $api, $url);
      
      $result = drupal_http_request($url);
      if($result == FALSE) {
        // could mean there is nothing there yet, no need to freak out
        // maybe write some logs?
        //throw new Exception("Could not fetch projects from $url");
      }
      $doc = new SmartDOMDocument();
      $doc->loadXML($result->data);
      
      $xpath = new DOMXPath($doc);
      $infos = array();
      
      $infos['title'] = $xpath->query("//title", $doc)->item(0)->nodeValue;
      if($infos['title'] == NULL) {
        continue;
      }
      
      $infos['short_name'] = $xpath->query("//short_name", $doc)->item(0)->nodeValue;
      $infos['api_version'] = $xpath->query("//api_version", $doc)->item(0)->nodeValue;
      $infos['recommended_major'] = $xpath->query("//recommended_major", $doc)->item(0)->nodeValue;
      $infos['supported_majors'] = $xpath->query("//supported_majors", $doc)->item(0)->nodeValue;
      $infos['default_major'] = $xpath->query("//default_major", $doc)->item(0)->nodeValue;
      $infos['project_status'] = $xpath->query("//project_status", $doc)->item(0)->nodeValue;
      $infos['link'] = $xpath->query("//link", $doc)->item(0)->nodeValue;
     
      $terms = array();
      $ts = $xpath->query("//terms/term", $doc);
      foreach($ts as $term) {
        $name = $xpath->query("./name", $term)->item(0)->nodeValue;
        $value = $xpath->query("./value", $term)->item(0)->nodeValue;
        
        if(array_key_exists($name,$terms)) {
          if(is_string($terms[$name])) {
            $terms[$name] = array($terms[$name], $value);
          }
          else {
            $terms[$name][] = $value;
          }
        }
        else {
          $terms[$name] = $value;
        }
      }
      try {
        $project_obj = new Project($infos, $terms, $this->server_key);
      }
      catch (Exception $e) {
        drupal_set_message('Could not create project '.$infos['short_name'].' due to missing fields. Error: '.$e->getMessage(), 'warning');
        // Could not create release, most probably validation failed, so fields missing
        // TODO: log this?
        continue;
      }
      if($fetch_releases) {
        $this->fetch_releases($doc, $project_obj);
        // update full_fetched, set it to the current date
        $project_obj->set_info('full_fetched', time());
      }
      
      if($save) {
        try {
          $project_obj->save();
        }
        catch(Exception $e) {
          drupal_set_message('Could not save ('.$project_obj->get_info('short_name').') due to errors (dublicate?) in the DB. '.$e, 'error');
        }
      }
      
      $projects_by_api[$api] = $project_obj;
    }
    return $projects_by_api;
  }
  
  protected function fetch_releases($doc, &$project_obj) {
    $xpath = new DOMXPath($doc);
    $releases = $xpath->query("//releases/release", $doc);
    foreach($releases as $release) {
      $infos = array();
      $infos['name'] = $xpath->query("./name", $release)->item(0)->nodeValue;
      $infos['version'] = $xpath->query("./version", $release)->item(0)->nodeValue;
      $infos['tag'] = $xpath->query("./tag", $release)->item(0)->nodeValue;
      $infos['version_major'] = $xpath->query("./version_major", $release)->item(0)->nodeValue;
      $infos['version_extra'] = $xpath->query("./version_extra", $release)->item(0)->nodeValue;
      $infos['status'] = $xpath->query("./status", $release)->item(0)->nodeValue;
      $infos['status'] = $xpath->query("./status", $release)->item(0)->nodeValue;
      $infos['status'] = $xpath->query("./status", $release)->item(0)->nodeValue;
      $infos['release_link'] = $xpath->query("./release_link", $release)->item(0)->nodeValue;
      $infos['download_link'] = $xpath->query("./download_link", $release)->item(0)->nodeValue;
      $infos['mdhash'] = $xpath->query("./mdhash", $release)->item(0)->nodeValue;
      $infos['date'] = $xpath->query("./date", $release)->item(0)->nodeValue;
      $infos['filesize'] = $xpath->query("./filesize", $release)->item(0)->nodeValue;
      
      $terms = array();
      $ts = $xpath->query("//terms/term", $doc);
      foreach($ts as $term) {
        $name = $xpath->query("./name", $term)->item(0)->nodeValue;
        $value = $xpath->query("./value", $term)->item(0)->nodeValue;
        
        $terms[$name] = $value;
      }
      try {
        $project_obj->add_release($infos, $terms);
      }
      catch(Exception $e) {
        drupal_set_message('Could not create the release "'.$infos['name'].'"('.$infos['tag'].') of project "'.$project_obj->get_info('short_name').'" due to missing fields. Error: '.$e->getMessage(), 'warning');
        // Could not create release, most probably validation failed, so fields missing
        // TODO: log this?
        continue;
      }
    }
  }
  
  protected function project_list_url() {
    return '/project-list/all';
  }
  
  protected function project_data_url() {
    return '/%p/%api';
  }
  
  public function overall_project_count() {
    return $this->projects_count;
  }
  
  public function allready_fetched_count() {
    return $this->cur_pos;
  }

  public function host() {
      return $this->host;
  }
  
  public function server_key() {
    return $this->server_key;
  }
  
  public function __destruct() {
    unset($this->project_list_store);
  }
}