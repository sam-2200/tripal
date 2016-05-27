<?php

class chado_linker__relationship extends TripalField {
  /**
   * @see TripalField::field_info()
   */
  function field_info() {
    return array(
      'label' => t('Relationships'),
      'description' => t('Relationships between records.'),
      'default_widget' => 'chado_linker__relationship_widget',
      'default_formatter' => 'chado_linker__relationship_formatter',
      'settings' => array(),
      'storage' => array(
        'type' => 'field_chado_storage',
        'module' => 'tripal_chado',
        'active' => TRUE
      ),
    );
  }

  /**
   * @see TripalField::attach_info()
   */
  function attach_info($entity_type, $bundle, $target) {
    $field_info = array();

    $table_name = $target['data_table'];
    $type_table = $target['type_table'];
    $type_field = $target['field'];
    $cv_id      = $target['cv_id'];
    $cvterm_id  = $target['cvterm_id'];

    // If the linker table does not exists then we don't want to add attach.
    $rel_table = $table_name . '_relationship';
    if (!chado_table_exists($rel_table)) {
      return $field_info;
    }

    $schema = chado_get_schema($rel_table);
    $pkey = $schema['primary key'][0];

    // Initialize the field array.
    $field_info = array(
      'field_name' => $table_name . '_relationship',
      'field_type' => 'chado_linker__relationship',
      'widget_type' => 'chado_linker__relationship_widget',
      'widget_settings' => array('display_label' => 1),
      'description' => 'A generic field for displaying relationships between data types',
      'label' => 'Relationships',
      'is_required' => 0,
      'cardinality' => FIELD_CARDINALITY_UNLIMITED,
      'storage' => 'field_chado_storage',
      'field_settings' => array(
        'chado_table' => $rel_table,
        'chado_column' => $pkey,
        'base_table' => $table_name,
        'semantic_web' => 'SBO:0000374',
      ),
    );

    return $field_info;

  }
  /**
   * @see TripalField::widget_info()
   */
  function widget_info() {
    return array(
      'label' => t('Relationship Settings'),
      'field types' => array('chado_linker__relationship')
    );
  }
  /**
   * @see TripalField::formatter_info()
   */
  function formatter_info() {
    return array(
      'label' => t('Relationships'),
      'field types' => array('chado_linker__relationship'),
      'settings' => array(
      ),
    );
  }

