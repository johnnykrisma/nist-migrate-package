<?php
/**
 * @file
 * This migrates user photos
 *
 */
class NistUserImageMigration extends NistMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->description = t('Import of user images');

    $fields = array(
      'changed' => 'Date the page was last modified',
      'id' => 'Image ID',
      'authorimageurl' => 'CP Image ID'
    );

    $this->highwaterField = array(
      'name' => 'changed', // Column to be used as highwater mark
      'alias' => 'w',
    );

    $json_file = '';

    $this->source = new MigrateSourceList(new PageListJSON($json_file),
      new PageItemJSON($json_file, array()), $fields);

    $this->destination = new MigrateDestinationFile();

    $this->map = new MigrateSQLMap($this->machineName,
        array('id' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'ImageID',
          )
        ),
      MigrateDestinationFile::getKeySchema()
    );

    $this->addFieldMapping('value', 'authorimageurl');
    $this->addFieldMapping('preserve_files')
         ->defaultValue(true);
    $this->addFieldMapping('file_replace')
         ->defaultValue(MigrateFile::FILE_EXISTS_REUSE);
    $this->addFieldMapping('destination_dir')
         ->defaultValue('public://userphotos');

    $this->addUnmigratedDestinations(array('uid', 'timestamp', 'path',
      'destination_file', 'source_dir', 'urlencode'
    ));
  }


  /**
   * Implementation of Migration::prepareRow($row).
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) :
      return FALSE;
    endif;

    $this->drushLog($row->authorimageurl);

    // add some time (one min) to the changed value so it is forced to be updated.
    //$row->changed = $this->_get_updated_time_for_force_updating( $formattedDate );

  }
}
