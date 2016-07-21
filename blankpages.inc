<?php

/**
 * @file
 * Implementation of migration from Commonspot blank page custom elements to
 * Drupal Basic Page nodes.
 *
 */

class NistBlankPagesMigration extends NistMigration {
  public function __construct($arguments) {

    parent::__construct($arguments);

    $this->description = t('Import of Commonspot blank pages content types into Drupal\'s Basic Page content type');
    $this->dependencies = array('NistUser');

    // create a date that is 2 days older than now
    $fromdate = $this->getFromDate(2);

    // CommonSpot form ID
    $form_id = "";

    // the URL to pull ID and Pages from
    $list_url = "{JSON array with list of item IDs that will be passed into the method below}";
    $item_url = "{single record method that takes an id query string}=:id";

    $fields = array(
      'changed' => 'Date the page was last modified',
      'created' => 'Date the page was last created',
      'cs_uid' => 'Commonspot user that created the page',
      'fic_blank_page_column_amout' => 'meta info about the number of columns layout of the page',
      'fic_blank_page_content' => 'big nasty blob of HTML that is the content of the page',
      'fic_contact_information' => 'Contact block from the page',
      'owner_name' => 'Plaintext username of the owner of the page',
      'page_description' => 'Description of the page, often not used and same and title',
      'page_title' => 'Title of the page',
      'pageid' => 'CommonSpot PageID',
      'pagename' => 'Name of the page used in the url',
      'parsedimageids' => 'IDs of any images inside the page.',
      'path' => 'URL in the CommonSpot system possibly used for Pathauto purposes',
      'sidebar' => 'DOM scrape of content I can\'t pull as structured in the sidebar',
      'sidebar2' => 'the other sidebar of three column pages, see above',
      'taxonomy_ids' => 'IDs of taxonomy terms from FLEET',
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
    $this->addFieldMapping('body', 'bodyContentWithOutInitialH1Tag');
    $this->addFieldMapping('body:format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('body:language')->defaultValue('en');

    $this->addFieldMapping('changed', 'changed');

    $this->addFieldMapping('created', 'created');

    $this->addFieldMapping('field_image', 'parsedimageids')
      ->separator(', ')
      ->sourceMigration('NistImage');
    $this->addFieldMapping('field_image:file_class')
      ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_image:preserve_files')
      ->defaultValue(TRUE);

    // field_legacy_contact
    $this->addFieldMapping('field_legacy_contact', 'fic_contact_information');
    $this->addFieldMapping('field_legacy_contact:format')
      ->defaultValue('filtered_html');

    // field_legacy_owner_name
    $this->addFieldMapping('field_legacy_owner_name', 'owner_name');
    $this->addFieldMapping('field_legacy_owner_name:language')
      ->defaultValue('en');

    // field_legacy_sidebar_1
    $this->addFieldMapping('field_legacy_sidebar_1', 'sideBar1Processed');
    $this->addFieldMapping('field_legacy_sidebar_1:format')
      ->defaultValue('filtered_html');

    // field_legacy_sidebar_2
    $this->addFieldMapping('field_legacy_sidebar_2', 'sideBar2Processed');
    $this->addFieldMapping('field_legacy_sidebar_2:format')
      ->defaultValue('filtered_html');

    // field_legacy_url
    $this->addFieldMapping('field_legacy_url', 'legacyPath');
    $this->addFieldMapping('field_legacy_url:language')
      ->defaultValue('en');

    // field_access_rights
    $this->addFieldMapping('field_access_rights', 'permissionTIDBasedOnSubSite')
      ->separator(', ');
    $this->addFieldMapping('field_access_rights:source_type')
      ->defaultValue('tid');
    $this->addFieldMapping('field_access_rights:create_term')
      ->defaultValue('False');
    
    // field_nist_org
    $this->addFieldMapping('field_nist_org', 'generatedOrg');

    // field_nist_topics
    $this->addFieldMapping('field_nist_topics', 'taxonomy_ids')
      ->separator(', ')
      ->sourceMigration('NistTerm');

    // NEW field_nist_topic_areas
    $this->addFieldMapping('field_nist_topic_areas', 'newTaxonomyTermList')
      ->separator(', ');
    $this->addFieldMapping('field_nist_topic_areas:source_type')
      ->defaultValue('tid');
    $this->addFieldMapping('field_nist_topic_areas:create_term')
      ->defaultValue('False');

    // field_test_content
    $this->addFieldMapping('field_test_content')
      ->defaultValue(0);

    // field_nist_topics
    $this->addFieldMapping('field_nist_topics:create_term')
      ->defaultValue('False');
    $this->addFieldMapping('field_nist_topics:source_type')
      ->defaultValue('tid');

    /*
    $this->addFieldMapping('pathauto')
      ->defaultValue(0);
    */

    $this->addFieldMapping('status')
      ->defaultValue(1);

    $this->addFieldMapping('title', 'theTitle');

    $this->addFieldMapping('translate')
      ->defaultValue(0);
    $this->addFieldMapping('uid', 'cs_uid')
      ->sourceMigration('NistUser');

    // sections for pages
    $this->addFieldMapping('field_section', 'generatedSectionFromPath');

    // don't want to add the path anymore. borks menus
    // path
    // $this->addFieldMapping('path', 'path');

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

    $this->addUnmigratedSources(array('pagename', 'page_description'));
  }

  /**
   * Implementation of Migration::prepareRow($row).
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) :
      return FALSE;
    endif;

    $row->theTitle = $this->getPageTitle($row->page_title, $row->fic_blank_page_content);

    // get the new Taxonomy terms based on current Commonspot taxonomy ids
    $row->newTaxonomyTermList = $this->getNewMappedTaxonomyTermsByOldIDList($row->taxonomy_ids);

    // Fixes userID. translates it from Commonspot to CPR ID.
    $row->cs_uid = $this->_translate_user_id($row->cs_uid);

    $row->bodyContentWithOutInitialH1Tag = $this->removeInitialH1Tag($this->_process_body_text_native($row->fic_blank_page_content, 'blankpage-migrate-image'));
    //$row->bodyContentWithOutInitialH1Tag = $row->fic_blank_page_content;
    $row->sideBar1Processed = $this->_process_body_text_native($row->sidebar, 'blankpage-migrate-image');
    $row->sideBar2Processed = $this->_process_body_text_native($row->sidebar2, 'blankpage-migrate-image');

    $row->legacyPath = $row->path;

    // Fix path to CS path without the .cfm
    //$row->path = $this->_process_path($row->path);

    // Translates CS Subsite to NIST organization
    $row->generatedOrg = $this->_get_org_chart_name( $row->path );

    // add some time (one min) to the changed value so it is forced to be updated.
    $row->changed = $this->_get_updated_time_for_force_updating( $row->changed );

    // access rights taxonomy
    $theTID = $this->getPermissionTaxonomyDataBySubSiteURL( $this->getSubSiteFromPath($row->path) );
    $row->permissionTIDBasedOnSubSite = $theTID;
    $row->generatedSectionFromPath = $this->getGeneratedSectionFromPath($theTID);

    // Cleans up rubbish code
    //$row->fic_blank_page_content = $this->_process_body_text_native($row->fic_blank_page_content, 'blankpage-migrate-image');
  }

  /**
   * I return the page title based on if one is supplied in the page_title
   * field
   * @param $pageTitle
   * @param $bodyContent
   * @return string
   */
  public function getPageTitle($pageTitle, $bodyContent) {

    if ( strlen($pageTitle) == 0 && strlen($bodyContent) != 0) {
      $dom = new DOMDocument();
      $dom->loadHTML( $bodyContent );
      $h1List = $dom->getElementsByTagName( "h1" );
      $theFirstH1Node = $h1List->item( 0 );

      $result = $theFirstH1Node->nodeValue;
    }
    else {
      $result = $pageTitle;
    }

    return $result;
  }

  /**
   * I remove the initial occurrence of the H1 Tag.
   * @param $bodyContent
   * @return mixed
   */
  public function removeInitialH1Tag($bodyContent) {


    // make the result
    $result = $bodyContent;

    if (strlen($bodyContent) !== 0) {

      libxml_use_internal_errors(TRUE);

      // build a dom object and find the 1st H1 tag
      $dom = new DOMDocument();
      $dom->loadHTML($bodyContent);
      $h1List = $dom->getElementsByTagName("h1");
      $theFirstH1Node = $h1List->item(0);
      $theH1Content = $theFirstH1Node->nodeValue;

      // now build the string to replace
      $stringToFind = '<h1>' . $theH1Content . '</h1>';

      // find the position of the H1 tag
      $position = strpos($bodyContent, $stringToFind);

      // if there is an h1, kill it...
      if ($position !== FALSE) {
        $result = substr_replace($bodyContent, '', $position, strlen($stringToFind));
      }
    }

    return $result;
  }

  /**
   * I am called by convention by the Migrate framework after an import has
   * run.
   */
  public function postImport() {

    $this->logMessage( 'BlankPages Migrated', 'blankpages.log' );
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

    // save some memory
    drupal_static_reset();
  }

  /**
   * I am called by convention. I call the prepareRollback
   * @param array $nids
   */
  public function bulkRollback(array $nids) {

    //$this->prepareRollback($nids, 'Award');
  }
}
