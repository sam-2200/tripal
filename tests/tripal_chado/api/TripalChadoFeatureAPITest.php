<?php
namespace Tests\tripal_chado\api;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;

class TripalChadoFeatureAPITest extends TripalTestCase {
  // Uncomment to auto start and rollback db transactions per test method.
  use DBTransaction;

  /**
   * Tests chado_get_feature_sequences().
   *
   * @group tripal_chado_api
   * @group feature-module
   */
  public function testChadoGetFeatureSeqs() {

    // Preparation... Create the features.
    $base_seq = 'GGG' . str_repeat('A', 25) . str_repeat('G', 10) . str_repeat('C', 50) . 'GGG';
    $base_feature = factory('chado.feature')->create([
      'residues' => $base_seq,
    ]);
    // Create two derived features matching the region of AAA's.
    $mRNA_feature = factory('chado.feature')->create([
      'residues' => '',
    ]);
    chado_insert_record('featureloc', [
      'feature_id' => $mRNA_feature->feature_id,
      'srcfeature_id' => $base_feature->feature_id,
      'fmin' => 3,
      'fmax' => (3 + 25),
      'strand' => 1,
      'rank' => 0,
    ]);
    chado_insert_record('featureloc', [
      'feature_id' => $mRNA_feature->feature_id,
      'srcfeature_id' => $base_feature->feature_id,
      'fmin' => (3 + 25 + 10),
      'fmax' => (3 + 25 + 10 + 50),
      'strand' => 1,
      'rank' => 1,
    ]);

    // Make sure we can retrieve the correct sequence for the base feature.
    $returned_seq_results = chado_get_feature_sequences([
      'feature_id' => $base_feature->feature_id
    ], ['is_html' => FALSE, 'width' => 100]);

    $this->assertCount(1, $returned_seq_results, "There should only be the base sequence returned.");

    $returned_seq = $returned_seq_results[0]['residues'];
    $this->assertEquals($base_seq, $returned_seq, "The returned sequence did not match the base sequence we created the feature with.");

    // Now check we can get the derived sequence based using the mRNA.
    $returned_seq_results = chado_get_feature_sequences([
      'feature_id' => $mRNA_feature->feature_id
    ], ['is_html' => FALSE, 'width' => 100, 'derive_from_parent' => 1]);
    $this->assertCount(2, $returned_seq_results, 'We added two locations therefore we should have two returned sequences.');
    $this->assertEquals(
      str_repeat('A', 25),
      $returned_seq_results[0]['residues'],
      'The first region located on the base was not returned correctly.'
    );
    $this->assertEquals(
      str_repeat('C', 50),
      $returned_seq_results[1]['residues'],
      'The second region located on the base was not returned correctly.'
    );

    // Now change our featurelocs to be on the negative strand.
    // Note: fmin/fmax do not change because they are always the leftmost not
    // necessarily the start or 5' end.
    chado_delete_record('featureloc',
      ['srcfeature_id' => $base_feature->feature_id]);
    chado_insert_record('featureloc', [
      'feature_id' => $mRNA_feature->feature_id,
      'srcfeature_id' => $base_feature->feature_id,
      'fmin' => 3,
      'fmax' => (3 + 25),
      'strand' => -1,
      'rank' => 0,
    ]);
    chado_insert_record('featureloc', [
      'feature_id' => $mRNA_feature->feature_id,
      'srcfeature_id' => $base_feature->feature_id,
      'fmin' => (3 + 25 + 10),
      'fmax' => (3 + 25 + 10 + 50),
      'strand' => -1,
      'rank' => 1,
    ]);

    // Now check we can get the derived sequence based using the negative mRNA.
    $returned_seq_results = chado_get_feature_sequences([
      'feature_id' => $mRNA_feature->feature_id
    ], ['is_html' => FALSE, 'width' => 100, 'derive_from_parent' => 1]);
    $this->assertCount(2, $returned_seq_results, 'We added two locations therefore we should have two returned sequences.');
    $this->assertEquals(
      str_repeat('G', 50),
      $returned_seq_results[0]['residues'],
      'The first region located on the base was not returned correctly.'
    );
    $this->assertEquals(
      str_repeat('T', 25),
      $returned_seq_results[1]['residues'],
      'The second region located on the base was not returned correctly.'
    );
  }
}
