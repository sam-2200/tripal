<?php


/**
 * A base class for all Fields supported by Tripal.
 *
 * This class provides all of the necessary functions for a TripalField field.
 * It helps simplify and unify the process of creating fields for Tripal.  This
 * class simply defines the function prototypes. It is up to the class that
 * extends this class to implement the functions.
 *
 * Each module that creates new fields should use the normal Field API hooks
 * (e.g. hook_field_info(), hook_field_widget_form(), etc.) to instantiate the
 * appropriate TripalField class.
 *
 * Because of the way Drupal handles callbacks, AJAX callbacks, and theme
 * functions cannot be part of the implementation of this class.  Those
 * functions if needed, should be added to the bottom of the file where the
 * child class is housed.
 *
 */
class TripalField {
  /**
   * Provides information about this field.
   *
   * @return array
   *   An associative array with key/value pairs compatible with those from
   *   the hook_field_info() function of the Drupal Field API.
   */
  public function field_info() {
    return array(
    );
  }


  /**
   * Provides an array that allows Tripal to attach a field to an entity.
   *
   * @todo: This function needs better documentation.
   *
   */
  public function attach_info($entity_type, $bundle, $settings) {

  }
  /**
   * Provides information about the widget for this field.
   *
   * @return array
   *   An associative array with key/value paris compatible with those from the
   *   hook_field_widget_info() function of the Drupal Field API.
   */
  public function widget_info() {
    return array(
    );
  }
  /**
   * Provides information about the formatter for this field.
   *
   * @return
   *   An associative array with key/value paris compatible with those from the
   *   hook_field_formatter_info() function of the Drupal Field API.
   *
   */
  public function formatter_info() {
    return array(
    );
  }
  /**
   * Provides a summary of the formatter settings.
   *
   * On the 'Manage Display' page of the content type administration page,
   * fields are allowed to provide a settings form.  This settings form can
   * be used to allow the site admin to define how the field should be
   * formatted.  The settings are then available for the formatter()
   * function of this class.  This function provides a text-based description
   * of the settings for the site developer to see.  It appears on the manage
   * display page inline with the field.  A field must always return a
   * value in this function if the settings form gear button is to appear.
   *
   * See the hook_field_formatter_settings_summary() function for more
   * information.
   *
   * @param $field
   * @param $instance
   * @param $view_mode
   *
   * @return string
   *   A string that provides a very brief summary of the field settings
   *   to the user.
   *
   */
  public function formatter_settings_summary($field, $instance, $view_mode) {

  }

  /**
   * Provides the field's setting form.
   *
   * The settings form appears on the 'Manage Display' page of the content
   * type administration page. This function provides the form that will
   * appear on that page.
   *
   * To add a validate function, please create a static function in the
   * implementing class, and indicate that this function should be used
   * in the form array that is returned by this function.
   *
   * This form will not be displayed if the formatter_settings_summary()
   * function does not return anything.
   *
   * @param $field
   * @param $instance
   * @param $view_mode
   * @param $form
   * @param $form_state
   *
   * @return
   *   A Drupal Form array containing the settings form for this field.
   */
  public function formatter_settings_form($field, $instance,
      $view_mode, $form, &$form_state) {

  }

  /**
   *  Provides the display for a field
   *
   *  This function provides the display for a field when it is viewed on
   *  the web page.  The content returned by the formatter should only include
   *  what is present in the $items[$delta]['values] array. This way, the
   *  contents that are displayed on the page, via webservices and downloaded
   *  into a CSV file will always be identical.  The view need not show all
   *  of the data in the 'values' array.
   *
   *  @param $element
   *  @param $entity_type
   *  @param $entity
   *  @param $field
   *  @param $instance
   *  @param $langcode
   *  @param $items
   *  @param $display
   *
   *  @return
   *    An element array compatible with that returned by the
   *    hook_field_formatter_view() function.
   */
  public function formatter_view(&$element, $entity_type, $entity,
      $field, $instance, $langcode, $items, $display) {

    foreach($items as $delta => $item) {
      $element[$delta] = array(
        '#type' => 'markup',
        '#markup' => $item['value'],
      );
    }
  }

  /**
   * Provides the form for editing of this field.
   *
   * This form is diplayed when the user creates a new entity or edits an
   * existing entity.  If the field is attached to the entity then the form
   * provided by this function will be displayed.
   *
   * At a minimum, the form must have a 'value' element.  For Tripal, the
   * 'value' element of a field always corresponds to the value that is
   * presented to the end-user either directly on the page (with formatting)
   * or via web services, or some other mechanism.  However, the 'value' is
   * sometimes not enough for a field.  For example, the Tripal Chado module
   * maps fields to table columns and sometimes those columns are foreign keys
   * therefore, the Tripal Chado modules does not use the 'value' but adds
   * additional elements to help link records via FKs.  But even in this case
   * the 'value' element must always be present in the returne form and in such
   * cases it's value should be set equal to that added in the 'load' function.
   *
   * @param $widget
   * @param $form
   * @param $form_state
   * @param $field
   * @param $instance
   * @param $langcode
   * @param $items
   * @param $delta
   * @param $element
   *
   * @return
   *   A Drupal form. See the hook_field_widget_form() function for more information.
   */
  public function widget_form(&$widget, &$form, &$form_state, $field, $instance,
      $langcode, $items, $delta, $element) {

    $widget['value'] = array(
      '#type' => 'value',
      '#value' => array_key_exists($delta, $items) ? $items[$delta]['value'] : '',
    );
  }

