<?php

/**
 * @file
 * Administrative functions for the Views Exclusion List module.
 */

/**
 * Settings page form constructor.
 */
function views_exclusion_list_settings_form($form, &$form_state) {
  $options = array();
  foreach (entity_get_info() as $entity_type => $entity_info) {
    $options[$entity_type] = $entity_info['label'];
  }

  $form['views_exclusion_list_enabled_entities'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Enabled entities'),
    '#description' => t('Select the entitites that will be able to use Views Exclusion List.'),
    '#options' => $options,
    '#default_value' => variable_get('views_exclusion_list_enabled_entities'),
    '#required' => TRUE,
  );

  // Will clear views caches.
  $form['#submit'][] = 'views_exclusion_list_settings_form_submit';

  return system_settings_form($form);
}

/**
 * Submit callback for views_exclusion_list_settings_form().
 */
function views_exclusion_list_settings_form_submit($form, &$form_state) {
  cache_clear_all('*', 'cache_views', TRUE);
  cache_clear_all('*', 'cache_views_data', TRUE);
}
