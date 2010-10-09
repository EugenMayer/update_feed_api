<?php
// $Id$

// Copyright (c) 2010 KontextWork
// Author: Eugen Mayer

interface IUpdate_feed {
  public function __construct($host, $server_key, $stepping = 5, $projects_count = NULL);
  // should fetch the whole project list. That what fetch_next_projects operates on
  // This should also initialize the count of the projects overall
  public function fetch_project_list();
  // fetches the next set of projects. Should return TRUE if finished
  // or an integer for the turns needed to finish. Mostly needed for batch runs 
  public function fetch_next_projects();
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
  
  public function __construct($host, $server_key, $stepping = 5, $projects_count = NULL) {
    $this->host = $host;
    $this->server_key = $server_key;
    $this->cur_pos = 0;
    $this->stepping = $stepping;
    $this->projects_count = $projects_count;
  }

  public function fetch_project_list() {
    $url = $this->host.$this->project_list_url();
    $result = drupal_http_request($url);
    if($result == FALSE) {
      throw new Exception("Could not fetch projects from $url");
    }
    
    $tmp = file_directory_temp();
    $filename = uniqid().'xml';
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
  
  public function fetch_next_projects() {
    if($this->cur_pos < $this->projects_count) {
      // define the border, exclusivly
      $to = $this->cur_pos + $this->stepping;
      // finally fetch the projects shortinfos
      $projects_shortinfo = $this->fetch_projects($this->cur_pos,  $to);
      // no get all detailed informations, project by project and save them in the DB
      foreach($projects_shortinfo as $p) {
        // we dont fetch releases yet, just for debuuging
        // the result is an array of projects for this project, as they are seperated by the API key
        $projects = $this->fetch_project_details($p, TRUE, TRUE);
      }
      // we are done, set the new posion and return how many steps we will need to finish all projects
      $this->cur_pos = $to;
      $status = ceil( ($this->projects_count - $this->cur_pos) / $this->stepping ) + 1;
      return $status;
    }

    // if last step
    return TRUE;
  }
  
  protected function fetch_projects($from, $to){
    // We need to read the file each time, as it seem not to be persisten due a session in batch run
    // so we save it in the temporary file
    $doc = new SmartDOMDocument();
    $xml = file_get_contents($this->project_list_store);
    $doc->loadXML($xml);
    $xpath = new DOMXPath($doc);
    // we dont include the right border, as we will start with that element the next time
    $projects = $xpath->query("//project[position() >= $from and position() < $to]", $doc);
    $result = array();
    foreach($projects as $p) {
      $project_shortinfo = array();
      $project_shortinfo['short_name'] = $xpath->query("./short_name", $p)->item(0)->nodeValue;
      $project_shortinfo['title'] = $xpath->query("./title", $p)->item(0)->nodeValue;
      // do we need the link?
      $project_shortinfo['link'] = $xpath->query("./link", $p)->item(0)->nodeValue;
      $apis = $xpath->query("./api_versions/api_version", $p);#
      foreach($apis as $api) {
        $project_shortinfo['apis'][] = $api->nodeValue;
      }
      // on one sever, the short_name is unique
      $result[$project_shortinfo['short_name']] = $project_shortinfo;
    }
    return $result;
  }
  
  // be ware, returns an array. Each item is one of the project with a specfici API key
  protected function fetch_project_details($project_shortinfo, $fetch_releases = TRUE, $save = FALSE) {
    $projects_by_api = array();
    if(!array_key_exists('apis',$project_shortinfo)) {
      // TODO: what to do here?When a project does not have an API, we cant fetch anything 
      return array();
    }
    foreach($project_shortinfo['apis'] as $api) {
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
        
        $terms[$name] = $value;
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
      }
      if($save) {
        $project_obj->save();
      }
      $projects_by_api[] = $project_obj;
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
    return '/release-history/project-list/all';
  }
  
  protected function project_data_url() {
    return '/release-history/%p/%api';
  }
}