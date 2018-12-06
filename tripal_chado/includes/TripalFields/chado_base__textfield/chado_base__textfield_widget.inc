<?php


class chado_base__textfield_widget extends ChadoFieldWidget {

  // The list of field types for which this widget is appropriate.
  public static $field_types = ['text'];
  public static $default_label = 'Chado Text Field';


  /**
   *
   * @see TripalFieldWidget::form()
   */
  public function form(&$widget, &$form, &$form_state, $langcode, $items, $delta, $element) {
    parent::form($widget, $form, $form_state, $langcode, $items, $delta, $element);

    $settings = $this->field['settings'];
    $field_name = $this->field['field_name'];
    $field_type = $this->field['type'];
    $field_table = $this->instance['settings']['chado_table'];
    $field_column = $this->instance['settings']['chado_column'];

    $value = '';
    // Get the field defaults.
    if (count($items) > 0 and array_key_exists('value', $items[0])) {
      $value = $items[0]['value'];
    }

    if (array_key_exists('values', $form_state) and
      array_key_exists($field_name, $form_state['values'])) {
      $value = $form_state['values'][$field_name][$langcode][$delta]['value'];
    }

    $widget['value'] = [
      '#type' => 'value',
      '#value' => $value,
    ];

    $widget['chado-' . $field_table . '__' . $field_column] = [
      '#type' => 'textfield',
      '#title' => $element['#title'],
      '#description' => $element['#description'],
      '#weight' => isset($element['#weight']) ? $element['#weight'] : 0,
      '#default_value' => $value,
      '#delta' => $delta,
      '#cols' => 30,
    ];
  }

  /**
   * @see TripalFieldWidget::validateDefaults()
   */
  public function validateDefaults($element, $form, &$form_state, $langcode, $delta) {
    $field_name = $this->field['field_name'];
    $field_table = $this->instance['settings']['chado_table'];
    $field_column = $this->instance['settings']['chado_column'];

    $value = $form_state['values'][$field_name]['und'][$delta]['chado-' . $field_table . '__' . $field_column];
    $form_state['values'][$field_name]['und'][$delta]['value'] = $value;
  }

  /**
   *
   * @see TripalFieldWidget::submit()
   */
  public function validate($element, $form, &$form_state, $langcode, $delta) {
    $field_name = $this->field['field_name'];
    $field_table = $this->instance['settings']['chado_table'];
    $field_column = $this->instance['settings']['chado_column'];

    // If the information has been removed then we want to signal such.
    if (!empty($form_state['values'][$field_name]['und'][$delta]['value'])) {
      $form_state['values'][$field_name]['und'][$delta]['value'] = 'delete_me';
      $form_state['values'][$field_name]['und'][$delta]['chado-' . $field_table . '__' . $field_column] = '';
    }
  }
}

