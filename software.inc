<?php

/**
 * @file
 * Implementation of migration from Commonspot program custom element to Drupal program nodes.
 */

class NistSoftwareMigration extends NistMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Import of Commonspot Software custom element into Software type');
    $this->dependencies = array('NistUser');

    $fields = array(
      'changed' => 'Date the page was last modified',
      'created' => 'Date the page was last created',
      'cs_uid' => 'Commonspot user that created the page',
      'contact_block' => ' ',
      'fic_software_associated_ou' => ' ',
      'fic_software_authors' => ' ',
      'fic_software_description' => ' ',
      'fic_software_documentation' => ' ',
      'fic_software_download' => ' ',
      'fic_software_licensing_info' => ' ',
      'fic_software_name' => ' ',
      'fic_software_platform_requirements' => ' ',
      'fic_software_references' => ' ',
      'fic_software_selected_publications' => ' ',
      'fic_software_shots' => '',
      'fic_software_type' => ' ',
      'fic_software_updated' => ' ',
      'fic_software_uses' => ' ',
      'fic_software_version' => ' ',
      'owner_name' => 'Plaintext username of the owner of the page',
      'page_description' => 'Description of the page, often not used and same and title',
      'page_title' => 'Title of the page',
      'pageid' => 'CommonSpot PageID',
      'pagename' => 'Name of the page used in the url',
      'parsedimageids' => 'Image IDs parsed from the various fields of the page',
      'path' => 'URL in the CommonSpot system possibly used for Pathauto purposes',
      'taxonomy_ids' => 'IDs of taxonomy terms from FLEET'
    );

    $this->highwaterField = array(
      'name' => 'changed', // Column to be used as highwater mark
      'alias' => 'w',         
    );

    $fromDate = $this->getFromDate(2);
    $theURL = "";
    $this->source = new MigrateSourceJSON($theURL, 'pageid', $fields);

    $this->destination = new MigrateDestinationNode('software');

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

    $this->addFieldMapping('title', 'fic_software_name');

    $this->addFieldMapping('body', 'fic_software_description');
    $this->addFieldMapping('body:format')
         ->defaultValue('filtered_html');
    $this->addFieldMapping('body:language')
         ->defaultValue('en');

    $this->addFieldMapping('status')
         ->defaultValue(1);

    $this->addFieldMapping('field_legacy_url', 'legacyPath');
    $this->addFieldMapping('field_legacy_url:language')
         ->defaultValue('en');

    //$this->addFieldMapping('path', 'path');

    //$this->addFieldMapping('pathauto')->defaultValue(0);

    $this->addFieldMapping('field_legacy_owner_name', 'owner_name');
    $this->addFieldMapping('field_legacy_owner_name:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_documentation', 'fic_software_documentation');
    $this->addFieldMapping('field_documentation:language')
         ->defaultValue('en');
    $this->addFieldMapping('field_documentation:format')
         ->defaultValue('filtered_html');

    $this->addFieldMapping('field_download_information', 'fic_software_download');
    $this->addFieldMapping('field_download_information:language')
         ->defaultValue('en');
    $this->addFieldMapping('field_download_information:format')
         ->defaultValue('filtered_html');

    $this->addFieldMapping('field_credits', 'fic_software_references');
    $this->addFieldMapping('field_credits:language')
         ->defaultValue('en');
    $this->addFieldMapping('field_credits:format')
         ->defaultValue('filtered_html');

    $this->addFieldMapping('field_last_updated', 'fic_software_updated');
    $this->addFieldMapping('field_last_updated:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_licensing_information', 'fic_software_licensing_info');
    $this->addFieldMapping('field_licensing_information:language')
         ->defaultValue('en');
    $this->addFieldMapping('field_licensing_information:format')
         ->defaultValue('filtered_html');

    $this->addFieldMapping('field_related_publication', 'fic_software_selected_publications')
      ->separator(', ')
      ->sourceMigration('NistPublication');

    $this->addFieldMapping('field_selected_publications_:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_software_author', 'fic_software_authors');
    $this->addFieldMapping('field_software_author:language')
         ->defaultValue('en');
    $this->addFieldMapping('field_software_author:format')
         ->defaultValue('filtered_html');

    $this->addFieldMapping('field_software_type', 'fic_software_type');
    $this->addFieldMapping('field_software_type:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_software_version', 'fic_software_version');
    $this->addFieldMapping('field_software_version:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_system_requiremen', 'fic_software_platform_requirements');
    $this->addFieldMapping('field_system_requiremen:format')
         ->defaultValue('filtered_html');
    $this->addFieldMapping('field_system_requiremen:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_uses', 'fic_software_uses');
    $this->addFieldMapping('field_uses:language')
         ->defaultValue('en');
    $this->addFieldMapping('field_uses:format')
         ->defaultValue('filtered_html');

    $this->addFieldMapping('field_lead_ou_raw', 'fic_software_associated_ou');
    $this->addFieldMapping('field_lead_ou_raw:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_legacy_image_info', 'fic_software_shots')
         ->issueGroup(t('Issues'))
         ->description(t('Problematic field. Ostensibly it\'s an image, but it\'s presented as a textarea'))
         ->issuePriority(MigrateFieldMapping::ISSUE_PRIORITY_MEDIUM)
         ->issueNumber(770064);
    $this->addFieldMapping('field_legacy_image_info:language')
         ->defaultValue('en');
    $this->addFieldMapping('field_legacy_image_info:format')
         ->defaultValue('filtered_html');

    $this->addFieldMapping('translate')
         ->defaultValue(0);

    $this->addFieldMapping('field_nist_topics', 'taxonomy_ids')
         ->separator(', ')
         ->sourceMigration('NistTerm');
    $this->addFieldMapping('field_nist_topics:source_type')
         ->defaultValue('tid');
    $this->addFieldMapping('field_nist_topics:create_term')
         ->defaultValue('False');

    $this->addFieldMapping('field_legacy_contact', 'contact_block');

    $this->addFieldMapping('field_image', 'parsedimageids')
      ->separator(', ')
      ->sourceMigration('NistImage');
    $this->addFieldMapping('field_image:file_class')
      ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_image:preserve_files')
      ->defaultValue(TRUE);

    $this->addFieldMapping('field_nist_org', 'generatedOrg');

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
      'sticky', 'revision', 'log', 'language', 'tnid', 'comment',
      'body:summary', 'field_nist_topics:ignore_case'));

    // No unmapped source fields
    $this->addUnmigratedSources(array('pagename', 'page_description', 'page_title'));
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

    //$row->path = $this->_process_path($row->path);

    // Translates CS Subsite to NIST organization
    $row->generatedOrg = $this->_get_org_chart_name($row->path);

    // access rights taxonomy
    $theTID = $this->getPermissionTaxonomyDataBySubSiteURL( $this->getSubSiteFromPath($row->path) );
    $row->permissionTIDBasedOnSubSite = $theTID;

    $row->fic_software_authors = $this->_process_body_text_native($row->fic_software_authors);
    $row->fic_software_description = $this->_process_body_text_native($row->fic_software_description);
    $row->fic_software_documentation = $this->_process_body_text_native($row->fic_software_documentation);
    $row->fic_software_download = $this->_process_body_text_native($row->fic_software_download);
    $row->fic_software_licensing_info = $this->_process_body_text_native($row->fic_software_licensing_info);
    $row->fic_software_platform_requirements = $this->_process_body_text_native($row->fic_software_platform_requirements);
    $row->fic_software_references = $this->_process_body_text_native($row->fic_software_references);
    $row->fic_software_shots = $this->_process_body_text_native($row->fic_software_shots);
    $row->fic_software_uses = $this->_process_body_text_native($row->fic_software_uses);
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

    //$this->prepareRollback($nids, 'Software');
  }

  /**
   * I am called by convention by the Migrate framework after an import has
   * run.
   */
  public function postImport() {

    $this->logMessage('Software Migrated', 'software.log');
  }
}
