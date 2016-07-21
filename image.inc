<?php
/**
 * @file
 * Implementation of migration from Commonspot images to Drupal images.
 */
class NistImageMigration extends NistMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->description = t('Import of Commonspot images');
    $this->dependencies = array('NistUser');

    // test id 3240277

    // create a date that is x days older than now
    $fromdate = $this->getFromDate( 2 );

    $list_url = "";
    //$list_url = "";
    $item_url = "";

    $fields = array(
      'created' => 'Date image was created',
      'filename' => 'Filename from Commonspot',
      'id' => 'Image ID',
      'imagedescription' => 'Image description',
      'imageurl' => 'URL on source site',
      'ownerid' => 'Source owner ID',
      'subsiteurl' => 'URL on source site (minus filename)',
      'taxonomy_ids' => 'Taxonomy ids',
    );
    
    $this->highwaterField = array(
      'name' => 'created', // Column to be used as highwater mark
      'alias' => 'i',
    );

    $this->source = new MigrateSourceList(
      new MigrateListJSON($list_url),
      new MigrateItemJSON($item_url, array()),
      $fields
    );

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

    $this->addFieldMapping('value', 'imageurl');
    $this->addFieldMapping('destination_dir', 'subsiteurl');
    $this->addFieldMapping('destination_file', 'filename');
    $this->addFieldMapping('preserve_files')
         ->defaultValue(true);
    $this->addFieldMapping('file_replace')
         ->defaultValue(MigrateFile::FILE_EXISTS_REUSE);
    $this->addFieldMapping('uid', 'ownerid')
         ->sourceMigration('NistUser');
    $this->addFieldMapping('timestamp', 'created');

    // NEW field_nist_topic_areas
    $this->addFieldMapping('field_nist_topic_areas', 'newTaxonomyTermList')
      ->separator(', ');
    $this->addFieldMapping('field_nist_topic_areas:source_type')
      ->defaultValue('tid');
    $this->addFieldMapping('field_nist_topic_areas:create_term')
      ->defaultValue('False');


    $this->addUnmigratedDestinations(array(
      'fid', 'source_dir', 'path', 'urlencode'
    ));
    
    $this->addUnmigratedSources(array());

    $this->removeFieldMapping('pathauto');
  }

  /**
   * Implementation of Migration::prepareRow
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) :
      return FALSE;
    endif;

    // get the new Taxonomy terms based on current Commonspot taxonomy ids
    $row->newTaxonomyTermList = $this->getNewMappedTaxonomyTermsByOldIDList($row->taxonomy_ids);

    $row->filename = strtolower($row->filename);
    $row->subsiteurl = "public://images{$row->subsiteurl}";
    $row->subsiteurl = rtrim($row->subsiteurl, '/');

    $theURI = $row->subsiteurl . '/' . $row->filename;
    $this->drushLog( $theURI );

    // get the fid based on the URI
    $theQuery = db_query("select fid from file_managed where uri = '{$theURI}'");
    $result = $theQuery->fetchAll();

    // if there is a result get it from media...
    if ( count($result) === 1 ) {

      $fid = $result[0]->fid;
      $this->drushLog($fid);
      $fileEntity = file_load($fid);

      // if the commonspot description is under 255.
      if (strlen($row->imagedescription) < 255) {

        $fileEntity->alt = $row->imagedescription;

        // set some default stuff for media tokens
        if (!isset($fileEntity->field_file_image_alt_text->und)) {
          $fileEntity->field_file_image_alt_text = array(
            'und' => array(
              array(
                'value' => $row->imagedescription,
                'format' => NULL,
                'save_value' => htmlspecialchars($row->imagedescription)
              )
            )
          );
        }
      }
      else { // its over 255... truncate and populate
        $truncatedDescription = mb_strimwidth($row->imagedescription, 0, 251, "...");

        if (!isset($fileEntity->field_file_image_alt_text->und)) {
          $fileEntity->field_file_image_alt_text = array(
            'und' => array(
              array(
                'value' => $truncatedDescription,
                'format' => NULL,
                'save_value' => htmlspecialchars($truncatedDescription)
              )
            )
          );
        }
      }

      // populate the Image Caption if its over 64 k. We can guess that the
      // user on the CommonSpot side was really putting in a caption.
      if (strlen($row->imagedescription) > 64) {

        // always populate the description..
        if (! isset($fileEntity->field_image_caption->und)) {
          $fileEntity->field_image_caption = array(
            'und' => array(
              array(
                'value' => $row->imagedescription,
                'format' => NULL,
                'save_value' => htmlspecialchars($row->imagedescription)
              )
            )
          );
        }
      }

      // ALWAYS populate the description..
      if (! isset($fileEntity->field_description->und)) {
        $fileEntity->field_description = array(
          'und' => array(
            array(
              'value' => $row->imagedescription,
              'format' => NULL,
              'save_value' => htmlspecialchars($row->imagedescription)
            )
          )
        );
      }

      file_save($fileEntity);
    }
  }

  /**
   * I am called by convention by the Migrate framework after an import has
   * run.
   */
  public function postImport() {

    $this->logMessage('Image Migrated', 'image.log');
  }
}