<?php

/**
 * @file
 * Import of Commonspot pages that don't use custom elements into Drupal's
 * Basic Page content type.
 */

class NistCommonspotPagesMigration extends NistMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Import of Commonspot pages that don\'t use custom
      elements into Drupal\'s Basic Page content type');
    $this->dependencies = array('NistUser');

    // create a date that is 2 days older than now
    $fromdate = $this->getFromDate(2);
    
    $list_url = "";
    $item_url = "";
    
    $fields = array(
      'body' => '',
      'changed' => 'Date the page was last modified',
      'created' => 'Date the page was last created',
      'cs_uid' => 'Commonspot user that created the page',
      'id' => 'CommonSpot PageID',
      'owner_name' => 'Plaintext username of the owner of the page',
      'page_description' => 'Description of the page, often same as title',
      'page_title' => 'Title of the page',
      'pagename' => 'Name of the page used in the url',
      'pagetype' => 'This equates to the layout of these pages',
      'pageurl' => 'URL of the page',
      'parsedimageids' => 'IMages parsed from the body',
      'path' => 'URL in CommonSpot possibly used for Pathauto purposes',
      'sidebar' => 'content that was in the sidebar (usually left)',
      'sidebar2' => 'content that was in the other sidebar (usually right)',
      'taxonomy_ids' => 'IDs of taxonomy terms from FLEET'
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
    
    $this->destination = new MigrateDestinationNode('page');

    $this->map = new MigrateSQLMap(
      $this->machineName,
      array(
        'id' => array(
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

    $this->addFieldMapping('body', 'body');
    $this->addFieldMapping('body:format')
         ->defaultValue('filtered_html');
    $this->addFieldMapping('body:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_image', 'parsedimageids')
      ->separator(', ')
      ->sourceMigration('NistImage');
    $this->addFieldMapping('field_image:file_class')
      ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_image:preserve_files')
      ->defaultValue(TRUE);

    $this->addFieldMapping('field_nist_topics', 'taxonomy_ids')
         ->separator(', ')
         ->sourceMigration('NistTerm');
    $this->addFieldMapping('field_nist_topics:source_type')
         ->defaultValue('tid');
    $this->addFieldMapping('field_nist_topics:create_term')
         ->defaultValue('False');

    $this->addFieldMapping('field_legacy_layout', 'pagetype');
    $this->addFieldMapping('field_legacy_layout:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_legacy_owner_name', 'owner_name');
    $this->addFieldMapping('field_legacy_owner_name:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_legacy_url', 'legacyPath');
    $this->addFieldMapping('field_legacy_url:language')
         ->defaultValue('en');

    $this->addFieldMapping('field_legacy_sidebar_1', 'sideBar1Processed');
    $this->addFieldMapping('field_legacy_sidebar_1:format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('field_legacy_sidebar_1:language')
      ->defaultValue('en');

    $this->addFieldMapping('field_legacy_sidebar_2', 'sideBar2Processed');
    $this->addFieldMapping('field_legacy_sidebar_2:format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('field_legacy_sidebar_2:language')
      ->defaultValue('en');

    $this->addFieldMapping('field_nist_org', 'generatedOrg');

    $this->addFieldMapping('field_test_content')
      ->defaultValue('0');

    $this->addFieldMapping('pathauto')
         ->defaultValue(0);

    $this->addFieldMapping('status')
         ->defaultValue(1);

    $this->addFieldMapping('translate')
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

    // sections for pages
    $this->addFieldMapping('field_section', 'generatedSectionFromPath');


    // don't want to add the path anymore. borks menus
    // path
    //$this->addFieldMapping('path', 'path');


    // Unmapped destination fields
    $this->addUnmigratedDestinations(array('is_new', 'revision_uid', 'promote',
      'sticky', 'revision', 'log', 'language', 'tnid', 'comment', 'body:summary',
      'field_nist_topics:ignore_case', 'field_campus', 'field_campus:source_type',
      'field_campus:ignore_case', 'field_campus:create_term', 'field_nist_org:create_term',
      'field_nist_org:source_type', 'field_nist_org:ignore_case'));

    // Unmapped source fields
    $this->addUnmigratedSources(array('pagename', 'page_description', 'pageurl'));
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

    // Fixes userID. translates it from Commonspot to CPR ID.
    $row->cs_uid = $this->_translate_user_id($row->cs_uid);

    $row->legacyPath = $row->path;

    // Fix path to CS path without the .cfm
    $row->path = $this->_process_path( $row->path );

    // Translates CS Subsite to NIST organization
    $row->generatedOrg = $this->_get_org_chart_name( $row->path );

    // add some time (one min) to the changed value so it is forced to be updated.
    $row->changed = $this->_get_updated_time_for_force_updating( $row->changed );

    // access rights taxonomy
    $theTID = $this->getPermissionTaxonomyDataBySubSiteURL( $this->getSubSiteFromPath($row->path) );
    $row->permissionTIDBasedOnSubSite = $theTID;

    // section is based on taxonomy tid
    $row->generatedSectionFromPath = $this->getGeneratedSectionFromPath($theTID);

    $row->body = $this->_process_body_text_native($row->body, 'commonspotpage-migrate-image');
    $row->sideBar1Processed = $this->_process_body_text_native($row->sidebar, 'commonspotpage-migrate-image');
    $row->sideBareProcessed = $this->_process_body_text_native($row->sidebar2, 'commonspotpage-migrate-image');

  }

  /**
   * I am called by convention by the Migrate framework after an import has
   * run.
   */
  public function postImport() {

    $this->logMessage('CommonSpotPages Migrated', 'commonspotpages.log');
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

    // =========================================================================
    // SEE blankpages.inc THIS CODE IS A COPY AND PAST!!!!!!!

    // This is kinda breaking with Migrate tradition mention in the
    // documentation, but there are several people that like this approach and I
    // think it offers a little more control.
    // What were ARE doing: https://www.drupal.org/node/2550939
    // What were NOT doing: https://www.drupal.org/node/1117454

    // we don't REALLY have to implement a whole NEW  set of migrates because
    // what we have is already working pretty well. We DO want to be able to
    // control what might get into the object that is going to be saved.

    // by getting the persisted object from the db we have MAXIMUM POWER about
    // what's currently there and what were going to replace it with. I like this
    // approach better then creating new scripts and having a new sytsemOfRecord

    // get the current persisted object so we can inspect it and check stuff.
    $currentNode = node_load($node->nid);

    // we don't want to over write a blank pages section so see if the persisted
    // object has one. if it does set the new objects to the olds...
    if (isset($currentNode->field_section['und'][0]['target_id'])) {
      $currentSectionTID = $currentNode->field_section['und'][0]['target_id'];
      $node->field_section['und'][0]['target_id'] = $currentSectionTID;
    }
    // =========================================================================

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

    //$this->prepareRollback($nids, 'Award');
  }
}
