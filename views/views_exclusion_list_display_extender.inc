<?php

/**
 * @file
 * Definition of views_exclusion_list_display_extender class.
 */

class views_exclusion_list_display_extender extends views_plugin_display_extender {

  /**
   * {@inheritdoc}
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['exclusion_list_ignore'] = array('bool' => TRUE, 'default' => FALSE);
    return $options;
  }

  /**
   * {@inheritdoc}
   */
  function options_definition_alter(&$options) {
    $options['exclusion_list_ignore'] = array('bool' => TRUE, 'default' => FALSE);
  }

  /**
   * {@inheritdoc}
   */
  function options_summary(&$categories, &$options) {
    $ignored = $this->display->get_option('exclusion_list_ignore');
    $options['exclusion_list'] = array(
      'category' => 'other',
      'title' => t('Exclusion list'),
      'value' => $ignored ? t('Ignored') : t('Contributing results'),
    );
  }

  /**
   * {@inheritdoc}
   */
  function options_form(&$form, &$form_state) {
    if ($form_state['section'] == 'exclusion_list') {
      $form['#title'] .= t('Exclusion list setting');
      $form['exclusion_list_ignore'] = array(
        '#type' => 'checkbox',
        '#title' => 'Ignored by the exclusion list',
        '#description' => t('If checked, results of this view will not be added to the exclusion list, even if this view is selected in the filter section of another view.'),
        '#default_value' => $this->display->get_option('exclusion_list_ignore'),
      );
    }
  }

  /**
   * {@inheritdoc}
   */
  function options_submit(&$form, &$form_state) {
    if ($form_state['section'] == 'exclusion_list') {
      $ignore = $form_state['values']['exclusion_list_ignore'];
      $this->display->set_option('exclusion_list_ignore', $ignore);
    }
  }

}
