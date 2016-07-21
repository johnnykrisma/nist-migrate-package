<?php

/**
 * @file
 * Implementation of migration from Commonspot news custom element to Drupal article nodes.
 */

class NistNewsMigration extends NistMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Import of Commonspot news custom element into article content type');
    $this->dependencies = array('NistUser');


    // create a date that is 2 days older than now
    $fromdate = $this->getFromDate(2);
    $list_url = "";
    $item_url = "";

    $fields = array(
      'changed' => 'Date the page was last modified',
      'created' => 'Date the page was last created',
      'cs_uid' => 'Commonspot user that created the page',
      'fic_anchorslug' => 'used in TechBeat',
      'fic_commenturl' => 'used in TechBeat',
      'fic_contact_information' => 'Field in news content type with news contact',
      'fic_includeinslideshow' => 'supposed to be used to include news stories in slideshow',
      'fic_issuedate' => 'used in TechBeat',
      'fic_news_item_associated_campus' => 'associated campus',
      'fic_news_item_body' => 'Body of the news story',
      'fic_news_item_contact_email' => 'contact email',
      'fic_news_item_contact_name' => 'contact name',
      'fic_news_item_contact_phone' => 'contact phone number',
      'fic_news_item_other_campus' => 'other associated Campus',
      'fic_news_item_release_date' => 'news release date',
      'fic_news_item_thumbnail' => 'thumbnail image',
      'fic_news_item_title' => 'Title',
      'fic_news_video' => 'video embed code',
      'fic_news_video_caption' => 'video caption',
      'fic_pbaapproved' => 'did PBA approve this?',
      'fic_quicklink' => 'used in TechBeat',
      'fic_releaseinfo' => 'release into like for immediate release',
      'fic_slideshowimage' => 'slideshow image',
      'fic_slideshowsection' => '',
      'fic_slideshowtext' => 'slideshow text',
      'fic_sortorder' => 'used in TechBeat',
      'owner_name' => 'Plaintext username of the owner of the page',
      'page_description' => 'Description of the page, often not used and same and title',
      'page_title' => 'Title of the page',
      'pageid' => 'CommonSpot PageID',
      'pagename' => 'Name of the page used in the url',
      'parsedimageids' => 'IDs of inline images',
      'path' => 'URL in the CommonSpot system possibly used for Pathauto purposes',
      'taxonomy_ids' => 'IDs of taxonomy terms from FLEET',
    );

    $this->highwaterField = array(
      'name' => 'changed', // Column to be used as highwater mark
      'alias' => 'w',
    );

    // $this->source = new MigrateSourceJSON('http://www.nist.gov/com/DataPull.cfc?method=getCEList&formid=2449&fromDate=2000-01-01&returnFormat=json', 'pageid', $fields);
    $this->source = new MigrateSourceList(
      new MigrateListJSON($list_url),
      new MigrateItemJSON($item_url, array()),
      $fields
    );

    $this->destination = new MigrateDestinationNode('article');

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

    $this->addFieldMapping('title', 'fic_news_item_title');

    $this->addFieldMapping('body', 'fic_news_item_body');
    $this->addFieldMapping('body:language')
      ->defaultValue('en');
    $this->addFieldMapping('body:format')
      ->defaultValue('filtered_html');

    $this->addFieldMapping('status')
      ->defaultValue(1);

    $this->addFieldMapping('translate')
      ->defaultValue(0);

    $this->addFieldMapping('field_image', 'parsedimageids')
      ->sourceMigration('NistImage');
    $this->addFieldMapping('field_image:file_class')
      ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_image:preserve_files')
      ->defaultValue(TRUE);

    $this->addFieldMapping('field_campus', 'fic_news_item_associated_campus')
      ->separator(', ');

    $this->addFieldMapping('field_campus:create_term')
      ->defaultValue(FALSE);

    $this->addFieldMapping('field_contact', 'fic_news_item_contact_name')
      ->issueGroup(t('Issues'))
      ->description(t('Not sure if just putting a name in works'))
      ->issuePriority(MigrateFieldMapping::ISSUE_PRIORITY_MEDIUM)
      ->issueNumber(23);

    $this->addFieldMapping('field_legacy_url', 'legacyPath');

    $this->addFieldMapping('pathauto')
      ->defaultValue(0);

    $this->addFieldMapping('field_date', 'fic_news_item_release_date');
    $this->addFieldMapping('field_date:timezone')
      ->defaultValue('America/New_York')
      ->issueGroup(t('Issues'))
      ->description(t('Timezone based on campus location?'))
      ->issuePriority(MigrateFieldMapping::ISSUE_PRIORITY_MEDIUM)
      ->issueNumber(22);

    $this->addFieldMapping('field_legacy_contact', 'contact_block');
    $this->addFieldMapping('field_legacy_contact:format')
      ->defaultValue('filtered_html');

    $this->addFieldMapping('field_legacy_video_embed', 'fic_news_video');
    $this->addFieldMapping('field_legacy_video_embed:language')
      ->defaultValue('en');
    $this->addFieldMapping('field_legacy_video_embed:format')
      ->defaultValue('filtered_html');

    $this->addFieldMapping('field_video_embed_caption', 'fic_news_video_caption');
    $this->addFieldMapping('field_video_embed_caption:language')
      ->defaultValue('en');
    $this->addFieldMapping('field_video_embed_caption:format')
      ->defaultValue('filtered_html');

    $this->addFieldMapping('field_other_campus', 'fic_news_item_other_campus');
    $this->addFieldMapping('field_other_campus:language')
      ->defaultValue('en');

    $this->addFieldMapping('field_nist_topics', 'taxonomy_ids')
      ->separator(', ')
      ->sourceMigration('NistTerm');
    $this->addFieldMapping('field_nist_topics:source_type')
      ->defaultValue('tid');
    $this->addFieldMapping('field_nist_topics:create_term')
      ->defaultValue('False');

    $this->addFieldMapping('field_pba_approval', 'fic_pbaapproved');

    $this->addFieldMapping('field_release_info', 'fic_releaseinfo');
    $this->addFieldMapping('field_release_info:language')
      ->defaultValue('en');

    $this->addFieldMapping('field_techbeat_anchor_slug', 'fic_anchorslug');
    $this->addFieldMapping('field_techbeat_anchor_slug:language')
      ->defaultValue('en');

    $this->addFieldMapping('field_techbeat_issue_date', 'fic_issuedate');
    $this->addFieldMapping('field_techbeat_issue_date:language')
      ->defaultValue('en');

    $this->addFieldMapping('field_techbeat_sort_order', 'fic_sortorder');
    $this->addFieldMapping('field_test_content')
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

    $this->addFieldMapping('field_nist_org', 'generatedOrg');

    // Unmapped destination fields
    $this->addUnmigratedDestinations(array('is_new', 'revision_uid', 'promote',
      'sticky',
      'revision',
      'log',
      'language',
      'tnid',
      'comment',
      'field_campus:source_type',
      'field_campus:ignore_case',
      'body:summary', 'field_date:rrule', 'field_date:to', 'field_nist_topics:ignore_case',
      'field_image:language',
      'field_related_event',
      'field_related_publication',
      'field_related_project',
      'field_related_instrument_tool',
      'field_related_news',
      'field_image:alt',
      'field_image:title',
      'field_featured',
      'field_featured_homepage',
      'field_image_caption',
      'field_image_caption:language',
      'field_rss',
      'field_contact_description',
      'field_contact_description:language',
      'field_contact_description_2',
      'field_contact_description_2:language',
      'field_contact_description_3',
      'field_contact_description_3:language',
      'field_external_contact',
      'field_external_contact_2',
      'field_external_contact_3',
      'field_legacy_contact:language',
      'field_nist_internal_contact',
      'field_nist_internal_contact_2',
      'field_nist_internal_contact_3',
      'field_related_video'
    ));

    // Unmapped source fields
    $this->addUnmigratedSources(array(
      'pagename',
      'page_description',
      'fic_news_item_contact_email',
      'fic_news_item_contact_phone',
      'fic_news_item_thumbnail',
      'fic_commenturl',
      'fic_includeinslideshow',
      'fic_slideshowimage',
      'fic_slideshowtext',
      'fic_quicklink',
      'owner_name',
      'page_title'
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

    $row->cs_uid = $this->_translate_user_id($row->cs_uid);

    // get the new Taxonomy terms based on current Commonspot taxonomy ids
    $row->newTaxonomyTermList = $this->getNewMappedTaxonomyTermsByOldIDList($row->taxonomy_ids);

    // $row->migrate_thisIsMyField = 'somevalue';
    $row->fic_news_item_associated_campus = $this->_process_campus($row->fic_news_item_associated_campus);

    $row->fic_news_item_body = $this->_process_body_text_native($row->fic_news_item_body, 'news-migrate-image');

    // Translates CS Subsite to NIST organization
    $row->generatedOrg = $this->_get_org_chart_name($row->path);

    // add some time (one min) to the changed value so it is forced to be updated.
    $row->changed = $this->_get_updated_time_for_force_updating( $row->changed );

    // access rights taxonomy
    $theTID = $this->getPermissionTaxonomyDataBySubSiteURL( $this->getSubSiteFromPath($row->path) );

    if ($theTID === NULL) {
      $theTID = $this->getPermissionTaxonomyDataBySubSiteURL( '/pba/' );
    }

    $row->permissionTIDBasedOnSubSite = $theTID;

    $row->contact_block = '<a href="mailto:' . $row->fic_news_item_contact_email . '">'
      . $row->fic_news_item_contact_name . '</a><br>' . $row->fic_news_item_contact_phone;
  }

  /*
  public function convertTableImageToFigureImage( $body ){

    $dom = new DOMDocument();
    $dom->loadHTML( $body );
    $tableListObject = $dom->getElementsByTagName( "table" );
    $theFirstH1Node = $tableListObject->item( 0 );
  }
  */


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

    //$this->prepareRollback($nids, 'News');
  }

  /**
   * I am called by convention by the Migrate framework after an import has
   * run.
   */
  public function postImport() {

    $this->logMessage('News Migrated', 'news.log');
  }
}
