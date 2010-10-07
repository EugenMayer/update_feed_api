<?php
// Our base class as a container to include the basic meta data
// and the serialization to fields for the database
abstract class FeedEntryContainer {
  protected $dest_table = NULL;
  protected $info = array();
  protected $terms = array();
  public function __construct($info, $terms) {
    // validate methods should throw exceptions, so we dont need conditions
    $this->validate_info($info);
    $this->validate_terms($terms);
    
    // validations passed, so proceed
    $this->info = $info;
    $this->terms = $terms;
  }
  
  public function get_term($key) {
    if(array_key_exists($key, $this->terms)) {
      return $this->terms[$key];
    }
    // Exception?
    throw new Exception("No such term in Release ($key)");
    return FALSE;
  }
  
  public function get_info($key) {
    if(array_key_exists($key, $this->info)) {
      return $this->info[$key];
    }
    
    throw new Exception("No such info in Release ($key)");
    return FALSE;
  }
  
  // A little function which take an array as keys
  protected function array_keys_exists($keys, $array) {
    foreach($keys as $k) {
        if(!isset($array[$k])) {
        return false;
        }
    }
    return true;
  }
  
  // A little function which take an array as keys
  public function save() {
    if($this->dest_table == NULL) {
      throw new Exception("Your implementation({get_class($this)}) of FeedEntryContainer does not set dest_table! "); 
    }
    if(!dbtng_table_exists($this->dest_table)) {
      throw new Exception("Table {$this->dest_table} does not exists,  cant save {get_class($this)}");
    }
    
    $fields = $this->info;
    
    foreach(array_keys($fields) as $field) {
        if(! dbtng_field_exists($this->dest_table, $field) ) {
          throw new Exception("Field '$field' does not exist in table '{$this->dest_table}', cant save {get_class($this)}");
        }
    }
    // finally we are free to go
    db_insert($this->dest_table)->fields($fields)->execute();
  }
  
  protected function add_db_field(&$fields) {
    $fields['terms'] = serialize($this->terms);
  }
  
  abstract protected function validate_terms($terms);
  abstract protected function validate_info($info);
}