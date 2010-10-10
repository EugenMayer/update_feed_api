<?php
// $Id: filefield_field.inc,v 1.41 2010/06/30 23:52:45 quicksketch Exp $

function update_feed_cck_field_settings($op, $field) {
  switch ($op) {
    case 'database columns':
      return array(
        'title' => array('type' => 'varchar', 'length' => '255',  'default' => '', 'not null' => TRUE,  'views' => TRUE),
        'server_key' => array('type' => 'varchar', 'length' => '40',  'default' => '', 'not null' => TRUE, 'views' => TRUE),
        'short_name' => array('type' => 'varchar', 'length' => '50',  'default' => '', 'not null' => TRUE, 'views' => TRUE),
        'api_version' => array('type' => 'varchar', 'length' => '10',  'default' => '', 'not null' => TRUE, 'views' => TRUE),
        'release' => array('type' => 'varchar', 'length' => '50', 'default' => '', 'not null' => TRUE, 'views' => TRUE),
    );
  }
}

/**
 * 
 */
function update_feed_cck_content_is_empty($item, $field) {
  if (empty($item['server_key'])) {
    return TRUE;
  }
  return FALSE;
}