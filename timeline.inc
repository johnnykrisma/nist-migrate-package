<?php

/**
 * @file
 * Implementation of migration from Commonspot timeline custom element to
 * Drupal timeline nodes.
 */

class NistTimelineMigration extends NistMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Import of Commonspot timeline custom element into timeline content type');
    $this->dependencies = array(
      'NistImage'
    );

    $fields = array(
      'fic_caption' => 'Caption of the event',
      'fic_event_body' => 'Body of the event',
      'fic_event_date' => 'Date of the event',
      'fic_event_end_date' => 'end date of the event',
      'fic_event_headline' => 'headline',
      'fic_event_image' => 'image ID of the event',
      'fic_event_image_alt' => 'Alt text for the image',
      'fic_event_type' => 'What type of event',
      'fic_image_credit' => 'Image credit',
      'pageid' => 'Page ID'
    );

    $this->source = new MigrateSourceJSON('', 'pageid', $fields);

    $this->destination = new MigrateDestinationNode('timeline_event');

    $this->map = new MigrateSQLMap(
      $this->machineName,
      array(
        'pageid' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'Page ID',
        )
      ),
      MigrateDestinationNode::getKeySchema()
    );

    // Mapped fields
    $this->addFieldMapping('uid')
      ->defaultValue('1');

    $this->addFieldMapping('changed');

    $this->addFieldMapping('field_image_caption', 'fic_caption');
    $this->addFieldMapping('field_image_caption:format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('field_image_caption:language')
      ->defaultValue('en');
    $this->addFieldMapping('body', 'fic_event_body');
    $this->addFieldMapping('body:format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('field_date', 'fic_event_date');
    $this->addFieldMapping('field_date:to', 'fic_event_end_date');
    $this->addFieldMapping('field_date:timezone')
      ->defaultValue('America/New_York');
    $this->addFieldMapping('body:language')
      ->defaultValue('en');
    $this->addFieldMapping('status')
      ->defaultValue(1);
    $this->addFieldMapping('pathauto')
      ->defaultValue(0);
    $this->addFieldMapping('translate')
      ->defaultValue(0);
    $this->addFieldMapping('title', 'fic_event_headline');
    $this->addFieldMapping('field_image', 'fic_event_image')
      ->sourceMigration('NistImage');
    $this->addFieldMapping('field_image:file_class')
      ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_image:preserve_files')
      ->defaultValue(TRUE);
    $this->addFieldMapping('field_image:alt', 'fic_event_image_alt');
    $this->addFieldMapping('field_credit', 'fic_image_credit');
    $this->addFieldMapping('field_credit:language')
      ->defaultValue('en');

    // Unmapped destination fields
    $this->addUnmigratedDestinations(array('created', 'is_new',
      'revision_uid', 'promote', 'sticky', 'revision', 'log', 'language',
      'tnid', 'comment', 'body:summary', 'field_date:rrule', 'path',
      'field_image:title', 'field_image:language'));
  }

  /**
   * Implementation of Migration::prepareRow($row).
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) :
      return FALSE;
    endif;

    if (strlen($row->fic_event_date) == 4) {
      $row->fic_event_date = '01/01/' . $row->fic_event_date;
    }

    // add some time (one min) to the changed value so it is forced to be updated.
    $row->changed = $this->_get_updated_time_for_force_updating( $row->changed );
  }
}
