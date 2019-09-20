<?php

/**
 * @file
 * Views functions for views exclude previous.
 */

/**
 * Implements hook_views_data_alter()
 */
function views_exclusion_list_views_data_alter(&$data) {
  foreach (views_exclusion_list_enabled_entities() as $entity_type => $info) {
    if (isset($data[$info['table']])) {
      $data[$info['table']]['exclusion_list'] = array(
        'group' => t('Global'),
        'title' => t('Exclusion List'),
        'help' => t('Excludes results that have already been loaded by another view in the current page.'),
        'real field' => $info['field'],
        'filter' => array(
          'handler' => 'views_exclusion_list_filter',
        ),
      );
    }
  }
}

/**
 * Implements hook_views_plugins().
 */
function views_exclusion_list_views_plugins() {
  $path = drupal_get_path('module', 'views_exclusion_list') . '/views';
  $plugins = array();
  $plugins['display_extender']['exclusion_list'] = array(
    'title' => t('Exclusion list'),
    'help' => t('Manages exclusion list settings for this view.'),
    'handler' => 'views_exclusion_list_display_extender',
    'path' => $path,
    'enabled' => TRUE,
  );

  return $plugins;
}
