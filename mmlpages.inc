<?php

/**
 * @file
 * Implementation of migration from MM blank page custom elements to
 * Drupal Basic Page nodes.
 */

class NistMmlPagesMigration extends NistMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Import of MML blank pages content types into Drupal\'s Basic Page content type');
    $this->dependencies = array('NistUser');


    $fields = array(

      'changed' => 'Date the page was last modified',
      'created' => 'Date the page was last created',
      'cs_uid' => 'Commonspot user that created the page',

      //'fic_blank_page_column_amout' => 'meta info about the number of columns layout of the page',

      //'fic_blank_page_content' => 'big nasty blob of HTML that is the content of the page',
      'fic_msel_text' => 'big nasty blob of HTML that is the content of the page',

      //'fic_contact_information' => 'Contact block from the page',
      'contact_block' => 'Contact block from the page',

      'owner_name' => 'Plaintext username of the owner of the page',

      'page_description' => 'Description of the page, often not used and same and title',

      'page_title' => 'Title of the page',
      'pageid' => 'CommonSpot PageID',
      'pagename' => 'Name of the page used in the url',
      'parsedimageids' => 'IDs of any images inside the page.',

      'path' => 'URL in the CommonSpot system possibly used for Pathauto purposes',

      //'sidebar' => 'DOM scrape of content I can\'t pull as structured in the sidebar',
      //'sidebar2' => 'the other sidebar of three column pages, see above',
      'taxonomy_ids' => 'IDs of taxonomy terms from FLEET',
    );


    $this->highwaterField = array(
      'name' => 'changed', // Column to be used as highwater mark
      'alias' => 'w',
    );

    /*
    $this->source = new MigrateSourceList(
      new MigrateListJSON($list_url),
      new MigrateItemJSON($item_url, array()),
      $fields
    );
    */

    $fromDate = $this->getFromDate(5);
    $theURL = "";

    $this->source = new MigrateSourceJSON($theURL, 'pageid', $fields);

    //$this->destination = new MigrateDestinationNode('award');

    $this->destination = new MigrateDestinationNode('page');

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

    //$this->addFieldMapping('body', 'fic_blank_page_content');
    $this->addFieldMapping('body', 'fic_msel_text');
    $this->addFieldMapping('body:format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('body:language')->defaultValue('en');

    $this->addFieldMapping('changed', 'changed');
    $this->addFieldMapping('created', 'created');

    $this->addFieldMapping('field_image', 'parsedimageids')
      ->separator(',')
      ->sourceMigration('NistImage');
    $this->addFieldMapping('field_image:file_class')
      ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_image:preserve_files')
      ->defaultValue(TRUE);

    $this->addFieldMapping('field_legacy_contact', 'contact_block');
    $this->addFieldMapping('field_legacy_contact:format')
      ->defaultValue('filtered_html');

    $this->addFieldMapping('field_legacy_owner_name', 'owner_name');
    $this->addFieldMapping('field_legacy_owner_name:language')
      ->defaultValue('en');

    /*
    $this->addFieldMapping('field_legacy_sidebar_1', 'sidebar');
    $this->addFieldMapping('field_legacy_sidebar_1:format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('field_legacy_sidebar_2', 'sidebar2');
    $this->addFieldMapping('field_legacy_sidebar_2:format')
      ->defaultValue('filtered_html');
    */

    $this->addFieldMapping('field_legacy_url', 'legacyPath');
    $this->addFieldMapping('field_legacy_url:language')
      ->defaultValue('en');


    //$this->addFieldMapping('path', 'path');

    $this->addFieldMapping('field_nist_org', 'generatedOrg');

    $this->addFieldMapping('field_nist_topics', 'taxonomy_ids')
      ->separator(',')
      ->sourceMigration('NistTerm');


    $this->addFieldMapping('field_test_content')
      ->defaultValue(0);

    $this->addFieldMapping('field_nist_topics:create_term')
      ->defaultValue('False');
    $this->addFieldMapping('field_nist_topics:source_type')
      ->defaultValue('tid');

    $this->addFieldMapping('pathauto')
      ->defaultValue(0);

    $this->addFieldMapping('status')
      ->defaultValue(1);

    $this->addFieldMapping('title', 'page_title');

    $this->addFieldMapping('translate')
      ->defaultValue(0);

    $this->addFieldMapping('uid', 'cs_uid')
      ->sourceMigration('NistUser');


    // NEW field_nist_topic_areas
    $this->addFieldMapping('field_nist_topic_areas', 'newTaxonomyTermList')
      ->separator(',');
    $this->addFieldMapping('field_nist_topic_areas:source_type')
      ->defaultValue('tid');
    $this->addFieldMapping('field_nist_topic_areas:create_term')
      ->defaultValue('False');

    // field_access_rights
    $this->addFieldMapping('field_access_rights', 'permissionTIDBasedOnSubSite')
      ->separator(',');
    $this->addFieldMapping('field_access_rights:source_type')
      ->defaultValue('tid');
    $this->addFieldMapping('field_access_rights:create_term')
      ->defaultValue('False');

    $this->addUnmigratedDestinations(array('is_new', 'revision_uid', 'promote',
      'sticky', 'revision', 'log', 'language', 'tnid', 'comment', 'body:summary',
      'field_nist_topics:ignore_case', 'field_campus', 'field_campus:source_type',
      'field_campus:ignore_case', 'field_campus:create_term', 'field_contact_description',
      'field_contact_description_2', 'field_contact_description_3', 'field_external_contact',
      'field_external_contact_2', 'field_external_contact_3', 'field_document',
      'field_document:file_class', 'field_document:preserve_files',
      'field_document:destination_dir', 'field_document:destination_file',
      'field_document:file_replace', 'field_document:source_dir',
      'field_document:urlencode', 'field_document:description',
      'field_nist_internal_contact', 'field_nist_internal_contact_2',
      'field_nist_internal_contact_3',
      'field_document:display', 'field_hero_content',

    ));
    /*'field_nist_org:source_type',
     *'field_nist_org:create_term',
     *'field_nist_org:ignore_case',
     */

    $this->addUnmigratedSources(array('pagename', 'page_description'));
  }

  /**
   * Implementation of Migration::prepareRow($row).
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) :
      return FALSE;
    endif;

    // Fixes userID. translates it from Commonspot to CPR ID.
    $row->cs_uid = $this->_translate_user_id($row->cs_uid);

    // get the new Taxonomy terms based on current Commonspot taxonomy ids
    $row->newTaxonomyTermList = $this->getNewMappedTaxonomyTermsByOldIDList($row->taxonomy_ids);

    // access rights taxonomy
    $theTID = $this->getPermissionTaxonomyDataBySubSiteURL( $this->getSubSiteFromPath($row->path) );
    $row->permissionTIDBasedOnSubSite = $theTID;

    $row->legacyPath = $row->path;

    // Fix path to CS path without the .cfm
    $row->path = $this->_process_path($row->path);

    // Translates CS Subsite to NIST organization
    $row->generatedOrg = $this->_get_org_chart_name($row->path);
    // add some time (one min) to the changed value so it is forced to be updated.
    $row->changed = $this->_get_updated_time_for_force_updating( $row->changed );

    // Cleans up rubbish code
    $row->fic_blank_page_content = $this->_process_body_text_native($row->fic_msel_text);
  }

  /**
   * I insert a redirect into the redirect module
   * @param $node
   * @param \stdClass $row
   */
  public function complete($node, stdClass $row) {

    $this->addRedirect($node, $row);
    $this->addWorkBenchModerationNodeHistoryEntry($node);
  }

  /**
   * I am called by convention. I call teh completCollbck
   * @param array $nids
   */
  public function bulkRollback(array $nids) {

    $this->completeRollback($nids);
  }

  /**
   * I load all the Basic Page entites by node ids and delete
   * their redirects form the redirect table.
   * @param $nids
   */
  public function completeRollback($nids) {
    //parent::completeRollback();

    $entityArray = entity_load('Basic Page', $nids);

    foreach ($entityArray as &$entity) {
      // Remove all redirects for events
      $this->_delete_redirects_from_path($entity->path);
    }

  }
}
