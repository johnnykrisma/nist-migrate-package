<?php
/**
 * @file
 * Implementation of migration to import CommonSpot uploaded files into Drupal Files.
 */

class NistPubFileMigration extends NistMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    
    $this->description = t('Import of NIKE Pub files into Drupal files.');

    $list_url = '';
    $item_url = '';


    $fields = array(
      'pub_id' => 'Name of the file',
      'fileurl' => 'File url',
    );

    $this->source = new MigrateSourceList(
      new MigrateListJSON($list_url), 
      new MigrateItemJSON($item_url, array()), 
      $fields
    );

    $this->destination = new MigrateDestinationFile();

    $this->map = new MigrateSQLMap(
      $this->machineName,
      array(
        'pub_id' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'Page ID',
        )
      ),
      MigrateDestinationNode::getKeySchema()
    );

    // Mapped fields
    $this->addFieldMapping('value', 'Url');
    $this->addFieldMapping('destination_dir', 'dir');
    $this->addFieldMapping('preserve_files')
         ->defaultValue(true);
    $this->addFieldMapping('file_replace')
         ->defaultValue(MigrateFile::FILE_EXISTS_REUSE);
    $this->addFieldMapping('uid', 'uid')
         ->sourceMigration('NistUser');
    $this->addFieldMapping('timestamp', 'changed');

    $this->addUnmigratedDestinations(array(
      'destination_file',
      'fid',
      'source_dir',
      'path',
      'urlencode'
    ));
    
    $this->addUnmigratedSources(array(
      /*
      'id',
      'unixpath',
      'imageurl',
      'creatorid',
      'imagedescription',
      'doctype',
      'filename',
      'ispublic',
      'subsitedir',
      'subsiteid',
      'origheight',
      'origwidth',
      'filesize',
      'category',
      'taxonomy_ids'
      */
    ));
  }

  /**
   * Implementation of Migration::prepareRow($row).
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) :
      return FALSE;
    endif;

    //$row->title = str_replace('_', ' ', $row->title);

    $row->dir = rtrim($row->dir, '/');
    $row->dir = "public://documents{$row->dir}";
  }
}
