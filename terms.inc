<?php

/**
 * @file
 * Implementation of migration from Commonspot's Figleaf FLEET taxonomy to a 
 * Drupal taxonomy
 */

class NistTermMigration extends NistMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->description =
      t('Migrate terms currently stored in FLEET database to Drupal\'s built in
        taxonomy terms.');
      
    $fields = array(   
      'termid' => 'Commonspot ID of the term',
      'parent_id' => 'Parent term ID',
      'termname' => 'Term name',
      'parentname' => 'Parent term in plain english'
    );
    
    // URL of the JSON feed from Commonspot
    $json_file = "";
    $this->source = new MigrateSourceList(new TermListJSON($json_file),
      new TermItemJSON($json_file, array()), $fields);

    // Set up our destination - terms in the migrate_example_beer_styles
    // vocabulary (note that we pass the machine name of the vocabulary).
    $this->destination = new MigrateDestinationTerm('nist_topics');

    // Create a map object for tracking the relationships between source rows
    // and their resulting Drupal objects. We will use the MigrateSQLMap class,
    // which uses database tables for tracking. Pass the machine name (BeerTerm)
    // of this migration to use in generating map and message table names.
    // And, pass schema definitions for the primary keys of the source and
    // destination - we need to be explicit for our source, but the destination
    // class knows its schema already.
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'termid' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'Term ID',
        )
        ),
        MigrateDestinationTerm::getKeySchema()
      );

    $this->addFieldMapping('name', 'termname');
    $this->addFieldMapping('parent_name', 'parentname');
    
    // Unmapped destination fields
    $this->addUnmigratedDestinations(array('parent', 'format', 'weight',
      'description', 'path'));
    
    // Unmapped destination fields
    $this->addUnmigratedSources(array('parent_id'));

  }
}

