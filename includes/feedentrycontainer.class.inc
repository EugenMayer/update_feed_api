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
  
  public function set_info($key, $value) {
    $this->info[$key] = $value;
  }
  
  public function set_term($key, $value) {
    $this->terms[$key] = $value;
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
      throw new Exception("Your implementation(".get_class($this).") of FeedEntryContainer does not set dest_table! "); 
    }
    $fields = $this->info;
    $this->add_db_field($fields);
    $prim_key = $this->get_primary_key($fields);
    // finally we are free to go
    if($prim_key !== FALSE) {
        // primary key is set, so update
        $value = $fields[$prim_key];
        unset( $fields[$prim_key]);
        db_update($this->dest_table)->fields($fields)->condition( $prim_key, $value )->execute();
    }
    else {
      db_insert($this->dest_table)->fields($fields)->execute();
    }
  }
  
  protected function add_db_field(&$fields) {
    $fields['terms'] = serialize($this->terms);
  }
  
  abstract protected function validate_terms($terms);
  abstract protected function validate_info($info);
  abstract protected function get_primary_key(&$fields);
}