<?php

/**
 * @file
 * Implementation of migration from Commonspot timeline custom element to
 * Drupal timeline nodes.
 */

class NistNikeAuthorsMigration extends NistMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Import of NIKE Authors as separate entities');

    $fields = array(
      'display_name' => 'concatenation of first middle and last names for title sake',
      'author_id' => 'ID in NIKE',
      'author_first' => 'first name',
      'author_last' => 'last name',
      'last_modified' => 'last updated date of author record',
      'nist_id' => 'CPR ID for the NIKE author',
      'author_middle' => 'Middle initial or name'
    );
    
    $this->highwaterField = array(
      'name' => 'last_modified', // Column to be used as highwater mark
      'alias' => 'f',
    );

    $this->source = new MigrateSourceJSON('', 'nist_id', $fields);

    $this->destination = new MigrateDestinationNode('pub_authors');

    $this->map = new MigrateSQLMap(
      $this->machineName,
      array(
        'author_id' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'NIKE Author ID',
        )
      ),
      MigrateDestinationNode::getKeySchema()
    );

    // Mapped fields
    $this->addFieldMapping('uid')
      ->defaultValue('1');
    $this->addFieldMapping('status')
      ->defaultValue('1');
    $this->addFieldMapping('changed', 'last_modified');
    $this->addFieldMapping('title', 'display_name');
    $this->addFieldMapping('field_peoplefinder_id', 'nist_id');
    $this->addFieldMapping('field_nike_author_id', 'author_id');
    $this->addFieldMapping('field_nist_author', 'nist_id')
      ->separator(',')
      ->sourceMigration('NistUser');
    $this->addFieldMapping('field_first_name', 'author_first');
    $this->addFieldMapping('field_last_name', 'author_last');
    $this->addFieldMapping('field_middle_name', 'author_middle');
    $this->addFieldMapping('pathauto')
      ->defaultValue('0');
    
    // Unmapped destination fields
    $this->addUnmigratedDestinations(array('created', 'translate', 'is_new',
      'revision_uid', 'promote', 'sticky', 'revision', 'log', 'language',
      'tnid', 'comment'));
  }

  /**
   * Implementation of Migration::prepareRow($row).
   */
  public function prepareRow($row) {

    // add some time (one min) to the changed value so it is forced to be updated.
    // $row->changed = get_updated_time_for_force_updating( $row->changed );
  }

  /**
   * I increment a date so Migrate will FORCE the updating of a migration
   * @param $time - I am the date/time to add to
   * @return DateTime
   */
  function getUpdateTimeForForcedMigration($time) {

    $newDate = date_add(
      date_create( $time ),
      // use for updating
      date_interval_create_from_date_string('35 seconds')
    // use for no updating
    //date_interval_create_from_date_string('0 seconds')
    );

    $result = date_timestamp_get( $newDate );

    return  $result;
  }


  /**
   * I am called by convention by the Migrate framework
   * @param $node
   * @param \stdClass $row
   */
  public function complete($node, stdClass $row) {

    $node_alias = url("node/{$node->nid}");
    $node_alias = ltrim($node_alias, '/');
    $legacyCommonSpotURL = 'manuscript-publication-search.cfm?pub_id=';
    $legacyCommonSpotURL = $legacyCommonSpotURL . $row->id;

    $redirect = new stdClass();
    redirect_object_prepare($redirect);

    $redirect->source = 'manuscript-publication-search.cfm';
    $redirect->source_options = array(
      'query' =>
        array(
          'pub_id' => $row->id
        )
    );
    $redirect->redirect = $node_alias;
    $redirect->hash = drupal_hash_base64($row->id);

    redirect_save($redirect);

    drush_log('REDIRECT', $type = 'warning', $error = NULL);
    drush_log($legacyCommonSpotURL, $type = 'warning', $error = NULL);
    drush_log($node_alias, $type = 'warning', $error = NULL);


    return TRUE;
  }

}

