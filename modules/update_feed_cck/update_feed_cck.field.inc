<?php
// $Id$

// Copyright (c) 2010 KontextWork
// Author: Eugen Mayer

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



/**
 * Implementation of CCK's hook_field_settings($op = 'views_data').
 */
function update_feed_cck_settings_views_data($field) {
  $data = content_views_field_views_data($field);
  $db_info = content_database_info($field);
  $table_alias = content_views_tablename($field);

  // By defining the relationship, we already have a "Has file" filter
  // plus all the filters that Views already provides for files.
  // No need for having a filter by ourselves.
  unset($data[$table_alias][$field['field_name'] .'_fid']['filter']);

  // Add a relationship for related file.
  $data[$table_alias][$field['field_name'] .'_fid']['relationship'] = array(
    'base' => 'files',
    'field' => $db_info['columns']['fid']['column'],
    'handler' => 'content_handler_relationship',
    'label' => t($field['widget']['label']),
    'content_field_name' => $field['field_name'],
    'skip base' => array('files'),
  );

  // Use the Views boolean handler for filtering the list value.
  $data[$table_alias][$field['field_name'] .'_list']['filter']['handler'] = 'views_handler_filter_boolean_operator';

  // Add our special handler for serialized data.
  $data[$table_alias][$field['field_name'] .'_data']['field'] = array(
    'title' => $data[$table_alias][$field['field_name'] .'_data']['title'],
    'title short' => $data[$table_alias][$field['field_name'] .'_data']['title short'],
    'field' => $db_info['columns']['data']['column'],
    'table' => $db_info['table'],
    'handler' => 'filefield_handler_field_data',
    'click sortable' => FALSE,
    'access callback' => 'content_access',
    'access arguments' => array('view', $field),
  );

  // Remove handlers that are probably unnecessary.
  unset($data[$table_alias][$field['field_name'] .'_data']['filter']);
  unset($data[$table_alias][$field['field_name'] .'_data']['argument']);
  unset($data[$table_alias][$field['field_name'] .'_list']['argument']);

  // Set up relationship with file views.
  $data[$table_alias]['table']['join']['files'] = array(
    'table' => $db_info['table'],
    'left_field' => 'fid',
    'field' => $db_info['columns']['fid']['column'],
  );
  $data[$table_alias]['vid'] = array(
    'title' => t($field['widget']['label']),
    'help' => t('The node the uploaded file is attached to'),
    'relationship' => array(
      'label' => t($field['widget']['label']),
      'base' => 'node',
      'base field' => 'vid',
      // This allows us to not show this relationship if the base is already
      // node so users won't create circular relationships.
      'skip base' => array('node', 'node_revisions'),
    ),
  );

  return $data;
}
