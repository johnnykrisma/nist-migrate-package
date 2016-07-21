<?php

/**
 * @file
 * Implementation of migration from Commonspot Award custom element to Drupal award nodes.
 */

class NistAwardMigration extends NistMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Import of Commonspot Award custom element into award content type');
    $this->dependencies = array('NistUser');

    $fields = array(
      'changed' => 'Date the page was last modified',
      'created' => 'Date the page was last created',
      'cs_uid' => 'Commonspot user that created the page',
      'fic_award_award_name' => 'Name of the award',
      'fic_award_awarding_organization' => 'Organization making the Award',
      'fic_award_awarding_organization_contact_name' => ' ',
      'fic_award_campus' => 'Campus the award is associated with',
      'fic_award_citation' => 'uhhh...',
      'fic_award_date' => 'Date of the award',
      'fic_award_description' => 'Description of the award',
      'fic_award_organization_phone' => 'phone number of the awarding organization',
      'fic_award_organization_url' => 'web URL of the awarding organization',
      'fic_award_winner_name' => 'Who won?',
      'fic_award_winner_ou' => 'Who won\'s OU',
      'fic_contact_information' => 'Contact text block',
      'imagealt' => 'Alt text for the Award headshot',
      'imageid' => 'Image ID of the Award headshot',
      'owner_name' => 'Plaintext username of the owner of the page',
      'page_description' => 'Description of the page, often not used and same and title',
      'page_title' => 'Title of the page',
      'pageid' => 'CommonSpot PageID',
      'pagename' => 'Name of the page used in the url',
      'parsedimageids' => 'Image ids of any images parsed from body text',
      'path' => 'URL in the CommonSpot system possibly used for Pathauto purposes',
      'taxonomy_ids' => 'IDs of taxonomy terms from FLEET'
    );
    
    $this->highwaterField = array(
      'name' => 'changed'
    );

    // create a date that is 2 days older than now
    $fromdate = $this->getFromDate(2);

    $url = '{SEE EXAMPLEJSON/AWARDS}';

    $this->source = new MigrateSourceJSON($url, 'pageid', $fields);

    $this->destination = new MigrateDestinationNode('award');

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

    $this->addFieldMapping('title', 'page_title');

    $this->addFieldMapping('field_award_name', 'fic_award_award_name');
    $this->addFieldMapping('field_award_name:language')
         ->defaultValue('en');

    $this->addFieldMapping('body', 'fic_award_description');
    $this->addFieldMapping('body:language')
         ->defaultValue('en');
    $this->addFieldMapping('body:format')
         ->defaultValue('filtered_html');

    $this->addFieldMapping('field_awarding_organization', 'fic_award_awarding_organization');
    $this->addFieldMapping('field_awarding_organization:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_contact_name', 'fic_award_awarding_organization_contact_name');
    $this->addFieldMapping('field_contact_name:language')
         ->defaultValue('en');

    $this->addFieldMapping('status')
         ->defaultValue(1);

    $this->addFieldMapping('field_campus', 'fic_award_campus');
    $this->addFieldMapping('field_campus:create_term')
         ->defaultValue('True');

    $this->addFieldMapping('field_citation', 'fic_award_citation');
    $this->addFieldMapping('field_citation:language')
         ->defaultValue('en');
    $this->addFieldMapping('field_citation:format')
         ->defaultValue('filtered_html');

    $this->addFieldMapping('field_winners_name', 'fic_award_winner_name');
    $this->addFieldMapping('field_winners_name:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_date', 'fic_award_date');

    $this->addFieldMapping('field_awarding_org_phone', 'fic_award_organization_phone');
    $this->addFieldMapping('field_awarding_org_phone:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_url', 'fic_award_organization_url');
    $this->addFieldMapping('field_url:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_winners_ou_raw', 'fic_award_winner_ou')
         ->issueGroup(t('Issues'))
         ->description(t('Does the OU reference the winner\'s current OU or the OU at the time of the award? can OU point to user entity?'))
         ->issuePriority(MigrateFieldMapping::ISSUE_PRIORITY_MEDIUM)
         ->issueNumber(21);
    $this->addFieldMapping('field_winners_ou_raw:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_legacy_url', 'legacyPath');
    $this->addFieldMapping('field_legacy_url:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_legacy_contact', 'fic_contact_information');
    $this->addFieldMapping('field_legacy_contact:format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('field_legacy_contact:language')
      ->defaultValue('en');

    $this->addFieldMapping('field_image', 'imageid')
      ->sourceMigration('NistImage');
    $this->addFieldMapping('field_image:file_class')
      ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_image:preserve_files')
      ->defaultValue(TRUE);
    $this->addFieldMapping('field_image:alt', 'imagealt');

    //$this->addFieldMapping('path', 'path');
    $this->addFieldMapping('pathauto')
         ->defaultValue(1);

    // field_legacy_owner_name
    $this->addFieldMapping('field_legacy_owner_name', 'owner_name');
    $this->addFieldMapping('field_legacy_owner_name:language')
         ->defaultValue('en');

    $this->addFieldMapping('translate')
         ->defaultValue(0);

    // field_nist_topics
    $this->addFieldMapping('field_nist_topics', 'taxonomy_ids')
         ->separator(', ')
         ->sourceMigration('NistTerm');
    $this->addFieldMapping('field_nist_topics:source_type')
         ->defaultValue('tid');
    $this->addFieldMapping('field_nist_topics:create_term')
         ->defaultValue('False');

    // NEW field_nist_topic_areas
    $this->addFieldMapping('field_nist_topic_areas', 'newTaxonomyTermList')
      ->separator(', ');
    $this->addFieldMapping('field_nist_topic_areas:source_type')
      ->defaultValue('tid');
    $this->addFieldMapping('field_nist_topic_areas:create_term')
      ->defaultValue('False');

    // field_access_rights
    $this->addFieldMapping('field_access_rights', 'permissionTIDBasedOnSubSite')
      ->separator(', ');
    $this->addFieldMapping('field_access_rights:source_type')
      ->defaultValue('tid');
    $this->addFieldMapping('field_access_rights:create_term')
      ->defaultValue('False');

    $this->addFieldMapping('field_nist_org', 'generatedOrg');


    //$this->addFieldMapping('workbench_access')->defaultValue('calibrations');


    // Unmapped destination fields
    $this->addUnmigratedDestinations(array('is_new', 'revision_uid', 'promote',
      'sticky', 'revision', 'log', 'language', 'tnid', 'comment', 'body:summary', 
      'field_nist_topics:ignore_case', 'field_campus:source_type', 'field_campus:ignore_case',
      'field_date:timezone', 'field_date:rrule', 'field_date:to',
      'field_url:title', 'field_url:attributes'));

    // Unmapped source fields
    $this->addUnmigratedSources(array('pagename', 'page_description'));
  }

  /**
   * Implementation of Migration::prepareRow($row).
   */
  public function prepareRow( $row ) {
    if (parent::prepareRow($row) === FALSE) :
      return FALSE;
    endif;

    $row->legacyPath = $row->path;

    // get the new Taxonomy terms based on current Commonspot taxonomy ids
    $row->newTaxonomyTermList = $this->getNewMappedTaxonomyTermsByOldIDList($row->taxonomy_ids);

    // Translates CS Subsite to NIST organization
    $row->generatedOrg = $this->_get_org_chart_name( $row->path );

    // add some time (one min) to the changed value so it is forced to be updated.
    $row->changed = $this->_get_updated_time_for_force_updating( $row->changed );

    // access rights taxonomy
    $theTID = $this->getPermissionTaxonomyDataBySubSiteURL( $this->getSubSiteFromPath($row->path) );
    $row->permissionTIDBasedOnSubSite = $theTID;

    $row->cs_uid = $this->_translate_user_id($row->cs_uid);
    $row->fic_award_citation = $this->_process_body_text_native($row->fic_award_citation, 'award-migrate-image');
    $row->fic_award_description = $this->_process_body_text_native($row->fic_award_description, 'award-migrate-image');

    //drush_log($this->getSubSiteFromPath($row->path), $type = 'warning', $error = null);
  }


  /**
   * I am called by convention by the Migrate framework
   * @param $node
   * @param \stdClass $row
   */
  public function prepare( $node, stdClass $row ) {
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

    $this->logMessage('Awards Migrated', 'award.log');
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

  public function bulkRollback(array $nids) {


    drush_log('SHOUDL SEE HERE*********', $type = 'warning', $error = null);
    drush_log(implode($nids), $type = 'warning', $error = null);

    //$this->prepareRollback($nids, 'Award');
  }
   */
}