  /**
   *  Perform validation of the widget_form when adding or editing the entity.
   *
   *  Any errors encountered should be indicatd by adding a value to the $errors
   *  array according to the instructions below.
   *
   *  @param $entity_type
   *    The type of $entity.
   *  @param $entity
   *    The entity for the operation.
   *  @param $field
   *    The field structure for the operation.
   *  @param $instance
   *    The instance structure for $field on $entity's bundle.
   *  @param $langcode
   *    The language associated with $items.
   *  @param $items
   *    $entity->{$field['field_name']}[$langcode], or an empty array if unset.
   *  @param $errors
   *    The array of errors (keyed by field name, language code, and delta) that
   *    have already been reported for the entity. The function should add its
   *    errors to this array. Each error is an associative array with the
   *    following keys and values:
   *      - error: An error code (should be a string prefixed with the
   *        module name).
   *      - message: The human readable message to be displayed.
   *
   */
  public function validate($entity_type, $entity, $field, $instance, $langcode,
      $items, &$errors) {

  }

  /**
   *  Performs extra commands when the entity form is submitted.
   *
   *  Drupal typically does not provide a submit hook for fields.  The
   *  TripalField provides one to allow for behind-the-scenes actions to
   *  occur.   This function should never be used for updates, deletes or
   *  inserts into the storage backend. Rather, the appropriate Field Storage
   *  implementation will take care of that. An example where this function
   *  may be useful would be to set values in the $items array using values
   *  of the other.
   *
   *  @param $entity_type
   *    The type of $entity.
   *  @param $entity
   *    The entity for the operation.
   *  @param $field
   *    The field structure for the operation.
   *  @param $instance
   *    The instance structure for $field on $entity's bundle.
   *  @param $langcode
   *    The language associated with $items.
   *  @param $items
   *    $entity->{$field['field_name']}[$langcode], or an empty array if unset.
   *  @param $form
   *    The submitted form array.
   *  @param $form_state.
   *    The form state array.
   */
  public function submit($entity_type, $entity, $field, $instance, $langcode,
      &$items, $form, &$form_state) {
  }
  /**
   * Loads the field values from the underlying data store.
   *
   * @param $field
   * @param $entity
   * @param $details
   *
   * @return
   *   An array of the following format:
   *     $entity->{$field_name}['und'][0]['value'] = $value;
   *   where:
   *     - $entity is the enity object to which this field is attached.
   *     - $field_name is the name of this field
   *     - 'und' is the language code (in this case 'und' == undefined)
   *     - 0 is the cardinality.  Increment by 1 when more than one item is
   *       available.
   *     - 'value' is the key indicating the value of this field. It should
   *       always be set.  The value of the 'value' key will be the contents
   *       used for web services and for downloadable content.  The value
   *       should be of the follow format types: 1) A single value (text,
   *       numeric, etc.) 2) An array of key value pair. 3) If multiple entries
   *       then cardinality should incremented and format types 1 and 2 should
   *       be used for each item.
   *   The array may contain as many other keys at the same level as 'value'
   *   but those keys are for internal field use and are not considered the
   *   value of the field.
   *
   *
   */
  public function load($field, $entity, $details) {

  }

  /**
   * Provides a form for the 'Field Settings' of the field management page.
   *
   * This is an optional hook function and is similar to the
   * hook_field_settings_form function().
   *
   * @param $field
   *   The field structure being configured.
   * @param $instance
   *   The instance structure being configured.
   * @param $has_data
   *   TRUE if the field already has data, FALSE if not.
   */
  public function settings_form($field, $instance, $has_data) {
    $settings = $field['settings'];
    $element = array();

//     $element['semantic_web'] = array(
//       '#type' => 'textfield',
//       '#title' => 'Semantic Web',
//       '#description' => t('Each field must be associated with a term
//           from a controlled vocabulary.  This allows computer programs to understand
//           the data provided on this site.  Please be cautions changing these
//           values.  Defaults are set by Tripal and sites that use the same
//           terms can exchange information.'),
//       '#collapsed' => TRUE,
//       '#collapsible' => TRUE,
//       '#tree' => TRUE,
//     );


    return $element;
  }

  /**
   * Describes this fields "data tables" to Views.
   *
   * This function is the equivalent of the hook_views_data() function of
   * the Drupal Views API.  It provides the necessary details to allow
   * Views to integrate the field.
   *
   * @return
   *   An associative array describing the data structure of the field.
   */
  public function views_data_alter(&$data, $field, $entity_info) {

  }

}