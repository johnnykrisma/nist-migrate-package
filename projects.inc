<?php

/**
 * @file
 * Implementation of migration from Commonspot program custom element to Drupal program nodes.
 */

class NistProjectMigration extends NistMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Import of Commonspot programs custom element into event programs type');
    $this->dependencies = array(
      'NistUser',
      'NistImage',
      'NistPublication'
    );


    $fromDate = $this->getFromDate(1);
    $list_url = "";
    $item_url = "";


    $fields = array(
      'changed' => 'Date the page was last modified',
      'created' => 'Date the page was last created',
      'cs_uid' => 'Commonspot user that created the page',
      'contact_block' => 'Block containing the contact info',
      'fic_project_program_active' => 'Is this program active',
      'fic_project_program_additional_details' => 'Program/Project more details',
      'fic_project_program_associated_ou' => 'Program/Project associated OU',
      'fic_project_program_associated_product' => 'Program/Project associated products',
      'fic_project_program_campus' => 'Program/Project campus',
      'fic_project_program_campus_other' => '?',
      'fic_project_program_ccc_hidden' => '?',
      'fic_project_program_collaborators' => 'Collaborators',
      'fic_project_program_description' => 'Program/Project description',
      'fic_project_program_end_date' => 'Project end date',
      'fic_project_program_facilities' => 'Program/Project facilities',
      'fic_project_program_funding' => 'Program/Project funding',
      'fic_project_program_id' => 'Program/Project ID',
      'fic_project_program_lead_ou' => 'Program/Project lead OU',
      'fic_project_program_major_accomplishments' => 'Program/Project major accomplishments',
      'fic_project_program_name' => 'Name of the Program/Project',
      'fic_project_program_related_project' => 'Program/Project related projects',
      'fic_project_program_selected_publications' => 'Selected Pubs related to the Project',
      'fic_project_program_staff' => 'Staff on the Program/Project',
      'fic_project_program_start_date' => 'Program/Project start date',
      'fic_project_program_summary' => 'Summary of the Program/Project',
      'imagealt' => 'Alt text for specific Image element on page',
      'imageid' => 'ID of specific Image element on page',
      'owner_name' => 'Plaintext username of the owner of the page',
      'pageid' => 'CommonSpot PageID',
      'pagename' => 'Name of the page used in the url',
      'page_description' => 'Description of the page, often not used and same and title',
      'page_title' => 'Title of the page',
      'parsedimageids' => '',
      'path' => 'URL in the CommonSpot system possibly used for Pathauto purposes',
      'taxonomy_ids' => 'IDs of taxonomy terms from FLEET',
      'imagecaption' => 'I am the image caption entered by the user in the Image field.'
    );

    $this->highwaterField = array(
      'name' => 'changed', // Column to be used as highwater mark
      'alias' => 'w',
    );


    $this->source = new MigrateSourceList(
      new MigrateListJSON($list_url), 
      new MigrateItemJSON($item_url, array()), 
      $fields
    );


    $this->destination = new MigrateDestinationNode('project_program');

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
    $this->addFieldMapping('body', 'fic_project_program_description');
    $this->addFieldMapping('title', 'fic_project_program_name');
    $this->addFieldMapping('body:format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('body:language')
      ->defaultValue('en');
    $this->addFieldMapping('status')
      ->defaultValue(1);
    $this->addFieldMapping('translate')
      ->defaultValue(0);

    $this->addFieldMapping('field_nist_org', 'generatedOrg');

    $this->addFieldMapping('field_active', 'fic_project_program_active');

    $this->addFieldMapping('field_associated_product', 'fic_project_program_associated_product');
    $this->addFieldMapping('field_associated_product:language')
      ->defaultValue('en');
    $this->addFieldMapping('field_associated_product:format')
      ->defaultValue('filtered_html');

    $this->addFieldMapping('field_campus', 'fic_project_program_campus')
      ->separator(', ');
    $this->addFieldMapping('field_campus:ignore_case')
      ->defaultValue(TRUE);

    $this->addFieldMapping('field_contributors', 'fic_project_program_collaborators');
    $this->addFieldMapping('field_contributors:language')
      ->defaultValue('en');
    $this->addFieldMapping('field_contributors:format')
      ->defaultValue('filtered_html');


    // old
    $this->addFieldMapping('field_date', 'fic_project_program_start_date');

    // news
    $this->addFieldMapping('field_date_month', 'fic_project_program_start_date');


    $this->addFieldMapping('field_facilities', 'fic_project_program_facilities');
    $this->addFieldMapping('field_facilities:language')
      ->defaultValue('en');
    $this->addFieldMapping('field_facilities:format')
      ->defaultValue('filtered_html');

    // the IMAGE FIELD
    $this->addFieldMapping('field_image', 'imageid')
      ->separator(', ')
      ->sourceMigration('NistImage');
    $this->addFieldMapping('field_image:file_class')
      ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_image:preserve_files')
      ->defaultValue(TRUE);
    $this->addFieldMapping('field_image:alt', 'imagealt');
    $this->addFieldMapping('field_image:title', 'imagecaption');

    $this->addFieldMapping('field_legacy_contact', 'contact_block');
    $this->addFieldMapping('field_legacy_contact:language')
      ->defaultValue('en');
    $this->addFieldMapping('field_legacy_contact:format')
      ->defaultValue('filtered_html');

    $this->addFieldMapping('field_major_accomplishments', 'fic_project_program_major_accomplishments');
    $this->addFieldMapping('field_major_accomplishments:language')
      ->defaultValue('en');
    $this->addFieldMapping('field_major_accomplishments:format')
      ->defaultValue('filtered_html');


    $this->addFieldMapping('field_nist_topics', 'taxonomy_ids')
      ->separator(', ')
      ->sourceMigration('NistTerm');
    $this->addFieldMapping('field_nist_topics:source_type')
      ->defaultValue('tid');
    $this->addFieldMapping('field_nist_topics:create_term')
      ->defaultValue('False');
    $this->addFieldMapping('field_project_id', 'fic_project_program_id');
    $this->addFieldMapping('field_project_id:language')
      ->defaultValue('en');
    $this->addFieldMapping('field_related_projects_programs', 'fic_project_program_related_project');
    $this->addFieldMapping('field_related_projects_programs:language')
      ->defaultValue('en');
    $this->addFieldMapping('field_related_projects_programs:format')
      ->defaultValue('filtered_html');

    // selected publication
    $this->addFieldMapping('field_selected_publications_ref', 'fic_project_program_selected_publications')
      ->separator(', ')
      ->sourceMigration('NistPublication');


    $this->addFieldMapping('field_source_of_extra_funding', 'fic_project_program_funding');
    $this->addFieldMapping('field_source_of_extra_funding:language')
      ->defaultValue('en');
    $this->addFieldMapping('field_source_of_extra_funding:format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('field_staff', 'fic_project_program_staff');
    $this->addFieldMapping('field_staff:language')
      ->defaultValue('en');
    $this->addFieldMapping('field_staff:format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('field_summary', 'fic_project_program_summary');
    $this->addFieldMapping('field_summary:language')
      ->defaultValue('en');
    $this->addFieldMapping('field_summary:format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('field_technical_details', 'fic_project_program_additional_details');
    $this->addFieldMapping('field_technical_details:language')
      ->defaultValue('en');
    $this->addFieldMapping('field_technical_details:format')
      ->defaultValue('filtered_html');
    //$this->addFieldMapping('path', 'path');
    $this->addFieldMapping('pathauto')
      ->defaultValue(0);

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
      'field_date:timezone', 'field_date:rrule', 'field_campus:source_type',
      'field_campus:create_term', 'field_other_campus', 'field_other_campus:language',
      'field_nist_topics:ignore_case', 'field_contact', 'field_related_publication',
      'field_test_content', 'field_featured', 'field_related_project'
    ));




    // No unmapped source fields
    $this->addUnmigratedSources(array('pagename', 'page_description', 'owner_name',
      'fic_project_program_campus_other', 'page_title',
      'fic_project_program_ccc_hidden', 'fic_project_program_associated_ou'));

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

    // Translates CS Subsite to NIST organization
    $row->generatedOrg = $this->_get_org_chart_name($row->path);

    // add some time (one min) to the changed value so it is forced to be updated.
    $row->changed = $this->_get_updated_time_for_force_updating( $row->changed );

    // access rights taxonomy
    $theTID = $this->getPermissionTaxonomyDataBySubSiteURL( $this->getSubSiteFromPath($row->path) );
    $row->permissionTIDBasedOnSubSite = $theTID;

    $row->fic_project_program_lead_ou = $this->_process_ou($row->fic_project_program_lead_ou);

    $row->fic_project_program_campus = $this->_process_campus($row->fic_project_program_campus);

    $row->imageid = $row->imageid . ', ' . $row->parsedimageids;

    $row->fic_project_program_additional_details = $this->_process_body_text_native($row->fic_project_program_additional_details);
    $row->fic_project_program_associated_product = $this->_process_body_text_native($row->fic_project_program_associated_product);
    $row->fic_project_program_collaborators = $this->_process_body_text_native($row->fic_project_program_collaborators);
    $row->fic_project_program_description = $this->_process_body_text_native($row->fic_project_program_description);
    $row->fic_project_program_facilities = $this->_process_body_text_native($row->fic_project_program_facilities);
    $row->fic_project_program_funding = $this->_process_body_text_native($row->fic_project_program_funding);
    $row->fic_project_program_major_accomplishments = $this->_process_body_text_native($row->fic_project_program_major_accomplishments);
    $row->fic_project_program_related_project = $this->_process_body_text_native($row->fic_project_program_related_project);
    $row->fic_project_program_staff = $this->_process_body_text_native($row->fic_project_program_staff);
    $row->fic_project_program_summary = $this->_process_body_text_native($row->fic_project_program_summary);

    $row->path = $this->_process_path($row->path);

    // if we have an image caption set it to the file entity
    if (strlen($row->imagecaption) > 0) {
      $this->setImageCaption($row->imageid, $row->imagecaption);
    }
  }

  /**
   * I set an images caption field to the value from the feed.
   * @param $IDImage
   * @param $caption
   */
  public function setImageCaption($IDImage, $caption) {

    // kill any ', ' that snuck in there
    $IDImage = str_replace(', ', '', $IDImage);

    // get the fid from the commonspot image ID
    $query = db_select('migrate_map_nistimage', 'n')
      ->fields('n')
      ->condition('sourceid1', $IDImage, '=')
      ->execute()
      ->fetchAssoc();

    $IDImageNode = $query['destid1'];

    // load the file entity
    $imageObject = file_load($IDImageNode);

    // if the entity is valid and has the key set stuff and save.
    if (array_key_exists('field_image_caption', $imageObject)) {

      $this->drushLog('**********ADDING AN IMAGE CAPTION**********');

      // set the properties and save it
      $imageObject->field_image_caption['und'][0]['value'] = $caption;
      $imageObject->field_image_caption['und'][0]['safe_value'] = $caption;

      file_save($imageObject);
    }
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
    
    //$this->drushLog('ARE WE HERE?');

    // get the current persisted object so we can inspect it and check stuff.
    $currentNode = node_load($node->nid);

    //$selectedPubsToAddToRelatedPub = $currentNode->field_selected_publications_ref['und'];

    // keep shit the same and carry over from old node
    $node->field_related_publication = $currentNode->field_related_publication;
    $node->field_selected_publications_ref = $currentNode->field_selected_publications_ref;

    /*
    if( !isset($node->field_related_publication['und']) ){
      $node->field_related_publication['und'] = array();
    }
    */
    // append all the selected pubs to the related pubs field
    //array_push($node->field_related_publication['und'], $selectedPubsToAddToRelatedPub);
    
    //2830713 - selected
    //24698 - field_related_publication

    //print_r($node->field_related_publication);
    //print_r($node->field_related_publication['und']);
    //print_r($selectedPubsToAddToRelatedPub);
    //print_r($node->field_related_publication);
    //print_r($currentNode->field_selected_publications_ref);
    //print_r($node);
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
   * I am called by convention. I call the completCollbck
   * @param array $nids
   */
  public function bulkRollback(array $nids) {

    //$this->prepareRollback($nids, 'Projects');
  }

  /**
   * I am called by convention by the Migrate framework after an import has
   * run.
   */
  public function postImport() {

    $this->logMessage('Projects Migrated', 'project.log');
  }
}
