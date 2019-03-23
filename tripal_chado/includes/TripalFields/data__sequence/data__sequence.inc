<?php

class data__sequence extends ChadoField {


  // --------------------------------------------------------------------------
  //                     EDITABLE STATIC CONSTANTS
  //
  // The following constants SHOULD be set for each descendent class.  They are
  // used by the static functions to provide information to Drupal about
  // the field and it's default widget and formatter.
  // --------------------------------------------------------------------------

  // The default lable for this field.
  public static $default_label = 'Sequence';

  // The default description for this field.
  public static $description = 'A field for managing nucleotide and protein residues.';

  // Provide a list of instance specific settings. These can be access within
  // the instanceSettingsForm.  When the instanceSettingsForm is submitted
  // then Drupal with automatically change these settings for the instnace.
  // It is recommended to put settings at the instance level whenever possible.
  // If you override this variable in a child class be sure to replicate the
  // term_name, term_vocab, term_accession and term_fixed keys as these are
  // required for all TripalFields.
  public static $default_instance_settings = [
    // The short name for the vocabulary (e.g. shcema, SO, GO, PATO, etc.).
    'term_vocabulary' => 'data',
    // The name of the term.
    'term_name' => 'sequence',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => '2044',
    // Set to TRUE if the site admin is allowed to change the term
    // type. This will create form elements when editing the field instance
    // to allow the site admin to change the term settings above.
    'term_fixed' => FALSE,
  ];

  // Indicates the download formats for this field.  The list must be the
  // name of a child class of the TripalFieldDownloader.
  public static $download_formatters = [
    'TripalTabDownloader',
    'TripalCSVDownloader',
    'TripalNucFASTADownloader',
  ];

  // The default widget for this field.
  public static $default_widget = 'data__sequence_widget';

  // The default formatter for this field.
  public static $default_formatter = 'data__sequence_formatter';


  /**
   * @see TripalField::elementInfo()
   */
  public function elementInfo() {
    $field_term = $this->getFieldTermID();
    return [
      $field_term => [
        'operations' => [],
        'sortable' => FALSE,
        'searchable' => FALSE,
        'type' => 'xs:string',
        'readonly' => FALSE,
        'elements' => [
          'data:2044' => [
            'name' => 'sequence',
            'searchable' => FALSE,
          ],
          // @todo add this cvterm to the db.
          // https://www.ebi.ac.uk/ols/ontologies/ncit/terms?iri=http%3A%2F%2Fpurl.obolibrary.org%2Fobo%2FNCIT_C47845
          'NCIT:C47845' => [
            'name' => 'FASTA Format',
            'searchable' => FALSE,
          ],
        ],
      ],
    ];
  }

  /**
   * @see TripalField::load()
   */
  public function load($entity) {
    $field_name = $this->field['field_name'];
    $feature = (array) $entity->chado_record;

    // Retrieve all the sequence for the given feature.
    $sequences = chado_get_feature_sequences($feature, []);
    dpm($sequences, 'seq');
    // If there are no direct residues then try to derive from the parent.
    if (empty($sequences) OR (sizeof($sequences) == 1 AND empty($sequences[0]['residues']))) {
      $sequences = chado_get_feature_sequences($feature, ['derive_from_parent' => 1]);
    }

    // Add each sequence to the field as a fasta record.
    // NOTE: We keep the residues separate in case the formatter wants to
    //   determine it's own fasta record defline.
    // @debug dpm($sequences, 'sequences');
    foreach ($sequences as $k => $seq) {
      if (!empty($seq['residues'])) {
        $entity->{$field_name}['und'][$k]['value'] = [
          'NCIT:C47845' => '>' . $seq['defline'] . "\n" . $seq['residues'],
          'data:2044' => $seq['residues'],
        ];
      }
    }
  }
}

