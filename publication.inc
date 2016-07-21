<?php

/**
 * @file
 * Migration of Publications from SQL dump to Drupal
 */

class NistPublicationMigration extends NistMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Import of Publications from SQL export to Drupal');
    $this->dependencies = array(
      'NistUser',
      'NistTerm',
      'NistNikeAuthors'
    );


    $list_url = "";
    $item_url = "";
    
    $fields = array(
      'abstract' => 'Abstract',
      'author_list' => 'List of Authors',
      'citation' => 'Citation',
      'conf_dates' => 'Conference Dates',
      'conf_location' => 'Conference Location',
      'conf_title' => 'Conference Title',
      'CPR_ID_list' => 'list of CPR user-author IDs',
      'file_download' => 'DOI or local file download URL',
      'filesize' => 'Filesize',
      'issue' => 'Issue',
      'keywords' => 'Keywords',
      'last_modified' => 'Date Last Modified',
      'linktitles' => 'titles of the download links',
      'org_unit' => 'OU',
      'pages' => 'Pages',
      'proceedings_title' => 'Proceedings Title',
      'id' => 'Publication ID in NIKE',
      'NIKE_ID_list' => 'list of NIKE author ids',
      'pubdate' => 'Publication Date',
      'publisher_info' => 'Publisher Info',
      'pubtype' => 'the type of publication it is',
      'research_field' => 'Topic area from classic taxonomy',
      'series' => 'NIST\'s publication series',
      'start_page' => 'The start page of the publication',
      'end_page' => 'The end page of the publication',
      'title' => 'Pub Title',
      'volume' => 'Volume',
      'web_link' => 'Web Link'
    );

    $this->highwaterField = array(
      'name' => 'last_modified', // Column to be used as highwater mark
      'alias' => 'f',
    );

    $this->source = new MigrateSourceList(
      new MigrateListJSON($list_url),
      new MigrateItemJSON($item_url, array()),
      $fields
    );

    $this->destination = new MigrateDestinationNode('publication');

    $this->map = new MigrateSQLMap(
      $this->machineName, array(
        'id' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'Publication ID',
        )
      ),
      MigrateDestinationNode::getKeySchema()
    );

    // Mapped fields
    $this->addFieldMapping('uid')
      ->defaultValue('46611');
    $this->addFieldMapping('language')
      ->defaultValue('en');
    $this->addFieldMapping('body', 'abstract');
    $this->addFieldMapping('body:format')
      ->defaultValue('publication_author_html');
    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping('metatag_title', 'title');
    $this->addFieldMapping('changed', 'last_modified');
    $this->addFieldMapping('created', 'pubdate');
    $this->addFieldMapping('field_citation', 'citation');
    $this->addFieldMapping('field_conference_dates', 'conf_dates');
    $this->addFieldMapping('field_conference_location', 'conf_location');
    $this->addFieldMapping('field_conference_title', 'conf_title');
    $this->addFieldMapping('field_date', 'pubdate');
    $this->addFieldMapping('field_date:timezone')
      ->defaultValue('America/New_York');
    $this->addFieldMapping('field_file_size', 'filesize');
    $this->addFieldMapping('field_issue', 'issue');
    $this->addFieldMapping('field_nike_tags', 'keywords')
      ->separator(', ');
    $this->addFieldMapping('field_nike_tags:create_term')
      ->defaultValue('TRUE');
    $this->addFieldMapping('field_nike_tags:ignore_case')
      ->defaultValue('TRUE');
    $this->addFieldMapping('field_nist_authors', 'CPR_ID_list')
      ->sourceMigration('NistUser')
      ->separator(', ');
    $this->addFieldMapping('field_nike_authors', 'NIKE_ID_list')
      ->sourceMigration('NistNikeAuthors')
      ->separator(', ');
    $this->addFieldMapping('field_nist_org', 'generatedOrg');
    // NEW field_nist_topic_areas
    $this->addFieldMapping('field_nist_topic_areas', 'newTaxonomyTermList')
      ->separator(', ');
    $this->addFieldMapping('field_nist_topic_areas:source_type')
      ->defaultValue('tid');
    $this->addFieldMapping('field_nist_topic_areas:create_term')
      ->defaultValue('False');
    $this->addFieldMapping('field_nist_pub_series', 'series');
    $this->addFieldMapping('field_nist_pub_series:ignore_case')
      ->defaultValue('TRUE');
    $this->addFieldMapping('field_number_of_pages', 'pages');
    $this->addFieldMapping('field_proceedings_title', 'proceedings_title');
    $this->addFieldMapping('field_publication_authors', 'author_list');
    $this->addFieldMapping('field_publication_authors:format')
      ->defaultValue('publication_author_html');
    $this->addFieldMapping('field_publication_id', 'id');
    $this->addFieldMapping('field_pub_type', 'pubtype');
    $this->addFieldMapping('field_pub_type:ignore_case')
      ->defaultValue('TRUE');
    $this->addFieldMapping('field_publisher_info', 'publisher_info');
    $this->addFieldMapping('field_test_content')
      ->defaultValue('0');
    $this->addFieldMapping('field_url', 'file_download')
      ->separator(', ');
     $this->addFieldMapping('field_url:title', 'linktitles')
       ->separator(', ');
    $this->addFieldMapping('field_volume', 'volume');
    $this->addFieldMapping('field_pub_weblink', 'web_link');
    $this->addFieldMapping('field_pub_weblink:language')
      ->defaultValue('en');
    $this->addFieldMapping('pathauto')
      ->defaultValue('0');

    $this->addFieldMapping('field_start_page', 'start_page');
    $this->addFieldMapping('field_end_page', 'end_page');

    // Unmapped destination fields
    $this->addUnmigratedDestinations(array(
      'translate',
      'field_url:attributes',
      'is_new',
      'revision_uid',
      'promote',
      'sticky',
      'field_date:rrule',
      'field_date:to',
      'revision',
      'field_related_publication',
      'log',
      'tnid',
      'path',
      'comment',
      'body:summary',
      'field_pub_weblink:attributes',
      'field_pub_weblink:title',
      'field_nist_pub_series:source_type',
      'field_nist_pub_series:create_term',
      'field_pub_type:source_type',
      'field_pub_type:create_term'
    ));

    /*
     *'field_nist_org:source_type',
     *'field_nist_org:create_term',
     */

    // Unmapped source fields
    $this->addUnmigratedSources(array(
      'org_unit'
    ));

  }
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) :
      return FALSE;
    endif;

    // Translates CS Subsite to NIST organization
    //$row->generatedOrg = $this->_get_org_chart_name($row->path);

    switch ($row->pubtype) :
      case '0': $row->pubtype = 'Journal'; break;
      case '1': $row->pubtype = 'Book'; break;
      case '2': $row->pubtype = 'Encyclopedia'; break;
      case '3': $row->pubtype = 'Conference'; break;
      case '4': $row->pubtype = 'Website'; break;
      case '5': $row->pubtype = 'NIST Publication'; break;
      case '6': $row->pubtype = 'Other'; break;
      case '7': $row->pubtype = 'Book Chapter'; break;
      case '8': $row->pubtype = 'Talk'; break;

      default:
        break;
    endswitch;

    if (stristr($row->doi, 'http://') === FALSE && !empty($row->doi)) :
      $row->doi = "http://dx.doi.org/10.1002/{$row->doi}";
    endif;

    // default the org to null, but if its valid in the crosswalk use it
    $row->generatedOrg = NULL;
    if ($this->checkPublicationOrgIsValid($row->org_unit )) {
      $row->generatedOrg = $row->org_unit;
    }

    // get the new Taxonomy terms based on current Commonspot taxonomy ids
    $row->newTaxonomyTermList = $this->getPublicationTIDListFromPublicationResearchField($row->research_field);


    // add some time (one min) to the changed value so it is forced to be updated.
    $row->changed = $this->getUpdateTimeForForcedMigration( $row->last_modified );


    drush_log($row->series, $type = 'warning', $error = NULL);
    /*
    drush_log($row->title, $type = 'warning', $error = null);
    drush_log($row->generatedOrg, $type = 'warning', $error = null);
    drush_log($row->newTaxonomyTermList, $type = 'warning', $error = null);
    */
  }

  /**
   * I increment a date so Migrate will FORCE the updating of a migration
   * @param $time - I am the date/time to add to
   * @return DateTime
   */
  function getUpdateTimeForForcedMigration($time) {

    $newDate = date_add(
      date_create( $time ),
      // use for updating
      date_interval_create_from_date_string('35 seconds')
      // use for no updating
      //date_interval_create_from_date_string('0 seconds')
    );

    $result = date_timestamp_get( $newDate );
    
    return  $result;
  }


  /**
   * I am called by convention by the Migrate framework
   * @param $node
   * @param \stdClass $row
   */
  public function complete($node, stdClass $row) {

    $node_alias = url("node/{$node->nid}");
    $node_alias = ltrim($node_alias, '/');
    $legacyCommonSpotURL = 'manuscript-publication-search.cfm?pub_id=';
    $legacyCommonSpotURL = $legacyCommonSpotURL . $row->id;

    $redirect = new stdClass();
    redirect_object_prepare($redirect);

    $redirect->source = 'manuscript-publication-search.cfm';
    $redirect->source_options = array(
      'query' =>
        array(
          'pub_id' => $row->id
        )
    );
    $redirect->redirect = $node_alias;
    $redirect->hash = drupal_hash_base64($row->id);

    redirect_save($redirect);

/*    drush_log('REDIRECT', $type = 'warning', $error = null);
    drush_log($legacyCommonSpotURL, $type = 'warning', $error = null);
    drush_log($node_alias, $type = 'warning', $error = null);
*/
    
    return TRUE;
  }


}
