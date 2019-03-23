<?php

class data__sequence_formatter extends ChadoFieldFormatter {

  // The default lable for this field.
  public static $default_label = 'Sequence';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = ['data__sequence'];

  /**
   * @see TripalFieldFormatter::view()
   */
  public function view(&$element, $entity_type, $entity, $langcode, $items, $display) {

    // If there are no items, we don't want to return any markup.
    if (count($items) == 0 or (count($items) == 1 and empty($items[0]['value']['data:2044']))) {
      $element[0] = [
        '#type' => 'markup',
        '#markup' => 'No sequence is available.',
      ];
      return;
    }

    $num_bases = 50;

    foreach ($items as $k => $d) {
      $rendered_seq = '<pre class="residues-formatter">';
      $rendered_seq .= $d['value']['NCIT:C47845']; // render the fasta record.
      $rendered_seq .= '</pre>';

      $element[$k] = [
        // We create a render array to produce the desired markup,
        '#type' => 'markup',
        '#markup' => $rendered_seq,
      ];
    }
  }
}
