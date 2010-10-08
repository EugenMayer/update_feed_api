<?php
// $Id$

// Copyright (c) 2010 KontextWork
// Author: Eugen Mayer

/* basic class to fetch a update feed and the release feed
 * to extend this, you most probably only want to override
 * project_list_url
 * project_data_url
 * Of course you can reimplement this and fetch the data from somewhere else, e.g. a database or a file
 */
class Update_feed {
  protected $host, $server_key;
  protected $projects_shortinfo = array();
  protected $projects = array();
  protected $stepping = 10;
  public function __construct($host, $server_key) {
    $this->host = $host;
    $this->server_key = $server_key;
  }
  
  public function fetch() {
    $this->fetch_projects();
    
    foreach($this->projects_shortinfo as $p) {
      $this->projects[] = $this->fetch_project($p, FALSE);
    }
  }
  
  public function save() {
    foreach($this->projects as $p) {
      $p->save();
    }
  }
  
  public function fetch_project_list() {
    $url = $this->host.$this->project_list_url();
    $this->list = drupal_http_request($url);
    if($this->list == FALSE) {
      throw new Exception("Could not fetch projects from $url");
    }
    // else
    $this->list = $this->list->data;
  }
  
  public function fetch_next_projects() {
    dsm('fetching');
    // if last step
    return TRUE;
  }
  
  protected function fetch_projects(){
    $doc = new SmartDOMDocument();
    $doc->loadXML($result->data);
    
    $xpath = new DOMXPath($doc);
    // well if someone wants to use CVS the does not need to be a api_version or?
    $projects = $xpath->query("//project[count(./api_versions)>0]", $doc);
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
      $this->projects_shortinfo[$project_shortinfo['short_name']] = $project_shortinfo;
    }
  }
  
  protected function fetch_project($project_shortinfo, $fetch_releases = TRUE) {
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
        drupal_set_message('Could not create project '.$infos['short_name'].' due to missing fields. Error: '.$e->getMessage());
        // Could not create release, most probably validation failed, so fields missing
        // TODO: log this?
        continue;
      }
      if($fetch_releases) {
        $this->fetch_releases($doc, $project_obj);
      }
      
      return $project_obj;
    }
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
        drupal_set_message('Could not create the release "'.$infos['name'].'"('.$infos['tag'].') of project "'.$project_obj->get_info['short_name'].'" due to missing fields. Error: '.$e->getMessage());
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