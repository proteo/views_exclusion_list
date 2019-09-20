<?php

/**
 * @file
 * Definition of views_exclusion_list_filter class.
 */

/**
 * Class views_exclusion_list_handler_filter.
 */
class views_exclusion_list_filter extends views_handler_filter {

  /**
   * Stores the exclusion list.
   *
   * @var array
   */
  public $exclusion_list = array();

  /**
   * Disable the possibility to use operators.
   *
   * @var bool
   */
  public $no_operator = TRUE;

  /**
   * {@inheritdoc}
   */
  public function init(&$view, &$options) {
    views_handler::init($view, $options);

    $this->value = $this->options['value'];

    // If there are relationships in the view, allow empty should be true
    // so that we can do IS NULL checks on items. Not all filters respect
    // allow empty, but string and numeric do and that covers enough.
    if ($this->view->display_handler->get_option('relationships')) {
      $this->definition['allow empty'] = TRUE;
    }
  }

  /**
   * {@inheritdoc}
   */
  public function admin_summary() {
    if (isset($this->value['all'])) {
      $summary = t('using all views');
    }
    elseif (count($this->value) <= 3) {
      $views = array();
      foreach ($this->value as $view_id) {
        list ($name, $display) = explode(':', $view_id);
        $view = views_get_view($name);
        $views[] = $view->human_name . ' - ' . $view->display[$display]->display_title;
      }
      $summary = t('using @views', array('@views' => implode(', ', $views)));
    }
    else {
      $summary = t('using @count views', array('@count' => count($this->value)));
    }
    return $summary;
  }

  /**
   * {@inheritdoc}
   */
  public function can_expose() {
    return FALSE;
  }

  /**
   * {@inheritdoc}
   */
  public function option_definition() {
    $options = parent::option_definition();
    $options['value']['default'] = array();
    $options['force_load']['default'] = FALSE;

    return $options;
  }

  /**
   * Return a list of all available views.
   */
  public function get_value_options() {
    if (!isset($this->value_options)) {
      // Create a list of all the view:display combos.
      $views = views_get_all_views();
      $options = array('all' => t('- Any -'));
      foreach ($views as $view) {
        // Only use views that output entities that might be in this view.
        if ($view->base_table == $this->table) {
          foreach ($view->display as $display) {
            // Don't let users filter views display results recursively.
            if ($this->view->name != $view->name || $this->view->current_display != $display->id) {
              $options["{$view->name}:{$display->id}"] = "{$view->human_name}: {$display->display_title}";
            }
          }
        }
      }
      asort($options);
      $this->value_options = $options;
    }
  }

  /**
   * {@inheritdoc}
   */
  public function value_form(&$form, &$form_state) {
    $this->get_value_options();
    $options = $this->value_options;
    $default_value = (array) $this->value;

    $form['value'] = array(
      '#type' => 'select',
      '#title' => t('Views/displays to exclude'),
      '#description' => t("Please select which views/displays results will be excluded from this view's results."),
      '#options' => $options,
      '#default_value' => $default_value,
      '#multiple' => TRUE,
      '#size' => count($options) > 8 ? 8 : count($options),
    );

    $form['force_load'] = array(
      '#type' => 'checkbox',
      '#title' => t('Force view loading'),
      '#description' => t("By default, results of the selected views will be excluded only if the view has been previously loaded in the page. Check to force the filter to load the view if necessary."),
      '#default_value' => $this->options['force_load'],
    );
  }

  /**
   * {@inheritdoc}
   */
  public function query() {
    $this->ensure_my_table();
    $this->set_exclusion_list();

    if (!empty($this->exclusion_list)) {
      // Add the condition to the query.
      $field = "{$this->table_alias}.{$this->real_field}";
      $this->query->add_where(0, $field, $this->exclusion_list, 'not in');
    }
  }

  /**
   * Sets the array with IDs added to the exclusion list.
   */
  function set_exclusion_list() {
    $excluded = &drupal_static(EXCLUSION_LIST_STORAGE);
    $list = array();

    if (isset($this->value['all'])) {
      // Exclude results from any view previously loaded.
      foreach ($excluded as $view_storage) {
        $list = array_merge($list, $view_storage);
      }
    }
    else {
      // Get excluded IDs from the selected view/display combos.
      foreach ($this->value as $view_id) {
        // If the view hasn't been loaded and the force loading setting has been
        // enabled, load the view to feed the list with its results.
        if (!isset($excluded[$view_id]) && $this->options['force_load']) {
          list ($name, $display) = explode(':', $view_id);
          views_get_view_result($name, $display);
        }
        if (isset($excluded[$view_id]) && !empty($excluded[$view_id])) {
          $list = array_merge($list, $excluded[$view_id]);
        }
      }
    }

    $this->exclusion_list = $list;
  }

}
