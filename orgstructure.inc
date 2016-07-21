<?php

/**
 * @file
 * Implementation of migration of NIST organizational structure to a
 * Drupal taxonomy
 */

class NistOrgStructureMigration extends NistMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->description =
      t('Migrate org structure currently stored in CPR database to Drupal\'s built in taxonomy terms.');
      
    $fields = array(
      'org_id' => 'CPR ID for org',
      'org_parent_id' => 'CPR ID of Parent',
      'org_cd' => 'the number of the organization',
      'org_name' => 'Organization Name',
      'org_acrnm' => 'Organizational acronym'
    );

    /*
    / OLD DB source, changed to JSON for mo bettaness
    $query = Database::getConnection('default')
        ->select('nist_people_vw_org_with_ou', 'u')
        ->fields('u', array('org_id', 'org_parent_id', 'org_name', 'org_acrnm', 'org_cd'))
        ->orderBy('org_parent_id', 'ASC');


    // set the source as our DB query above
    $this->source = new MigrateSourceSQL($query);
    */

    $this->source = new MigrateSourceJSON('', 'org_id', $fields);

    // Set up our destination vocabulary.
    $this->destination = new MigrateDestinationTerm('nist_org');

    // Create a map object for tracking the relationships between source rows
    // and their resulting Drupal objects.
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'org_id' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'Organization ID',
        )
        ),
        MigrateDestinationTerm::getKeySchema()
      );

    $this->addFieldMapping('name', 'org_name');
    $this->addFieldMapping('parent', 'org_parent_id')
      ->sourceMigration('NistOrgStructure');
    $this->addFieldMapping('field_acronym', 'org_acrnm');
    $this->addFieldMapping('field_acronym:language')
      ->defaultValue('en');
    $this->addFieldMapping('field_org_cd', 'org_cd');
    $this->addFieldMapping('field_org_cd:language')
      ->defaultValue('en');

    // Unmapped destination fields
    $this->addUnmigratedDestinations(array('parent_name', 'format',
      'weight', 'description', 'path', 'pathauto'));
    
    // Unmapped Sources fields
    //$this->addUnmigratedSources(array(''));

  }

  /**
   * Implementation of Migration::prepareRow($row).
   */
  public function prepareRow($row) {
    $this->drushLog("imma here");
  }
  
  
}

