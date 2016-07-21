<?php

/**
 * @file
 * Implementation of migration from Commonspot event custom element to Drupal event nodes.
 */

class NistEventMigration extends NistMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Import of Commonspot Event custom element into event content type');
    $this->dependencies = array('NistUser');

    $fields = array(
      'changed' => 'Date the page was last modified',
      'contact_block' => 'contact block from CS page',
      'created' => 'Date the page was last created',
      'cs_uid' => 'Commonspot user that created the page',
      'fic_event_agenda' => 'Agenda of the event',
      'fic_event_audience' => 'Who is the audience',
      'fic_event_campus' => 'where is this event',
      'fic_event_contact_ou' => 'Contact OU',
      'fic_event_date' => 'Event Start Date',
      'fic_event_end_date' => 'Event End Date',
      'fic_event_event_url' => 'Event URL',
      'fic_event_format' => 'Format of the event',
      'fic_event_lodging' => 'Lodging information',
      'fic_event_name' => 'Event Name',
      'fic_event_online_registration_url' => 'The URL of the registration page',
      'fic_event_purpose' => 'The purpose of the event',
      'fic_event_registration_contact' => 'Registration contact',
      'fic_event_registration_information' => 'Registration information',
      'fic_event_related_project_name' => 'Related projects',
      'fic_event_security_instructions' => 'Event security instructions',
      'fic_event_speakers' => 'who\'s speaking',
      'fic_event_sponsor' => 'Sponsor of the event',
      'fic_event_technical_contact' => 'Technical contact for this event',
      'fic_event_testimonials' => 'testimonials about the event',
      'fic_event_thumbnail' => 'Thumbnail image in CPImage format',
      'fic_event_vendor_contact' => 'vendor contact',
      'fic_inactive' => 'Is the event active or inactive?',
      'fic_news_item_associated_campus' => 'associated campus',
      'owner_name' => 'Plaintext username of the owner of the page',
      'page_description' => 'Description of the page, often not used and same and title',
      'page_title' => 'Title of the page',
      'pageid' => 'CommonSpot PageID',
      'pagename' => 'Name of the page used in the url',
      'parsedimageids' => 'All images parsed out of page text',
      'path' => 'URL in the CommonSpot system possibly used for Pathauto purposes',
      'taxonomy_ids' => 'IDs of taxonomy terms from FLEET',
    );

    $this->highwaterField = array(
      'name' => 'changed', // Column to be used as highwater mark
      'alias' => 'w',
    );

    /* older pull from JSON */
    $form_id = "12836";
    $from_date = $this->getFromDate(2);
    // $theURL = "http://spin1-www.nist.gov/com/DataPull.cfc?method=getCEList&formid={$form_id}&fromDate={$from_date}&returnFormat=json";
    // $this->source = new MigrateSourceJSON($theURL, 'pageid', $fields);

    //http://www.nist.gov/com/DataPull.cfc?method=getCEList&formid=12836&fromDate=2000-01-01&returnFormat=json


    // new way using list of ids.
    $list_url = "";
    $item_url = "";


    $this->source = new MigrateSourceList(
      new MigrateListJSON($list_url),
      new MigrateItemJSON($item_url, array()),
      $fields
    );


    $this->destination = new MigrateDestinationNode('event');

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
    $this->addFieldMapping('changed', 'changed');

    $this->addFieldMapping('created', 'created');

    $this->addFieldMapping('uid', 'cs_uid')
         ->sourceMigration('NistUser');

    $this->addFieldMapping('title', 'fic_event_name');

    $this->addFieldMapping('body', 'fic_event_purpose');
    $this->addFieldMapping('body:format')
         ->defaultValue('filtered_html');
    $this->addFieldMapping('body:language')
         ->defaultValue('en');

    $this->addFieldMapping('status')
         ->defaultValue(1);

    $this->addFieldMapping('field_active', 'fic_inactive');

    $this->addFieldMapping('field_legacy_url', 'legacyPath');
    $this->addFieldMapping('field_legacy_url:language')
         ->defaultValue('en');


    $this->addFieldMapping('field_legacy_owner_name', 'owner_name');
    $this->addFieldMapping('field_legacy_owner_name:language')
         ->defaultValue('en');

    $this->addFieldMapping('translate')
         ->defaultValue(0);


    $this->addFieldMapping('field_nist_org', 'generatedOrg');

    $this->addFieldMapping('field_nist_topics', 'taxonomy_ids')
         ->separator(', ')
         ->sourceMigration('NistTerm');
    $this->addFieldMapping('field_nist_topics:source_type')
         ->defaultValue('tid');
    $this->addFieldMapping('field_nist_topics:create_term')
         ->defaultValue('False');

    $this->addFieldMapping('field_agenda', 'fic_event_agenda');
    $this->addFieldMapping('field_agenda:format')
         ->defaultValue('filtered_html');
    $this->addFieldMapping('field_agenda:language')
         ->defaultValue('en');
    $this->addFieldMapping('field_associated_ou_raw', 'fic_event_contact_ou');
    $this->addFieldMapping('field_associated_ou_raw:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_event_audience', 'fic_event_audience')
      ->separator(', ');

    $this->addFieldMapping('field_date', 'zeroedOutEventDate');
    $this->addFieldMapping('field_date:to', 'zeroedOutEventEndDate');

    $this->addFieldMapping('field_url', 'fic_event_event_url');
    $this->addFieldMapping('field_url:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_event_format', 'fic_event_format');
    $this->addFieldMapping('field_event_format:create_term')
         ->defaultValue('True');
    $this->addFieldMapping('field_event_format:ignore_case')
         ->defaultValue('True');

    $this->addFieldMapping('field_image_branding', 'fic_event_thumbnail')
      ->sourceMigration('NistImage');
    $this->addFieldMapping('field_image_branding:file_class')
      ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_image_branding:preserve_files')
      ->defaultValue(TRUE);
    $this->addFieldMapping('field_image_branding:language')
      ->defaultValue('en');
    $this->addFieldMapping('field_image_branding:alt', 'migrate_event_thumbnail_alt');
    $this->addFieldMapping('field_image_branding:title', 'migrate_event_thumbnail_alt');//

    $this->addFieldMapping('field_location', 'fic_event_campus');
    $this->addFieldMapping('field_location:language')
      ->defaultValue('en');
    $this->addFieldMapping('field_lodging_info', 'fic_event_lodging');
    $this->addFieldMapping('field_lodging_info:format')
         ->defaultValue('filtered_html');
    $this->addFieldMapping('field_lodging_info:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_registration_url', 'fic_event_online_registration_url');
    $this->addFieldMapping('field_registration_url:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_legacy_reg_contact', 'fic_event_registration_contact');
    $this->addFieldMapping('field_legacy_reg_contact:format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('field_legacy_reg_contact:language')
      ->defaultValue('en');

    $this->addFieldMapping('field_registration_info', 'fic_event_registration_information');
    $this->addFieldMapping('field_registration_info:format')
         ->defaultValue('filtered_html');
    $this->addFieldMapping('field_registration_info:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_legacy_project', 'fic_event_related_project_name');
    $this->addFieldMapping('field_legacy_project:format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('field_legacy_project:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_security_instructions', 'fic_event_security_instructions');
    $this->addFieldMapping('field_security_instructions:format')
         ->defaultValue('filtered_html');
    $this->addFieldMapping('field_security_instructions:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_speakers', 'fic_event_speakers');
    $this->addFieldMapping('field_speakers:format')
         ->defaultValue('filtered_html');
    $this->addFieldMapping('field_speakers:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_sponsors', 'fic_event_sponsor');
    $this->addFieldMapping('field_sponsors:format')
         ->defaultValue('filtered_html');
    $this->addFieldMapping('field_sponsors:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_legacy_technical_contact', 'fic_event_technical_contact');
    $this->addFieldMapping('field_legacy_technical_contact:format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('field_legacy_technical_contact:language')
      ->defaultValue('en');

    $this->addFieldMapping('field_testimonials', 'fic_event_testimonials');
    $this->addFieldMapping('field_testimonials:format')
         ->defaultValue('filtered_html');
    $this->addFieldMapping('field_testimonials:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_vendor_contact', 'fic_event_vendor_contact');
    $this->addFieldMapping('field_vendor_contact:format')
         ->defaultValue('filtered_html');
    $this->addFieldMapping('field_vendor_contact:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_campus', 'fic_news_item_associated_campus')
      ->separator(', ');

    $this->addFieldMapping('field_image', 'parsedimageids')
      ->separator(', ')
      ->sourceMigration('NistImage');
    $this->addFieldMapping('field_image:file_class')
      ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_image:preserve_files')
      ->defaultValue(TRUE);

    // field_access_rights
    $this->addFieldMapping('field_access_rights', 'permissionTIDBasedOnSubSite')
      ->separator(', ');
    $this->addFieldMapping('field_access_rights:source_type')
      ->defaultValue('tid');
    $this->addFieldMapping('field_access_rights:create_term')
      ->defaultValue('False');

    // NEW field_nist_topic_areas
    $this->addFieldMapping('field_nist_topic_areas', 'newTaxonomyTermList')
      ->separator(', ');
    $this->addFieldMapping('field_nist_topic_areas:source_type')
      ->defaultValue('tid');
    $this->addFieldMapping('field_nist_topic_areas:create_term')
      ->defaultValue('False');

    // Unmapped destination fields
    $this->addUnmigratedDestinations(array('is_new', 'revision_uid', 'promote',
      'sticky', 'revision', 'log', 'language', 'tnid', 'comment', 'body:summary',
      'field_nist_topics:ignore_case', 'field_event_format:source_type', 'field_date:rrule',
      'field_date:timezone', 'field_url:title', 'field_url:attributes',
      'field_registration_url:title', 'field_registration_url:attributes', 'field_campus:source_type',
      'field_campus:create_term', 'field_campus:ignore_case', 'field_image:language',
      'field_presentation_file', 'field_presentation_file:file_class',
      'field_presentation_file:language', 'field_presentation_file:preserve_files',
      'field_presentation_file:destination_dir', 'field_presentation_file:destination_file',
      'field_presentation_file:file_replace', 'field_presentation_file:source_dir',
      'field_presentation_file:urlencode', 'field_presentation_file:description',
      'field_presentation_file:display', 'field_document_agenda',
      'field_document_agenda:file_class', 'field_document_agenda:language',
      'field_document_agenda:preserve_files', 'field_document_agenda:destination_dir',
      'field_document_agenda:destination_file', 'field_document_agenda:file_replace',
      'field_document_agenda:source_dir', 'field_document_agenda:urlencode',
      'field_document_agenda:description', 'field_document_agenda:display', 'field_featured',
      'field_featured_homepage', 'field_pba_approval', 'field_rss', 'field_related_news',
      'field_related_event', 'field_related_instrument_tool', 'field_related_publication',
      'field_related_project', 'field_technical_contact', 'field_registration_contact',
      'field_nist_org:source_type', 'field_nist_org:create_term', 'field_nist_org:ignore_case'));


    // No unmapped source fields
    $this->addUnmigratedSources(array('pagename', 'page_title', 'page_description',
      'contact_block'));
  }

  /**
   * Implementation of Migration::prepareRow($row).
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) :
      return FALSE;
    endif;

    // get the new Taxonomy terms based on current Commonspot taxonomy ids
    $row->newTaxonomyTermList = $this->getNewMappedTaxonomyTermsByOldIDList($row->taxonomy_ids);

    $row->cs_uid = $this->_translate_user_id($row->cs_uid);

    $row->legacyPath = $row->path;

    // Translates CS Subsite to NIST organization
    $row->generatedOrg = $this->_get_org_chart_name($row->path);

    // add some time (one min) to the changed value so it is forced to be updated.
    $row->changed = $this->_get_updated_time_for_force_updating( $row->changed );

    $row->fic_news_item_associated_campus = $this->_process_campus($row->fic_news_item_associated_campus);

    $row->fic_event_contact_ou = $this->_process_ou($row->fic_event_contact_ou);

    // access rights taxonomy
    $theTID = $this->getPermissionTaxonomyDataBySubSiteURL( $this->getSubSiteFromPath($row->path) );
    $row->permissionTIDBasedOnSubSite = $theTID;

    $row->fic_event_lodging = $this->_process_body_text_native($row->fic_event_lodging);
    $row->fic_event_purpose = $this->_process_body_text_native($row->fic_event_purpose);
    $row->fic_event_registration_contact = $this->_process_body_text_native($row->fic_event_registration_contact);
    $row->fic_event_registration_information = $this->_process_body_text_native($row->fic_event_registration_information);
    $row->fic_event_related_project_name = $this->_process_body_text_native($row->fic_event_related_project_name);
    $row->fic_event_speakers = $this->_process_body_text_native($row->fic_event_speakers);
    $row->fic_event_sponsor = $this->_process_body_text_native($row->fic_event_sponsor);
    $row->fic_event_technical_contact = $this->_process_body_text_native($row->fic_event_technical_contact);
    $row->fic_event_testimonials = $this->_process_body_text_native($row->fic_event_testimonials);

    $row->zeroedOutEventDate = $this->zeroOutDateTime($row->fic_event_date);
    $row->zeroedOutEventEndDate = $this->zeroOutDateTime($row->fic_event_end_date);

    // Extract useful info from the CPImage format
    if (!empty($row->fic_event_thumbnail)) :
      // Splits the CPImage:{ID} from the alt text
      $image_array = explode('|', $row->fic_event_thumbnail);
      // Splits the CPImage: from the {ID}
      $image_id_array = explode(':', $image_array[0]);

      $row->fic_event_thumbnail = $image_id_array[1];
      $row->migrate_event_thumbnail_alt = $image_array[1];
    endif;
  }

  /**
   * I am called by convention by the Migrate framework
   * @param $node
   * @param \stdClass $row
   */
  public function prepare($node, stdClass $row) {
    // sometimes the checkbox for pathauto isn't checked by default. Lets go ahead
    // and force it.
    $node->path['pathauto'] = TRUE;

    $this->globalPrepare($node);
  }

  /**
   * I am called by convention by the Migrate framework after an import has
   * run.
   */
  public function postImport() {

    $this->logMessage('Event Migrated', 'event.log');
  }

  
  /**
   * I am called by convention by the Migrate framework
   * @param $node
   * @param \stdClass $row
   */
  public function complete($node, stdClass $row) {

    $this->addRedirect($node, $row);
    $this->addWorkBenchModerationNodeHistoryEntry($node);
  }

  /**
   * I am called by convention. I call the completRollbck
   * @param array $nids
   */
  public function bulkRollback(array $nids) {

    //$this->prepareRollback($nids, 'Award');
  }
}
