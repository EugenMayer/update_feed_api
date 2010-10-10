<?php
// $Id$

/**
 * Implementation of CCK's hook_widget_settings($op == 'form').
 */
function update_feed_cck_widget_settings_form($widget) {
  $form = array();

  // Convert the extensions list to be a human-friendly comma-separated list.
  $extensions = is_string($widget['file_extensions']) ? $widget['file_extensions'] : 'txt';
  $form['file_extensions'] = array(
    '#type' => 'textfield',
    '#title' => t('Permitted upload file extensions'),
    '#default_value' => $extensions,
    '#size' => 64,
    '#maxlength' => 512,
    '#description' => t('Extensions a user can upload to this field. Separate extensions with a space and do not include the leading dot. Leaving this blank will allow users to upload a file with any extension.'),
    '#element_validate' => array('_filefield_widget_settings_extensions_validate'),
    '#pre_render' => array('_filefield_widget_settings_extensions_value'),
    '#weight' => 1,
  );

  $form['progress_indicator'] = array(
    '#type' => 'radios',
    '#title' => t('Progress indicator'),
    '#options' => array(
      'bar' => t('Bar with progress meter'),
      'throbber' => t('Throbber'),
    ),
    '#default_value' => empty($widget['progress_indicator']) ? 'bar' : $widget['progress_indicator'],
    '#description' => t('Your server supports upload progress capabilities. The "throbber" display does not indicate progress but takes up less room on the form, you may want to use it if you\'ll only be uploading small files or if experiencing problems with the progress bar.'),
    '#weight' => 5,
    '#access' => filefield_progress_implementation(),
  );

  $form['path_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Path settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#weight' => 6,
  );
  $form['path_settings']['file_path'] = array(
    '#type' => 'textfield',
    '#title' => t('File path'),
    '#default_value' => is_string($widget['file_path']) ? $widget['file_path'] : '',
    '#description' => t('Optional subdirectory within the "%directory" directory where files will be stored. Do not include preceding or trailing slashes.', array('%directory' => variable_get('file_directory_path', 'files') . '/')),
    '#element_validate' => array('_filefield_widget_settings_file_path_validate'),
    '#suffix' => theme('token_help', 'user'),
  );

  $form['max_filesize'] = array(
    '#type' => 'fieldset',
    '#title' => t('File size restrictions'),
    '#description' => t('Limits for the size of files that a user can upload. Note that these settings only apply to newly uploaded files, whereas existing files are not affected.'),
    '#weight' => 6,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['max_filesize']['max_filesize_per_file'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum upload size per file'),
    '#default_value' => is_string($widget['max_filesize_per_file'])
                        ? $widget['max_filesize_per_file']
                         : '',
    '#description' => t('Specify the size limit that applies to each file separately. Enter a value like "512" (bytes), "80K" (kilobytes) or "50M" (megabytes) in order to restrict the allowed file size. If you leave this empty the file sizes will be limited only by PHP\'s maximum post and file upload sizes (current limit <strong>%limit</strong>).', array('%limit' => format_size(file_upload_max_size()))),
    '#element_validate' => array('_filefield_widget_settings_max_filesize_per_file_validate'),
  );
  $form['max_filesize']['max_filesize_per_node'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum upload size per node'),
    '#default_value' => is_string($widget['max_filesize_per_node'])
                        ? $widget['max_filesize_per_node']
                        : '',
    '#description' => t('Specify the total size limit for all files in field on a given node. Enter a value like "512" (bytes), "80K" (kilobytes) or "50M" (megabytes) in order to restrict the total size of a node. Leave this empty if there should be no size restriction.'),
    '#element_validate' => array('_filefield_widget_settings_max_filesize_per_node_validate'),
  );

  return $form;
}

/**
 * Implementation of CCK's hook_widget_settings($op == 'save').
 */
function update_feed_cck_widget_settings_save($widget) {
  return array('file_extensions', 'file_path', 'progress_indicator', 'max_filesize_per_file', 'max_filesize_per_node');
}


/**
 * The #value_callback for the filefield_widget type element.
 */
function update_feed_cck_widget_value($element, $edit = FALSE) {
  if (!$edit) {
    $file = field_file_load($element['#default_value']['fid']);
    $item = $element['#default_value'];
  }
  else {
    $item = $edit;
    $field = content_fields($element['#field_name'], $element['#type_name']);

    // Uploads take priority over value of fid text field.
    if ($fid = filefield_save_upload($element)) {
      $item['fid'] = $fid;
    }
    // Check for #filefield_value_callback values.
    // Because FAPI does not allow multiple #value_callback values like it does
    // for #element_validate and #process, this fills the missing functionality
    // to allow FileField to be extended purely through FAPI.
    elseif (isset($element['#filefield_value_callback'])) {
      foreach ($element['#filefield_value_callback'] as $callback) {
        $callback($element, $item);
      }
    }

    // Load file if the FID has changed so that it can be saved by CCK.
    $file = field_file_load($item['fid']);

    // If the file entry doesn't exist, don't save anything.
    if (empty($file)) {
      $item = array();
    }

    // Checkboxes loose their value when empty.
    // If the list field is present make sure its unchecked value is saved.
    if (!empty($field['list_field']) && empty($edit['list'])) {
      $item['list'] = 0;
    }
  }
  // Merge file and item data so it is available to all widgets.
  if (isset($item['data']) && isset($file['data'])) {
    $file['data'] = array_merge($item['data'], $file['data']);
  }
  $item = array_merge($item, $file);

  return $item;
}

/**
 * An element #process callback for the filefield_widget field type.
 *
 * Expands the filefield_widget type to include the upload field, upload and
 * remove buttons, and the description field.
 */
function update_feed_cck_widget_process($element, $edit, &$form_state, $form) {
 
}

/**
 * An #element_validate callback for the filefield_widget field.
 */
function update_feed_cck_widget_validate(&$element, &$form_state) {
  // If referencing an existing file, only allow if there are existing
  // references. This prevents unmanaged files (outside of FileField) from
  // being deleted if this node were to be deleted.
  if (!empty($element['fid']['#value'])) {
    $field = content_fields($element['#field_name'], $element['#type_name']);
    if ($file = field_file_load($element['fid']['#value'])) {
      $file = (object) $file;
      if ($file->status == FILE_STATUS_PERMANENT) {
        if (field_file_references($file) == 0) {
          form_error($element, t('Referencing to the file used in the %field field is not allowed.', array('%field' => $element['#title'])));
        }
      }
    }
    else {
      form_error($element, t('The file referenced by the %field field does not exist.', array('%field' => $element['#title'])));
    }
  }
}

/**
 * FormAPI theme function. Theme the output of an image field.
 */
function theme_update_feed_cck_widget($element) {
  $element['#id'] .= '-upload'; // Link the label to the upload field.
  return theme('form_element', $element, $element['#children']);
}
