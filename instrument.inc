<?php

/**
 * @file
 * Implementation of migration from Commonspot Award custom element to Drupal award nodes.
 */

class NistInstrumentMigration extends NistMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Import of Commonspot instrument custom element into instrument content type');
    $this->dependencies = array(
      'NistUser',
      'NistImage',
      'NistPublication'
    );

    $fields = array(
      'changed' => 'Date the page was last modified',
      'contact_block' => '',
      'created' => 'Date the page was last created',
      'cs_uid' => 'Commonnspot user that created the page',
      'fic_facility' => ' ',
      'fic_facility_user_agreement_or_crada' => ' ',
      'fic_instrument_access' => 'how to access this instrument',
      'fic_instrument_application' => 'applications of the instrument',
      'fic_instrument_campus' => 'campus where the instrument is',
      'fic_instrument_capabilities' => 'the capabilities of the instrument',
      'fic_instrument_contact1_email' => 'contact email',
      'fic_instrument_contact1_fax' => 'contact fax',
      'fic_instrument_contact1_name' => 'contact name',
      'fic_instrument_contact1_phone' => 'contact phone',
      'fic_instrument_contact2_email' => 'contact email',
      'fic_instrument_contact2_fax' => 'contact fax',
      'fic_instrument_contact2_name' => 'contact name',
      'fic_instrument_contact2_phone' => 'contact phone',
      'fic_instrument_contact3_email' => 'contact email',
      'fic_instrument_contact3_fax' => 'contact fax',
      'fic_instrument_contact3_name' => 'contact name',
      'fic_instrument_contact3_phone' => 'contact phone',
      'fic_instrument_contact4_email' => 'contact email',
      'fic_instrument_contact4_fax' => 'contact fax',
      'fic_instrument_contact4_name' => 'contact name',
      'fic_instrument_contact4_phone' => 'contact phone',
      'fic_instrument_contact_address' => 'Contact address',
      'fic_instrument_contact_email' => 'contact email',
      'fic_instrument_contact_fax' => 'contact fax',
      'fic_instrument_contact_name' => 'contact name',
      'fic_instrument_contact_phone' => 'contact phone',
      'fic_instrument_description' => 'Description of the instrument',
      'fic_instrument_name' => 'Name of Instrument',
      'fic_instrument_ou' => 'Associated OU',
      'fic_instrument_project' => 'associated project',
      'fic_instrument_proprietary' => 'is instrument proprietary',
      'fic_instrument_schedule' => 'schedule for the instrument',
      'fic_instrument_selected_publications' => 'publications related to the instrument',
      'imagealt' => '',
      'imageid' => '',
      'owner_name' => 'Plaintext username of the owner of the page',
      'page_description' => 'Description of the page, often not used and same and title',
      'page_title' => 'Title of the page',
      'pageid' => 'CommonSpot PageID',
      'pagename' => 'Name of the page used in the url',
      'parsedimageids' => '',
      'path' => 'URL in the CommonSpot system possibly used for Pathauto purposes',
      'taxonomy_ids' => 'IDs of taxonomy terms from FLEET',
    );

    $this->highwaterField = array(
      'name' => 'changed', // Column to be used as highwater mark
      'alias' => 'w',         
    );

    $form_id = "15531";
    $from_date = $this->getFromDate(2);
    $theURL = "";
    $this->source = new MigrateSourceJSON($theURL, 'pageid', $fields);

    $this->destination = new MigrateDestinationNode('instrument');

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

    $this->addFieldMapping('title', 'fic_instrument_name');

    $this->addFieldMapping('body', 'fic_instrument_description');
    $this->addFieldMapping('body:format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('body:language')
      ->defaultValue('en');

    $this->addFieldMapping('status')
      ->defaultValue(1);

    $this->addFieldMapping('pathauto')
      ->defaultValue(0);

    $this->addFieldMapping('translate')
      ->defaultValue(0);

    $this->addFieldMapping('field_nist_topics', 'taxonomy_ids')
      ->separator(',')
      ->sourceMigration('NistTerm');
    $this->addFieldMapping('field_nist_topics:source_type')
      ->defaultValue('tid');
    $this->addFieldMapping('field_nist_topics:create_term')
      ->defaultValue('False');

    $this->addFieldMapping('field_access_information', 'fic_instrument_access');
    $this->addFieldMapping('field_access_information:format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('field_access_information:language')
      ->defaultValue('en');

    $this->addFieldMapping('field_applications', 'fic_instrument_application');
    $this->addFieldMapping('field_applications:format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('field_applications:language')
      ->defaultValue('en');
    $this->addFieldMapping('field_associated_projects', 'fic_instrument_project');
    $this->addFieldMapping('field_associated_projects:format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('field_associated_projects:language')
      ->defaultValue('en');
    $this->addFieldMapping('field_campus', 'fic_instrument_campus');
    $this->addFieldMapping('field_legacy_contact', 'contact_block');
    $this->addFieldMapping('field_legacy_contact:format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('field_legacy_contact:language')
      ->defaultValue('en');
    $this->addFieldMapping('field_operating_schedule', 'fic_instrument_schedule');
    $this->addFieldMapping('field_operating_schedule:format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('field_operating_schedule:language')
      ->defaultValue('en');

    $this->addFieldMapping('field_proprietary', 'fic_instrument_proprietary');

    $this->addFieldMapping('field_related_publication', 'fic_instrument_selected_publications')
      ->separator(',')
      ->sourceMigration('NistPublication');

    $this->addFieldMapping('field_specs_capabilities', 'fic_instrument_capabilities');
    $this->addFieldMapping('field_specs_capabilities:format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('field_specs_capabilities:language')
      ->defaultValue('en');

    $this->addFieldMapping('field_image', 'imageid')
      ->separator(',')
      ->sourceMigration('NistImage');
    $this->addFieldMapping('field_image:file_class')
      ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_image:preserve_files')
      ->defaultValue(TRUE);

    $this->addFieldMapping('field_legacy_url', 'legacyPath');
    $this->addFieldMapping('field_legacy_url:language')
      ->defaultValue('en');

    $this->addFieldMapping('field_nist_org', 'generatedOrg');

    $this->addFieldMapping('field_instrument_or_tool_select')
      ->defaultValue('instrument');

    $this->addFieldMapping('field_legacy_owner_name', 'owner_name');

    $this->addFieldMapping('field_facility', 'fic_facility');

    $this->addFieldMapping('field_agreement_type', 'fic_facility_user_agreement_or_crada');

    // field_access_rights
    $this->addFieldMapping('field_access_rights', 'permissionTIDBasedOnSubSite')
      ->separator(',');
    $this->addFieldMapping('field_access_rights:source_type')
      ->defaultValue('tid');
    $this->addFieldMapping('field_access_rights:create_term')
      ->defaultValue('False');

    // NEW field_nist_topic_areas
    $this->addFieldMapping('field_nist_topic_areas', 'newTaxonomyTermList')
      ->separator(',');
    $this->addFieldMapping('field_nist_topic_areas:source_type')
      ->defaultValue('tid');
    $this->addFieldMapping('field_nist_topic_areas:create_term')
      ->defaultValue('False');

    // Unmapped destination fields
    $this->addUnmigratedDestinations(array('is_new', 'revision_uid', 'promote',
      'sticky', 'revision', 'log', 'language', 'tnid', 'comment',
      'field_nist_topics:ignore_case', 'body:summary', 'field_image:language',
      'field_campus:source_type', 'field_campus:create_term', 'field_campus:ignore_case',
      'field_image:alt', 'field_image:title', 'path',
      'field_test_content'
    ));

    // No unmapped source fields
    $this->addUnmigratedSources(array('pagename', 'page_description', 'page_title',
      'imagealt', 'parsedimageids'
    ));
  }

  /**
   * Implementation of Migration::prepareRow($row).
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) :
      return FALSE;
    endif;

    $row->legacyPath = $row->path;

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

    $row->fic_instrument_campus = $this->_process_campus($row->fic_instrument_campus);

    $row->fic_instrument_ou = $this->_process_ou($row->fic_instrument_ou);

    $row->imageid = $row->imageid . ',' . $row->parsedimageids;

    $row->fic_facility_user_agreement_or_crada = $this->process_agreement($row->fic_facility_user_agreement_or_crada);
    $row->fic_instrument_proprietary = $this->process_prop($row->fic_instrument_proprietary);
    $row->fic_instrument_access = $this->_process_body_text_native($row->fic_instrument_access);
    $row->fic_instrument_application = $this->_process_body_text_native($row->fic_instrument_application);
    $row->fic_instrument_capabilities = $this->_process_body_text_native($row->fic_instrument_capabilities);
    $row->fic_instrument_description = $this->_process_body_text_native($row->fic_instrument_description);
    $row->fic_instrument_project = $this->_process_body_text_native($row->fic_instrument_project);
    $row->fic_instrument_schedule = $this->_process_body_text_native($row->fic_instrument_schedule);
    $row->contact_block = $this->assemble_contact_block($row);
  }

  public function clean_contact_address($address) {
    // this was added into the content, let's remove it to put it on the template.
    $string_to_kill = '<i><b>Disclaimer:</b>Any mention of commercial products within NIST web pages is for information only; it does not imply recommendation or endorsement by NIST.</i>';
    $clean_string = str_replace($string_to_kill, '', $address);
    //various cleanup sbased on Commonspot ridiculosity
    $clean_string = str_replace('Gaithersburg', '<br>Gaithersburg', $clean_string);
    $clean_string = str_replace('Boulder', '<br>Boulder', $clean_string);
    $clean_string = str_replace('Division100', 'Division<br>100 Bureau Drive', $clean_string);
    $clean_string = str_replace('NISTFire', 'NIST<br>Fire', $clean_string);
    $clean_string = str_replace('NISTBuilding', 'NIST<br>Building', $clean_string);

    return $clean_string;
  }

  public function process_prop($value) {
    switch ($value) {
      case 'Collaborative':
        $prop = 'collaborative';
        break;
      case 'Non-proprietary':
        $prop = 'nonproprietary';
        break;
      case 'Proprietary':
        $prop = 'proprietary';
        break;
       default:
         $prop = '';
    }
    return $prop;
  }

  public function process_agreement($value) {
    switch ($value) {
      case 'Facility User Agreement':
        $agreement = 'facility_user_agreement';
        break;
      case 'CRADA':
        $agreement = 'crada';
        break;
      default:
        $agreement = '';
    }
    return $agreement;
  }

  public function assemble_contact_block($row) {
    $contact_block = '';

    if (strlen($row->fic_instrument_contact_name)) {
      $contact_block .= '<strong>Name:</strong>' . $row->fic_instrument_contact_name . '<br>';
    }
    if (strlen($row->fic_instrument_contact_phone)) {
      $contact_block .= '<strong>Phone:</strong>' . $row->fic_instrument_contact_phone . '<br>';
    }
    if (strlen($row->fic_instrument_contact_fax)) {
      $contact_block .= '<strong>Fax:</strong>' . $row->fic_instrument_contact_fax . '<br>';
    }
    if (strlen($row->fic_instrument_contact_email)) {
      $contact_block .= '<strong>Email:</strong> <a href="mailto:' . $row->fic_instrument_contact_email . '">' . $row->fic_instrument_contact_email . '</a><br>';
    }
    if (strlen($row->fic_instrument_contact_address)) {
      $contact_block .= '<strong> Address:</strong><br>' . $this->clean_contact_address($row->fic_instrument_contact_address);
    }
    if (strlen($row->fic_instrument_contact1_name)) {
      $contact_block .= '<hr><strong>Name:</strong>' . $row->fic_instrument_contact1_name . '<br>';
    }
    if (strlen($row->fic_instrument_contact1_phone)) {
      $contact_block .= '<strong>Phone:</strong>' . $row->fic_instrument_contact1_phone . '<br>';
    }
    if (strlen($row->fic_instrument_contact1_fax)) {
      $contact_block .= '<strong>Fax:</strong>' . $row->fic_instrument_contact1_fax . '<br>';
    }
    if (strlen($row->fic_instrument_contact1_email)) {
      $contact_block .= '<strong>Email:</strong> <a href="mailto:' . $row->fic_instrument_contact1_email . '">' . $row->fic_instrument_contact1_email . '</a>';
    }
    if (strlen($row->fic_instrument_contact2_name)) {
      $contact_block .= '<hr><strong>Name:</strong>' . $row->fic_instrument_contact2_name . '<br>';
    }
    if (strlen($row->fic_instrument_contact2_phone)) {
      $contact_block .= '<strong>Phone:</strong>' . $row->fic_instrument_contact2_phone . '<br>';
    }
    if (strlen($row->fic_instrument_contact2_fax)) {
      $contact_block .= '<strong>Fax:</strong>' . $row->fic_instrument_contact2_fax . '<br>';
    }
    if (strlen($row->fic_instrument_contact2_email)) {
      $contact_block .= '<strong>Email:</strong> <a href="mailto:' . $row->fic_instrument_contact2_email . '">' . $row->fic_instrument_contact2_email . '</a>';
    }
    if (strlen($row->fic_instrument_contact3_name)) {
      $contact_block .= '<hr><strong>Name:</strong>' . $row->fic_instrument_contact3_name . '<br>';
    }
    if (strlen($row->fic_instrument_contact3_phone)) {
      $contact_block .= '<strong>Phone:</strong>' . $row->fic_instrument_contact3_phone . '<br>';
    }
    if (strlen($row->fic_instrument_contact3_fax)) {
      $contact_block .= '<strong>Fax:</strong>' . $row->fic_instrument_contact3_fax . '<br>';
    }
    if (strlen($row->fic_instrument_contact3_email)) {
      $contact_block .= '<strong>Email:</strong> <a href="mailto:' . $row->fic_instrument_contact3_email . '">' . $row->fic_instrument_contact3_email . '</a>';
    }
    if (strlen($row->fic_instrument_contact4_name)) {
      $contact_block .= '<hr><strong>Name:</strong>' . $row->fic_instrument_contact4_name . '<br>';
    }
    if (strlen($row->fic_instrument_contact4_phone)) {
      $contact_block .= '<strong>Phone:</strong>' . $row->fic_instrument_contact4_phone . '<br>';
    }
    if (strlen($row->fic_instrument_contact4_fax)) {
      $contact_block .= '<strong>Fax:</strong>' . $row->fic_instrument_contact4_fax . '<br>';
    }
    if (strlen($row->fic_instrument_contact4_email)) {
      $contact_block .= '<strong>Email:</strong> <a href="mailto:' . $row->fic_instrument_contact4_email . '">' . $row->fic_instrument_contact4_email . '</a>';
    }

    return $contact_block;

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

    //$this->prepareRollback($nids, 'Instrument');
  }

  /**
   * I am called by convention by the Migrate framework after an import has
   * run.
   */
  public function postImport() {

    $this->logMessage('Instrument Migrated', 'instrument.log');
  }
}
