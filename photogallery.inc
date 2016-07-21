<?php
/**
 * @file
 *
 */
class NistPhotoGalleryMigration extends NistMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->description = t('Import of NIST Photo Gallery images');

    $fields = array(
      'author' => 'This is the credit line from the image gallery',
      'avnumber' => 'internal reference ID',
      'copyright' => 'this is a boolean indicating whether or not the image is copyrighted',
      'description' => 'Image description',
      'fileformat' => 'File format fo the image.',
      'imagedate' => 'date image was taken',
      'imageid' => 'Image ID',
      'imagesource' => 'originating organization',
      'imagetype' => 'what kind of image is it',
      'imageurl' => 'URL on source site',
      'moddate' => 'Date image was modified',
      'subjects' => 'people tagged in the images',
      'title' => 'The Title of the Image',
      'topics' => 'keywords from the gallery topic list'
      /*
       * FROM FEED
      ** author
      ** avnumber
      ** copyright
      ** description
      editorid
      ** file150 NOT USED
      ** file300 NOT USED
      ** file72 NOT USED
      ** fileformat
      ** imagedate
      ** imageid
      ** imagesource
      ** imagetype
      ** imageurl
      ** internalonly NOT USED, WE'RE ONLY PULLING THINGS ALLOWED ON EXT
      ** moddate
      ** subjects
      ** title
      ** topics
      */
      // NOT USING'filename' => 'Filename from Commonspot',
      // NOT USING 'ownerid' => 'Source owner ID',
      // NOT USING 'subsiteurl' => 'URL on source site (minus filename)',
      // NOT USING 'taxonomy_ids' => 'Taxonomy ids',
    );

    $this->highwaterField = array(
      'name' => 'moddate', // Column to be used as highwater mark
      'alias' => 'i',
    );

    $this->source = new MigrateSourceJSON('', 'imageid', $fields);

    $this->destination = new MigrateDestinationFile('image');
    $this->map = new MigrateSQLMap($this->machineName,
      array('imageid' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'ImageID',
      )
      ),
      MigrateDestinationFile::getKeySchema()
    );

    //$this->addFieldMapping('source_dir', 'imageurl');
    $this->addFieldMapping('value', 'imageurl');
    $this->addFieldMapping('destination_dir', 'public://');
    $this->addFieldMapping('destination_file', 'generatedFileName');
    $this->addFieldMapping('preserve_files')
      ->defaultValue(true);
    $this->addFieldMapping('file_replace')
      ->defaultValue(MigrateFile::FILE_EXISTS_REUSE);

    $this->addFieldMapping('filename', 'title');

    $this->addFieldMapping('timestamp', 'moddate');

    $this->addFieldMapping('field_credit', 'author');
    $this->addFieldMapping('field_av_number', 'avnumber');
    $this->addFieldMapping('field_image_gallery')
      ->defaultValue(true);

    $this->addFieldMapping('field_copyright_protected', 'copyright');
    $this->addFieldMapping('field_image_caption', 'description');
    $this->addFieldMapping('field_date', 'imagedate');
    
    /*
    Going to revisit this when we get a mapping from the weird old
    Tag to the new gloabal taxonomy.
    $this->addFieldMapping('field_keyword_tags', 'topics')
      ->separator('/');
    $this->addFieldMapping('field_keyword_tags:source_type')
      ->defaultValue('term');
    $this->addFieldMapping('field_keyword_tags:create_term')
      ->defaultValue(TRUE);
    */

    $this->addFieldMapping('field_image_source‎', 'imagesource');
    $this->addFieldMapping('field_subjects', 'subjects');

    // THIS WILL NEED A MAPPING FROM THE OLD SYSTEM
    $this->addFieldMapping('field_image_format', 'generatedImageType');


    $this->addUnmigratedSources(array(
      'imagedescription'
    ));

    $this->addUnmigratedDestinations(array(
      'fid', 'source_dir', 'path', 'urlencode'
    ));

    $this->removeFieldMapping('pathauto');
  }

  /**
   * Implementation of Migration::prepareRow
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) :
      return FALSE;
    endif;

    $row->generatedImageType = $this->getImageTypeFromMap($row->imagetype);

    // replcae strings and lower case the file name
    $newImageFileName = preg_replace('/[^a-z0-9]+/', '_', strtolower($row->title)) . '.' . $row->fileformat;
    $row->generatedFileName = '/images/photogallery/' . $newImageFileName;

    $row->theImageURL = trim($row->imageurl);

    /*
    drush_log($row->generatedImageType, $type = 'warning', $error = null);
    drush_log($row->theImageURL, $type = 'warning', $error = null);
    drush_log($row->generatedFileName, $type = 'warning', $error = null);
    drush_log($row->topics, $type = 'warning', $error = null);
    drush_log($row->subjects, $type = 'warning', $error = null);
    */
  }


}