  /**
   * @see TripalField::formatter_view()
   */
  function formatter_view(&$element, $entity_type, $entity,
      $field, $instance, $langcode, $items, $display) {
    // Get the settings
    $settings = $display['settings'];

    $rows = array();
    $headers = array('Subject' ,'Type', 'Object');
    $headers = array('Relationship');

    foreach ($items as $delta => $item) {
      if (!$item['value']) {
        continue;
      }
      $subject_name = $item['value']['subject']['name'];
      $subject_type = $item['value']['subject']['type'];
      $object_name = $item['value']['object']['name'];
      $object_type = $item['value']['object']['type'];
      $phrase = $item['value']['phrase'];

      // Handle some special cases.
      // For mRNA objects we don't want to show the CDS, exons, 5' UTR, etc.
      // we want to show the parent gene and the protein.
      if ($object_type == 'mRNA' and ($subject_type != 'polypeptide')) {
        continue;
      }
      if ($subject_type == 'mRNA' and ($object_type != 'gene')) {
        continue;
      }

      $phrase = preg_replace("/$subject_type/", "<b>$subject_type</b>", $phrase);
      $phrase = preg_replace("/$object_type/", "<b>$object_type</b>", $phrase);

      if (array_key_exists('entity_id', $item['value']['object'])) {
        $object_entity_id = $item['value']['object']['entity_id'];
        if ($object_entity_id != $entity->id) {
          $link = l($object_name, 'bio_data/' . $object_entity_id);
          $phrase = preg_replace("/$object_name/", $link, $phrase);
        }
      }
      if (array_key_exists('entity_id', $item['value']['subject'])) {
        $subject_entity_id = $item['value']['subject']['entity_id'];
        if ($subject_entity_id != $entity->id) {
          $link = l($subject_name, 'bio_data/' . $subject_entity_id);
          $phrase = preg_replace("/$subject_name/", $link, $phrase);
        }
      }

      $rows[] = array($phrase);
    }


    // the $table array contains the headers and rows array as well as other
    // options for controlling the display of the table.  Additional
    // documentation can be found here:
    // https://api.drupal.org/api/drupal/includes%21theme.inc/function/theme_table/7
    $table = array(
      'header' => $headers,
      'rows' => $rows,
      'attributes' => array(
        'id' => 'chado-linker--relationship-table',
        'class' => 'tripal-data-table'
      ),
      'sticky' => FALSE,
      'caption' => '',
      'colgroups' => array(),
      'empty' => 'There are no relationships',
    );

    // once we have our table array structure defined, we call Drupal's theme_table()
    // function to generate the table.
    $element[$delta] = array(
      '#type' => 'markup',
      '#markup' => theme_table($table),
    );
  }
  /**
   * @see TripalField::widget_form()
   */
  function widget_form(&$widget, &$form, &$form_state, $field, $instance,
      $langcode, $items, $delta, $element) {

    $field_name = $field['field_name'];
    $field_type = $field['type'];
    $field_table = $field['settings']['chado_table'];
    $field_column = $field['settings']['chado_column'];

    // Get the FK column that links to the base table.
    $chado_table = $field['settings']['chado_table'];
    $base_table = $field['settings']['base_table'];
    $schema = chado_get_schema($chado_table);
    $pkey = $schema['primary key'][0];
    $fkeys = array_values($schema['foreign keys'][$base_table]['columns']);
    $fkey = $fkeys[0];

    // Get the field defaults.
    $record_id = '';
    $fkey_value = $element['#entity']->chado_record_id;
    $subject_id = '';
    $subject_uniquename = '';
    $type_id = '';
    $type = '';
    $object_id = '';
    $object_uniquename = '';
    $value = '';
    $rank = '';

    // If the field already has a value then it will come through the $items
    // array.  This happens when editing an existing record.
    if (array_key_exists($delta, $items)) {
      $record_id = $items[$delta][$field_table . '__' . $pkey];
      $subject_id = $items[$delta][$field_table . '__subject_id'];
      $type_id = $items[$delta][$field_table . '__type_id'];
      $object_id = $items[$delta][$field_table . '__object_id'];

      $value = $items[$delta][$field_table . '__value'];
      $rank = $items[$delta][$field_table . '__rank'];

      $object_uniquename = $items[$delta]['object_name'];
      $subject_uniquename = $items[$delta]['subject_name'];
      $type = $items[$delta]['type_name'];
    }

    // Check $form_state['values'] to see if an AJAX call set the values.
    if (array_key_exists('values', $form_state) and array_key_exists($delta, $form_state['values'])) {
      $record_id = tripal_chado_get_field_form_values($field_name, $form_state, $delta, $field_table . '__' . $pkey);
      $subject_id = tripal_chado_get_field_form_values($field_name, $form_state, $delta, $field_table . '__subject_id');
      $subject_uniquename = tripal_chado_get_field_form_values($field_name, $form_state, $delta, 'subject_name');
      $type_id = tripal_chado_get_field_form_values($field_name, $form_state, $delta, $field_table . '__type_id');
      $type = tripal_chado_get_field_form_values($field_name, $form_state, $delta, 'type_name');
      $object_id = tripal_chado_get_field_form_values($field_name, $form_state, $delta, $field_table . '__object_id');
      $object_uniquename = tripal_chado_get_field_form_values($field_name, $form_state, $delta, 'object_name');
      $value = tripal_chado_get_field_form_values($field_name, $form_state, $delta, $field_table . '__value');
      $rank = tripal_chado_get_field_form_values($field_name, $form_state, $delta, $field_table . '__rank');
    }

    $widget['#table_name'] = $chado_table;

    $widget['#fkeys'] = $schema['foreign keys'];
    $widget['#base_table'] = $base_table;
    $widget['#chado_record_id'] = $form['#entity']->chado_record_id;
    //$widget['#element_validate'] = array('chado_linker__relationship_validate');
    $widget['#theme'] = 'chado_linker__relationship_widget';
    $widget['#prefix'] =  "<span id='$chado_table-$delta'>";
    $widget['#suffix'] =  "</span>";

    $widget['value'] = array(
      '#type' => 'value',
      '#value' => array_key_exists($delta, $items) ? $items[$delta]['value'] : '',
    );
    $widget[$field_table . '__' . $pkey] = array(
      '#type' => 'value',
      '#default_value' => $record_id,
    );
    $widget[$field_table . '__subject_id'] = array(
      '#type' => 'value',
      '#default_value' => $subject_id,
    );
    $widget[$field_table . '__type_id'] = array(
      '#type' => 'value',
      '#default_value' => $type_id,
    );
    $widget[$field_table . '__object_id'] = array(
      '#type' => 'value',
      '#default_value' => $object_id,
    );
    if (array_key_exists('value', $schema['fields'])) {
      $widget[$field_table . '__value'] = array(
        '#type' => 'value',
        '#default_value' => $value,
      );
    }
    if (array_key_exists('rank', $schema['fields'])) {
      $widget[$field_table . '__rank'] = array(
        '#type' => 'value',
        '#default_value' => $rank,
      );
    }
    $widget['subject_name'] = array(
      '#type' => 'textfield',
      '#title' => t('Subject'),
      '#default_value' => $subject_uniquename,
      '#required' => $element['#required'],
      '#maxlength' => array_key_exists('length', $schema['fields']['subject_id']) ? $schema['fields']['subject_id']['length'] : 255,
      '#size' => 35,
      '#autocomplete_path' => "admin/tripal/storage/chado/auto_name/$base_table",
    );
    $default_cv = tripal_get_default_cv($field_table, 'type_id');
    if (!$default_cv) {
      drupal_set_message("There is no default vocabulary set for the relationships....");
    }
    $options = tripal_get_cvterm_select_options($default_cv->cv_id, TRUE);
    $widget['type_name'] = array(
      '#type' => 'select',
      '#title' => t('Type'),
      '#options' => $options,
      '#required' => $element['#required'],
      '#default_value' => $type_id,
    );
    $widget['object_name'] = array(
      '#type' => 'textfield',
      '#title' => t('Object'),
      '#default_value' => $object_uniquename,
      '#required' => $element['#required'],
      '#maxlength' => array_key_exists('length', $schema['fields']['object_id']) ? $schema['fields']['object_id']['length'] : 255,
      '#size' => 35,
      '#autocomplete_path' => "admin/tripal/storage/chado/auto_name/$base_table",
    );
  }
  /**
   * @see TripalField::validate()
   */
  function validate($entity_type, $entity, $field, $instance, $langcode, $items, &$errors) {

    $field_name = $field['field_name'];
    $field_type = $field['type'];
    $field_table = $field['settings']['chado_table'];
    $field_column = $field['settings']['chado_column'];
    $base_table = $field['settings']['base_table'];

    $chado_record_id = $entity->chado_record_id;

    $schema = chado_get_schema($field_table);
    $fkeys = $schema['foreign keys'];

    foreach ($items as $delta => $item) {
      $subject_id = $item[$field_table . '__subject_id'];
      $object_id = $item[ $field_table . '__object_id'];
      $type_id = $item[$field_table . '__type_id'];
      $type_name = $item['type_name'];
      $subject_name = $item['subject_name'];
      $object_name = $item['object_name'];


      // If the row is empty then just return, there's nothing to validate.
      if (!$type_name and !$subject_name and !$object_name) {
        continue;
      }

      // Make sure we have values for all of the fields.
      $form_error = FALSE;
      if (!$type_name) {
        $errors[$field['field_name']][$langcode][$delta][] = array(
          'error' => 'chado_linker__relationship',
          'message' => t("Please provide the type of relationship."),
        );
      }
      if (!$subject_name) {
        $errors[$field['field_name']][$langcode][$delta][] = array(
          'error' => 'chado_linker__relationship',
          'message' => t("Please provide the subject of the relationship."),
        );
      }
      if (!$object_name) {
        $errors[$field['field_name']][$langcode][$delta][] = array(
          'error' => 'chado_linker__relationship',
          'message' => t("Please provide the object of the relationship."),
        );
      }
      if ($form_error) {
        continue;
      }

      // Before submitting this form we need to make sure that our subject_id and
      // object_ids are real values.  There are two ways to get the value, either
      // just with the text value or with an [id: \d+] string embedded.  If the
      // later we will pull it out.
      $subject_id = '';
      $fkey_rcolumn = $fkeys[$base_table]['columns']['subject_id'];
      $matches = array();
      if (preg_match('/\[id: (\d+)\]/', $subject_name, $matches)) {
        $subject_id =  $matches[1];
        $values = array($fkey_rcolumn => $subject_id);
        $subject = chado_select_record($base_table, array($fkey_rcolumn), $values);
        if (count($subject) == 0) {
          $errors[$field['field_name']][$langcode][$delta][] = array(
            'error' => 'chado_linker__relationship',
            'message' => t("The subject record cannot be found using the specified id (e.g. [id: xx])."),
          );
        }
      }
      else {
        $values = array('uniquename' => $subject_name);
        $subject = chado_select_record($base_table, array($fkey_rcolumn), $values);
        if (count($subject) == 0) {
          $errors[$field['field_name']][$langcode][$delta][] = array(
            'error' => 'chado_linker__relationship',
            'message' => t("The subject record cannot be found. Please check spelling."),
          );
        }
        elseif (count($subject) > 1) {
          $errors[$field['field_name']][$langcode][$delta][] = array(
            'error' => 'chado_linker__relationship',
            'message' => t("The subject is not unique and therefore the relationship cannot be made."),
          );
        }
      }

      // Now check for a matching object.
      $object_id = '';
      $fkey_rcolumn = $fkeys[$base_table]['columns']['object_id'];
      $matches = array();
      if (preg_match('/\[id: (\d+)\]/', $object_name, $matches)) {
        $object_id = $matches[1];
        $values = array($fkey_rcolumn => $object_id);
        $object = chado_select_record($base_table, array($fkey_rcolumn), $values);
        if (count($subject) == 0) {
          $errors[$field['field_name']][$langcode][$delta][] = array(
            'error' => 'chado_linker__relationship',
            'message' => t("The object record cannot be found using the specified id (e.g. [id: xx])."),
          );
        }
      }
      else {
        $values = array('uniquename' => $object_name);
        $object = chado_select_record($base_table, array($fkey_rcolumn), $values);
        if (count($object) == 0) {
          $errors[$field['field_name']][$langcode][$delta][] = array(
            'error' => 'chado_linker__relationship',
            'message' => t("The object record cannot be found. Please check spelling."),
          );;
        }
        elseif (count($object) > 1) {
          $errors[$field['field_name']][$langcode][$delta][] = array(
            'error' => 'chado_linker__relationship',
            'message' =>  t("The object is not unique and therefore the relationship cannot be made."),
          );
        }
      }

      // Make sure that either our object or our subject refers to the base record.
      if ($object_id != $chado_record_id  and $subject_id != $chado_record_id) {
        $errors[$field['field_name']][$langcode][$delta][] = array(
          'error' => 'chado_linker__relationship',
          'message' =>  t("Either the subject or the object in the relationship must refer to this record."),
        );
      }

      if ($object_id == $chado_record_id  and $subject_id == $chado_record_id) {
        $errors[$field['field_name']][$langcode][$delta][] = array(
          'error' => 'chado_linker__relationship',
          'message' =>  t("The subject and the object in the relationship cannot both refer to this record."),
        );
      }
    }
  }
  /**
   * @see TripalField::submit()
   */
  public function submit($entity_type, $entity, $field, $instance, $langcode,
      &$items, $form, &$form_state) {

    $field_name = $field['field_name'];
    $field_type = $field['type'];
    $field_table = $field['settings']['chado_table'];
    $field_column = $field['settings']['chado_column'];
    $base_table = $field['settings']['base_table'];

    $chado_record_id = $entity->chado_record_id;

    $schema = chado_get_schema($field_table);
    $fkeys = $schema['foreign keys'];

    foreach ($items as $delta => $item) {
      $subject_id = $item[$field_table . '__subject_id'];
      $object_id = $item[ $field_table . '__object_id'];
      $type_id = $item[$field_table . '__type_id'];
      $type_name = $item['type_name'];
      $subject_name = $item['subject_name'];
      $object_name = $item['object_name'];

      // If the row is empty then skip this one, there's nothing to validate.
      if (!$type_name and !$subject_name and !$object_name) {
        continue;
      }

      // Get the subject ID.
      $subject_id = '';
      $fkey_rcolumn = $fkeys[$base_table]['columns']['subject_id'];
      $matches = array();
      if (preg_match('/\[id: (\d+)\]/', $subject_name, $matches)) {
        $subject_id =  $matches[1];
      }
      else {
        $values = array('uniquename' => $subject_name);
        $subject = chado_select_record($base_table, array($fkey_rcolumn), $values);
        $subject_id = $subject[0]->$fkey_rcolumn;
      }

      // Get the object ID.
      $object_id = '';
      $fkey_rcolumn = $fkeys[$base_table]['columns']['object_id'];
      $matches = array();
      if (preg_match('/\[id: (\d+)\]/', $object_name, $matches)) {
        $object_id = $matches[1];
      }
      else {
        $values = array('uniquename' => $object_name);
        $object = chado_select_record($base_table, array($fkey_rcolumn), $values);
        $object_id = $object[0]->$fkey_rcolumn;
      }

      // Set the IDs according to the values that were determined above.
      $items[$delta][$field_table . '__subject_id'] = $subject_id;
      $items[$delta][$field_table . '__object_id'] = $object_id;
      $items[$delta][$field_table . '__type_id'] = $type_name;
      $items[$delta][$field_table . '__rank'] = $item['_weight'];
    }
  }
  /**
   * @see TripalField::load()
   */
  function load($field, $entity, $details) {
    $settings = $field['settings'];

    $record = $details['record'];

    $field_name = $field['field_name'];
    $field_type = $field['type'];
    $field_table = $field['settings']['chado_table'];
    $field_column = $field['settings']['chado_column'];
    $base_table = $field['settings']['base_table'];

    // Get the PKey for this table
    $schema = chado_get_schema($field_table);
    $pkey = $schema['primary key'][0];

    // Get the Pkeys for the subject and object tables
    $subject_fkey_table = '';
    $object_fkey_table = '';
    $fkeys = $schema['foreign keys'];
    foreach ($fkeys as $fktable => $details) {
      foreach ($details['columns'] as $fkey_lcolumn => $fkey_rcolumn) {
        if ($fkey_lcolumn == 'subject_id') {
          $subject_fkey_table = $fktable;
        }
        if ($fkey_lcolumn == 'object_id') {
          $object_fkey_table = $fktable;
        }
      }
    }
    $subject_schema = chado_get_schema($subject_fkey_table);
    $object_schema = chado_get_schema($object_fkey_table);
    $subject_pkey = $subject_schema['primary key'][0];
    $object_pkey = $object_schema['primary key'][0];

    // Get the FK that links to the base record.
    $schema = chado_get_schema($field_table);
    $fkey_lcolumn = key($schema['foreign keys'][$base_table]['columns']);
    $fkey_rcolumn = $schema['foreign keys'][$base_table]['columns'][$fkey_lcolumn];

    // Set some defaults for the empty record.
    // TODO: don't hardcode the uniquename as all tables won't have that.
    $entity->{$field_name}['und'][0] = array(
      'value' => array(),
      $field_table . '__' . $pkey => '',
      $field_table . '__subject_id' => '',
      $field_table . '__object_id' => '',
      $field_table . '__type_id' => '',
      // These elements don't need to follow the naming scheme above
      // becasue we don't need the chado_field_storage to try and
      // save these values.
      'object_name' => '',
      'subject_name' => '',
      'type_name' => '',
    );
    if (array_key_exists('value', $schema['fields'])) {
      $entity->{$field_name}['und'][0][$field_table . '__value'] = '';
    }
    if (array_key_exists('rank', $schema['fields'])) {
      $entity->{$field_name}['und'][0][$field_table . '__rank'] = '';
    }

    if (!$record) {
      return;
    }

    // Expand the object to include the relationships.
    $options = array(
      'return_array' => 1,
      // we don't want to fully recurse we only need information about the
      // relationship type and the object and subject
      'include_fk' => array(
        'type_id' => 1,
        'object_id' => array(
          'type_id' => 1,
        ),
        'subject_id'  => array(
          'type_id' => 1,
        ),
      ),
    );
    $rel_table = $base_table . '_relationship';
    $schema = chado_get_schema($rel_table);
    if (array_key_exists('rank', $schema['fields'])) {
      $options['order_by'] = array('rank' => 'ASC');
    }
    $record = chado_expand_var($record, 'table', $rel_table, $options);
    if (!$record->$rel_table) {
      return;
    }
    $srelationships = $record->$rel_table->subject_id;
    $orelationships = $record->$rel_table->object_id;

    $i = 0;
    if ($orelationships) {
      foreach ($orelationships as $relationship) {
        $rel_acc = $relationship->type_id->dbxref_id->db_id->name . ':' . $relationship->type_id->dbxref_id->accession;
        $rel_type = $relationship->type_id->name;
        $verb = $this->get_rel_verb($rel_type);
        $subject_name = $relationship->subject_id->name;
        $subject_type = $relationship->subject_id->type_id->name;
        $object_name = $relationship->object_id->name;
        $object_type = $relationship->object_id->type_id->name;
        $entity->{$field_name}['und'][$i]['value'] = array(
          '@type' => $rel_acc,
          'subject' => array(
            '@type' => $relationship->subject_id->type_id->dbxref_id->db_id->name .
                ':' . $relationship->subject_id->type_id->dbxref_id->accession,
            'type' =>  $subject_type,
            'name' => $subject_name,
          ),
          'type' => $relationship->type_id->name,
          'object' => array(
            '@type' => $relationship->object_id->type_id->dbxref_id->db_id->name .
               ':' . $relationship->object_id->type_id->dbxref_id->accession,
            // TODO the entity_id should not be here.... wherre to put it.
            'entity_id' => $entity->id,
            'entity_type' => 'TripalEntity',
            'type' =>  $object_type,
            'name' => $object_name,
          )
        );
        if (property_exists($relationship->subject_id, 'uniquename')) {
          $subject_name = $relationship->subject_id->uniquename;
          $entity->{$field_name}['und'][$i]['value']['subject']['name'] = $subject_name;
        }
        if (property_exists($relationship->object_id, 'uniquename')) {
          $object_name = $relationship->object_id->uniquename;
          $entity->{$field_name}['und'][$i]['value']['object']['name'] = $object_name;
        }
        if (property_exists($relationship->subject_id, 'entity_id')) {
          $entity_id = $relationship->subject_id->entity_id;
          $entity->{$field_name}['und'][$i]['value']['subject']['entity_id'] = $entity_id;
          $entity->{$field_name}['und'][$i]['value']['subject']['entity_type'] = 'TripalEntity';
        }
        $rel_type_clean = lcfirst(preg_replace('/_/', ' ', $rel_type));
        $entity->{$field_name}['und'][$i]['value']['phrase'] = 'The ' . $subject_type . ', ' .
          $subject_name . ', ' . $verb . ' '  . $rel_type_clean . ' this '  .
          $object_type . '.';


        $entity->{$field_name}['und'][$i][$field_table . '__' . $pkey] = $relationship->$pkey;
        $entity->{$field_name}['und'][$i][$field_table . '__subject_id'] = $relationship->subject_id->$subject_pkey;
        $entity->{$field_name}['und'][$i][$field_table . '__type_id'] = $relationship->type_id->cvterm_id;
        $entity->{$field_name}['und'][$i][$field_table . '__object_id'] = $relationship->object_id->$object_pkey;

        $entity->{$field_name}['und'][$i]['type_name'] = $relationship->type_id->name;
        $entity->{$field_name}['und'][$i]['subject_name'] = $relationship->subject_id->uniquename . ' [id: ' . $relationship->subject_id->$fkey_rcolumn . ']';
        $entity->{$field_name}['und'][$i]['object_name'] = $relationship->object_id->uniquename  . ' [id: ' . $relationship->object_id->$fkey_rcolumn . ']';
        if (array_key_exists('value', $schema['fields'])) {
          $entity->{$field_name}['und'][$i][$field_table . '__value'] = $relationship->value;
        }
        if (array_key_exists('rank', $schema['fields'])) {
          $entity->{$field_name}['und'][$i][$field_table . '__rank'] = $relationship->rank;
        }
        $i++;
      }
    }

    if ($srelationships) {
      foreach ($srelationships as $relationship) {
        $rel_acc = $relationship->type_id->dbxref_id->db_id->name . ':' . $relationship->type_id->dbxref_id->accession;
        $rel_type = $relationship->type_id->name;
        $verb = $this->get_rel_verb($rel_type);
        $subject_name = $relationship->subject_id->name;
        $subject_type = $relationship->subject_id->type_id->name;
        $object_name = $relationship->object_id->name;
        $object_type = $relationship->object_id->type_id->name;
        $entity->{$field_name}['und'][$i]['value'] = array(
          '@type' => $rel_acc,
          'subject' => array(
            '@type' => $relationship->subject_id->type_id->dbxref_id->db_id->name .
               ':' . $relationship->subject_id->type_id->dbxref_id->accession,
            'type' =>  $subject_type,
            'name' => $subject_name,
            'entity_id' => $entity->id,
            'entity_type' => 'TripalEntity',
          ),
          'type' => $relationship->type_id->name,
          'object' => array(
            '@type' => $relationship->object_id->type_id->dbxref_id->db_id->name .
               ':' . $relationship->object_id->type_id->dbxref_id->accession,
            'type' =>  $object_type,
            'name' => $object_name,
          )
        );
        if (property_exists($relationship->subject_id, 'uniquename')) {
          $subject_name = $relationship->subject_id->uniquename;
          $entity->{$field_name}['und'][$i]['value']['subject']['name'] = $subject_name;
        }
        if (property_exists($relationship->object_id, 'uniquename')) {
          $object_name = $relationship->object_id->uniquename;
          $entity->{$field_name}['und'][$i]['value']['object']['name'] = $object_name;
        }
        if (property_exists($relationship->object_id, 'entity_id')) {
          $entity_id = $relationship->object_id->entity_id;
          $entity->{$field_name}['und'][$i]['value']['object']['entity_id'] = $entity_id;
          $entity->{$field_name}['und'][$i]['value']['object']['entity_type'] = 'TripalEntity';
        }
        $rel_type_clean = lcfirst(preg_replace('/_/', ' ', $rel_type));
        $entity->{$field_name}['und'][$i]['value']['phrase'] = 'This  ' .
          $subject_type . ' ' . $verb . ' '  . $rel_type_clean . ' the '  .
          $object_type . ', ' . $object_name . '.';
        $entity->{$field_name}['und'][$i][$field_table . '__' . $pkey] = $relationship->$pkey;
        $entity->{$field_name}['und'][$i][$field_table . '__subject_id'] = $relationship->subject_id->$subject_pkey;
        $entity->{$field_name}['und'][$i][$field_table . '__type_id'] = $relationship->type_id->cvterm_id;
        $entity->{$field_name}['und'][$i][$field_table . '__object_id'] = $relationship->object_id->$object_pkey;

        $entity->{$field_name}['und'][$i]['type_name'] = $relationship->type_id->name;
        $entity->{$field_name}['und'][$i]['subject_name'] = $relationship->subject_id->uniquename  . ' [id: ' . $relationship->subject_id->$fkey_rcolumn . ']';
        $entity->{$field_name}['und'][$i]['object_name'] = $relationship->object_id->uniquename  . ' [id: ' . $relationship->object_id->$fkey_rcolumn . ']';

        if (array_key_exists('value', $schema['fields'])) {
          $entity->{$field_name}['und'][$i][$field_table . '__value'] = $relationship->value;
        }
        if (array_key_exists('rank', $schema['fields'])) {
          $entity->{$field_name}['und'][$i][$field_table . '__rank'] = $relationship->rank;
        }
        $i++;
      }
    }
  }
  /**
   * A helper function to define English verbs for relationship types.
   *
   * @param $rel_type
   *   The vocabulary term name for the relationship.
   *
   * @return
   *   The verb to use when creating a sentence of the relationship.
   */
  private function get_rel_verb($rel_type) {
    $rel_type_clean = lcfirst(preg_replace('/_/', ' ', $rel_type));
    $verb = $rel_type_clean;
    switch ($rel_type_clean) {
      case 'integral part of':
      case 'instance of':
        $verb = 'is an';
        break;
      case 'proper part of':
      case 'transformation of':
      case 'genome of':
      case 'part of':
        $verb = 'is a';
      case 'position of':
      case 'sequence of':
      case 'variant of':
        $verb = 'is a';
        break;
      case 'derives from':
      case 'connects on':
      case 'contains':
      case 'finishes':
      case 'guides':
      case 'has origin':
      case 'has part':
      case 'has quality':
      case 'is consecutive sequence of':
      case 'maximally overlaps':
      case 'overlaps':
      case 'starts':
        break;
      default:
        $verb = 'is';
    }

    return $verb;
  }
}

/**
 * Theme function for the chado_linker__relationship_widget.
 */
function theme_chado_linker__relationship_widget($variables) {
  $element = $variables['element'];
  $field_name = $element['#field_name'];
  $field = field_info_field($field_name);
  $field_type = $field['type'];
  $field_table = $field['settings']['chado_table'];
  $field_column = $field['settings']['chado_column'];

  $layout = "
      <div class=\"chado-linker--relationship-widget\">
        <div class=\"chado-linker--relationship-widget-item\">" .
          drupal_render($element['subject_name']) . "
        </div>
        <div class=\"chado-linker--relationship-widget-item\">" .
          drupal_render($element['type_name']) . "
        </div>
        <div class=\"chado-linker--relationship-widget-item\">" .
          drupal_render($element['object_name']) . "
        </div>
      </div>
    ";
  return $layout;
}

