<?php
/**
 * @file
 * Implementation of migration to import Kaltura video files into
 * Drupal remote stream wrapped video files entities.
 */

class NistKalturaVideoMigration extends NistMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Import of NIST Kaltura video into Drupal video file entities.');
    $this->dependencies = array();

    $fields = array(
      'content_loc' => 'URL of the video file',
      'player_loc' => 'URL of the custom player that plays the video file',
      'thumbnail_loc' => 'URL of the thumbnail for the video',
      'title' => 'Title of the video',
      'description' => 'Description of the video',
      'view_count' => 'The amount of views of the video',
      'publication_date' => 'The publication date of the video',
      'tag' => 'Free tags on the video',
      'category' => 'Category (tag) of the video',
      'family_friendly' => 'If the video is family friendly',
      'duration' => 'Duration of the video',
      'id' => 'The entry ID for the video.'
    );

    $this->feed_url = "";
    
    //$this->feed_file = __DIR__.'/processed_kaltura_feed.json';

    // Was getting permission denied errors on the cloud servers because the FS isn't writeable
    // see https://docs.acquia.com/articles/file-permissions-while-using-acquia-cloud
    // So create a temp php file
    $this->feed_file = tempnam(sys_get_temp_dir(), 'processed_kaltura_feed');

    $this->source = new MigrateSourceJSON($this->feed_file, 'id', $fields);
    $this->destination = new MigrateDestinationMedia('video', 'MigrateFileUriAsIs');

    $this->map = new MigrateSQLMap(
      $this->machineName,
      array(
        'id' => array(
          'type' => 'varchar',
          'length' => 255,
          'not null' => TRUE,
          'description' => 'The entry ID.',
        )
      ),
      MigrateDestinationMedia::getKeySchema()
    );

    // Mapped fields
    $this->addFieldMapping('uid')
         ->defaultValue(1);

    $this->addFieldMapping('value', 'content_loc');
    $this->addFieldMapping('timestamp', 'publication_date');

    /*
    $this->addFieldMapping('path', '');
    $this->addFieldMapping('pathauto', '');

    $this->addFieldMapping('source_dir', '');
    $this->addFieldMapping('urlencode', '');
    */

    $this->addFieldMapping('field_video_title', 'migrate_file_title');

    $this->addFieldMapping('preserve_files')
         ->defaultValue(true);
    $this->addFieldMapping('file_replace')
         ->defaultValue(MigrateFile::FILE_EXISTS_REUSE);

    $this->addFieldMapping('field_video_image', 'thumbnail_loc');

    /*
    $this->addFieldMapping('field_video_image:file_class')
         ->defaultValue();
    */
    $this->addFieldMapping('field_video_image:preserve_files')
         ->defaultValue(TRUE);
    $this->addFieldMapping('field_video_image:destination_dir')
         ->defaultValue('public://images/kaltura_import');

    $this->addFieldMapping('field_video_image:destination_file', 'migrate_thumbnail_file');

    $this->addFieldMapping('field_video_image:file_replace')
         ->defaultValue(MigrateFile::FILE_EXISTS_REUSE);
    /*
    $this->addFieldMapping('field_video_image:source_dir')
         ->defaultValue();
    */
    /*
    $this->addFieldMapping('field_video_image:urlencode')
         ->defaultValue();
    */
    $this->addFieldMapping('field_video_image:alt', 'migrate_thumbnail_alt');
    $this->addFieldMapping('field_video_image:title', 'migrate_thumbnail_title');

    $this->addFieldMapping('field_video_player_location', 'player_loc');
    //$this->addFieldMapping('field_video_thumbnail_location', 'thumbnail_loc');
    $this->addFieldMapping('field_video_duration', 'duration');
    $this->addFieldMapping('field_video_view_count', 'view_count');

    $this->addFieldMapping('field_video_description', 'description');
    $this->addFieldMapping('field_video_description:format')
         ->defaultValue('filtered_html');

    $this->addFieldMapping('field_video_tags', 'tag')
         ->separator(',');
    $this->addFieldMapping('field_video_tags:create_term')
         ->defaultValue(TRUE);
    $this->addFieldMapping('field_video_tags:ignore_case')
         ->defaultValue(TRUE);

    $this->addFieldMapping('field_video_date', 'publication_date');
    /*
    $this->addFieldMapping('field_video_date:timezone', '');
    $this->addFieldMapping('field_video_date:rrule', '');
    $this->addFieldMapping('field_video_date:to', '');
    */
    
    $this->addUnmigratedDestinations(array(
      //'destination_file',
    ));

    $this->addUnmigratedSources(array(

    ));
  }

  /**
   * Implementation of Migration::prepareRow($row).
   */
  public function preImport() {
    parent::preImport();
    
    /**
     * Was having issues with the fact that some nodes were
     * namespaced while other weren't so do some preprocessing
     * in this function. Just convert and clean the kaltura XML
     * to JSON (using JSON allows us to extend the NIST base class)
     * 
     * ¯\_(ツ)_/¯
     */
    $feed = file_get_contents($this->feed_url);

    $xml_namespaces = ' xmlns="http://www.sitemaps.org/schemas/sitemap/0.9" xmlns:video="http://www.google.com/schemas/sitemap-video/1.1"';
    $loc_prefix = '<loc>http://nist.gov/?entryId=';
    $loc_suffix = '</loc>';
    $id_open_tag = '<id>';
    $id_close_tag = '</id>';
    $video_open_tag = '<video:video>';
    $video_close_tag = '</video:video>';
    $video_prefix = "video:";

    // Plain string parsing for perfomance reasons
    // (as opposed creating an object in memory, iterating and then serializing back to a string)

    // remove the top level namespaces
    $feed = str_replace($xml_namespaces, '', $feed);

    // convert the loc tag to a just an id
    $feed = str_replace($loc_prefix, $id_open_tag, $feed);
    $feed = str_replace($loc_suffix, $id_close_tag, $feed);

    // remove the <video:video> tag so all it's properties move
    // to the top level
    $feed = str_replace($video_open_tag, '', $feed);
    $feed = str_replace($video_close_tag, '', $feed);

    // remove the video name space prefix
    $feed = str_replace($video_prefix, '', $feed);

    $feed_xml = simplexml_load_string($feed);

    // If the feed doesn't parse, return early so the migrate can run off
    // the previously parsed file
    if ($feed_xml === FALSE) {
      return;
    }

    $feed_json = json_encode($feed_xml);
    $feed = json_decode($feed_json);

    // Remove the top level URL property so we just have a list of
    // videos at the top level
    $feed = $feed->url;

    foreach ($feed as $key => $video) {
      // Convert the tags array to comma separated string
      if (isset($video->tag) && is_array($video->tag)) {
        $feed[$key]->tag = implode(',', $video->tag);  
      }

      // Convert the category string to an array
      //$feed[$key]->category = explode(',', $video->category);

      // Convert the date to a timestamp
      $feed[$key]->publication_date = strtotime($video->publication_date);
    }

    $feed_json_string = json_encode($feed);

    // migrate assumes reading a file as opposed to a string in memory
    // so need to save it to the file system
    file_put_contents($this->feed_file, $feed_json_string); 
  }

  /**
   * Implementation of Migration::prepare
   */
  public function prepareRow($row) {
    $row->migrate_file_title = $row->title;

    // if the xml property is empty this parses as an object
    // so set it to empty string
    if (!is_string($row->description)) {
      $row->description = "";
    }

    // Remove random line breaks that are in the kaltura feed
    $row->description = str_replace(array("\r", "\n"), '', $row->description);

    $thumbnail_extension = ".jpeg";
    $title = $row->title . " Thumbnail";
    
    // create a lower case / underscore separated for file system
    $machine_title = strtolower($title . $thumbnail_extension);
    $machine_title = str_replace(" ", "_", $machine_title);

    $row->migrate_thumbnail_file = $machine_title;
    $row->migrate_thumbnail_alt = $title;
    $row->migrate_thumbnail_title = $title;
  }

  /**
   * Implementation of Migration::prepare
   */
  public function prepare($file, $row) {
    // Set the file name to the title from the feed instead of the filename from the URL
    $file->filename = $row->title;
  }

  /**
   * Implementation of Migration::complete
   */
  public function complete($file, $row) {
    $file = $this->setThumbnailFileFields($file, $row);
  }

  /**
   * Helper function to set the field api fields for the thumbnail 
   * image file entity that is attached to the video file file entity.
   * Have to do this somewhat manually since the thumbnail image is a
   * file in a field, not the file that is the destination of the
   * migration 
   */
  private function setThumbnailFileFields($file, $row) {
    $thumbnail_file_id = $file->field_video_image[LANGUAGE_NONE][0]['fid'];

    $thumbnail_file = file_load($thumbnail_file_id);

    $thumbnail_file->field_file_image_alt_text[LANGUAGE_NONE][0] = array(
      'value' => $row->migrate_thumbnail_alt,
      'format' => NULL,
      'safe_value' => $row->migrate_thumbnail_alt,
    );

    $thumbnail_file->field_file_title[LANGUAGE_NONE][0] = array(
      'value' => $row->migrate_thumbnail_title,
      'format' => NULL,
      'safe_value' => $row->migrate_thumbnail_title,
    );

    $thumbnail_file->field_copyright_protected[LANGUAGE_NONE][0] = array();

    $thumbnail_file->field_image_gallery[LANGUAGE_NONE][0] = array(
      'value' => 0
    );

    $thumbnail_file->field_file_image_title_text[LANGUAGE_NONE][0] = array(
      'value' => $row->migrate_thumbnail_title,
      'format' => NULL,
      'safe_value' => $row->migrate_thumbnail_title,
    );

    file_save($thumbnail_file);
  }
}
