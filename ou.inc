<?php

/**
 * @file
 * Implementation of migration from Commonspot Award custom element to Drupal award nodes.
 */

class NistOuMigration extends NistMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Import of Commonspot Divisions content type into the OU/lab content type');
    $this->dependencies = array('NistUser');

    $fields = array(
      'changed' => 'Date the page was last modified',
      'contact_block' => 'legacy contact info',
      'created' => 'Date the page was last created',
      'cs_uid' => 'Commonspot user that created the page',
      'fic_ou_id' => ' ',
      'fic_ou_selected_publications' => ' ',
      'fic_ou_welcome_text' => ' ',
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

    $this->source = new MigrateSourceJSON('', 'pageid', $fields);

    $this->destination = new MigrateDestinationNode('laboratory_organizational_unit');

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
    $this->addFieldMapping('body', 'fic_ou_welcome_text');
    $this->addFieldMapping('body:format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('body:language')
      ->defaultValue('en');

    $this->addFieldMapping('changed', 'changed');

    $this->addFieldMapping('created', 'created');

    $this->addFieldMapping('field_legacy_contact', 'contact_block');
    $this->addFieldMapping('field_legacy_contact:format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('field_legacy_contact:language')
      ->defaultValue('en');

    $this->addFieldMapping('field_nist_org', 'generatedOrg');

    $this->addFieldMapping('field_nist_topics', 'taxonomy_ids')
      ->separator(',')
      ->sourceMigration('NistTerm');
    $this->addFieldMapping('field_nist_topics:create_term')
      ->defaultValue('False');
    $this->addFieldMapping('field_nist_topics:source_type')
      ->defaultValue('tid');

    $this->addFieldMapping('path', 'path');

    $this->addFieldMapping('pathauto')
      ->defaultValue(1);

    $this->addFieldMapping('status')
      ->defaultValue(1);

    $this->addFieldMapping('title', 'page_title');

    $this->addFieldMapping('translate')
      ->defaultValue(0);

    $this->addFieldMapping('uid', 'cs_uid')
      ->sourceMigration('NistUser');

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
      'sticky', 'revision', 'log', 'language', 'tnid', 'comment', 'body:summary',
      'field_campus:ignore_case', 'field_campus:source_type', 'field_campus:create_term',
      'field_nist_org:source_type', 'field_nist_org:create_term', 'field_nist_org:ignore_case',
      'field_nist_topics:ignore_case'
    ));

    // Unmapped source fields
    $this->addUnmigratedSources(array('pagename', 'page_description', 'generatedOrg'));
  }

  /**
   * Implementation of Migration::prepareRow($row).
   */
  public function prepareRow( $row ) {
    if (parent::prepareRow($row) === FALSE) :
      return FALSE;
    endif;

    $row->path = $this->_process_path($row->path);

    // get the new Taxonomy terms based on current Commonspot taxonomy ids
    $row->newTaxonomyTermList = $this->getNewMappedTaxonomyTermsByOldIDList($row->taxonomy_ids);

    // Translates CS Subsite to NIST organization
    $row->generatedOrg = $this->_get_org_chart_name($row->path);
    // add some time (one min) to the changed value so it is forced to be updated.
    $row->changed = $this->_get_updated_time_for_force_updating($row->changed);

    $row->cs_uid = $this->_translate_user_id($row->cs_uid);
    $row->fic_group_welcome_text = $this->_process_body_text_native($row->fic_division_welcome_text, 'ou-migrate-image');

    // access rights taxonomy
    $theTID = $this->getPermissionTaxonomyDataBySubSiteURL( $this->getSubSiteFromPath($row->path) );
    $row->permissionTIDBasedOnSubSite = $theTID;
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
  }

  /**
   * I am called by convention. I call the completRollbck
   * @param array $nids
   */
  public function bulkRollback(array $nids) {

    //$this->prepareRollback($nids, 'Award');
  }
}

