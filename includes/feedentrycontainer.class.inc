<?php
// $Id$

// Copyright (c) 2010 KontextWork
// Author: Eugen Mayer

// Our base class as a container to include the basic meta data
// and the serialization to fields for the database
abstract class FeedEntryContainer {
  protected $dest_table = NULL;
  protected $prim_key_field = NULL;
  protected $infos = array();
  protected $terms = array();

  public function __construct($infos, $terms) {
    // validate methods should throw exceptions, so we dont need conditions
    $this->validate_infos($infos);
    $this->validate_terms($terms);

    // validations passed, so proceed
    $this->infos = $infos;
    $this->terms = $terms;
  }


  public function get_terms() {
    return $this->terms;
  }

  public function get_term($key) {
    if (array_key_exists($key, $this->terms)) {
      return $this->terms[$key];
    }
    // Exception?
    throw new Exception("No such term in Release ($key)");
    return FALSE;
  }

  public function get_info($key) {
    if (array_key_exists($key, $this->infos)) {
      return $this->infos[$key];
    }

    throw new Exception("No such info in Release ($key)");
    return FALSE;
  }

  public function set_info($key, $value) {
    $this->infos[$key] = $value;
  }

  public function set_term($key, $value) {
    $this->terms[$key] = $value;
  }

  // A little function which take an array as keys
  protected function array_keys_exists($keys, $array) {
    foreach ($keys as $k) {
      if (!isset($array[$k])) {
        return false;
      }
    }
    return true;
  }

  // A little function which take an array as keys
  public function save() {
    if ($this->dest_table == NULL) {
      throw new Exception("Your implementation(" . get_class($this) . ") of FeedEntryContainer does not set dest_table! ");
    }

    $fields = $this->infos;
    // Let others add fields
    $this->alter_db_fields($fields);
    $prim_key = $this->primary_key_exists($fields);
    // finally we are free to go
    if ($prim_key !== FALSE) {
      // primary key is set, so update
      $value = $fields[$prim_key];
      unset($fields[$prim_key]);
      db_update($this->dest_table)->fields($fields)->condition($prim_key, $value)->execute();
    }
    else {
      $key_value = db_insert($this->dest_table)->fields($fields)->execute();
      $this->infos[$this->prim_key_field] = $key_value;
    }
  }

  protected function alter_db_fields(&$fields) {
    $fields['terms'] = serialize($this->terms);
  }

  protected function primary_key_exists(&$fields) {
    if ($this->prim_key_field == NULL) {
      throw new Exception("Your implementation(" . get_class($this) . ") of FeedEntryContainer does not set the primary key! ");
    }

    if (array_key_exists($this->prim_key_field, $fields)) {
      return $this->prim_key_field;
    }
    // else
    return FALSE;
  }

  abstract protected function validate_terms($terms);

  abstract protected function validate_infos($info);
}
