<?php

/**
 * @file
 * Implementation of migration from Commonspot Award custom element to Drupal award nodes.
 */

class NistCnsttoolMigration extends NistMigration {
    public function __construct($arguments) {
        parent::__construct($arguments);
        $this->description = t('Import of Commonspot CNST Tool custom element into tool content type');
        $this->dependencies = array('NistUser');

        $fields = array(
            'changed' => 'Date the page was last modified',
            'created' => 'Date the page was last created',
            'contact_block' => '',
            'cs_uid' => 'Commonspot user that created the page',
            'fic_address' => ' ',
            'fic_contact' => ' ',
            'fic_description' => 'Descriptiom of the tool',
            'fic_email' => ' ',
            'fic_hourlyraterow1fullcost' => ' ',
            'fic_hourlyraterow1label' => ' ',
            'fic_hourlyraterow1sharedcost' => ' ',
            'fic_hourlyraterow2fullcost' => ' ',
            'fic_hourlyraterow2label' => ' ',
            'fic_hourlyraterow2sharedcost' => ' ',
            'fic_hourlyraterow3fullcost' => ' ',
            'fic_hourlyraterow3label' => ' ',
            'fic_hourlyraterow3sharedcost' => ' ',
            'fic_hourlyraterow4fullcost' => ' ',
            'fic_hourlyraterow4label' => ' ',
            'fic_hourlyraterow4sharedcost' => ' ',
            'fic_hoursofoperation' => ' ',
            'fic_image' => 'Image of the tool or instrument',
            'fic_image_caption' => 'Image caption of the tool or instrument',
            'fic_location' => 'Where is this tool located',
            'fic_name' => 'Name of the tool or instrument',
            'fic_operatinginstructions' => 'Basic operating instructions for the tool or instument',
            'fic_phone' => 'Phone number of who to contact of the instrument.',
            'fic_relatedprocesses' => 'Related Processes',
            'fic_specificationscapabilities' => 'Specifications and capabilities',
            'fic_supportedsamplesizes' => 'supported sample sizes',
            'fic_title' => 'title of page, typically Tool Name',
            'fic_toolreservationrules' => 'Rules about reserving the tool or instrument',
            'fic_trainingraterow1groupcost' => ' ',
            'fic_trainingraterow1individualcost' => ' ',
            'fic_trainingraterow1label' => ' ',
            'fic_trainingraterow2groupcost' => ' ',
            'fic_trainingraterow2individualcost' => ' ',
            'fic_trainingraterow2label' => ' ',
            'fic_trainingraterow3groupcost' => ' ',
            'fic_trainingraterow3individualcost' => ' ',
            'fic_trainingraterow3label' => ' ',
            'fic_trainingraterow4groupcost' => ' ',
            'fic_trainingraterow4individualcost' => ' ',
            'fic_trainingraterow4label' => ' ',
            'fic_trainingtime' => ' ',
            'fic_typicalapplications' => 'Typical Applications of this tool',
            'owner_name' => 'Plaintext username of the owner of the page, mostly for making sure the relationships are correct',
            'page_description' => 'Description of the page, often not used and same and title',
            'page_title' => 'Title of the page',
            'pageid' => 'CommonSpot PageID',
            'pagename' => 'Name of the page used in the url',
            'parsedimageids' => '',
            'path' => 'URL in the CommonSpot system possibly used for Pathauto purposes',
            'taxonomy_ids' => 'IDs of taxonomy terms from FLEET'
        );

        $this->highwaterField = array(
            'name' => 'changed', // Column to be used as highwater mark
            'alias' => 'w',
        );

        // JSON URL
        $form_id = "1544965";
        $from_date = $this->getFromDate(2);
        $this->source = new MigrateSourceJSON("{MIgrate source Array}", 'pageid', $fields);

        // Content Type
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

        $this->addFieldMapping('uid', 'cs_uid')->sourceMigration('NistUser');

        $this->addFieldMapping('title', 'fic_title');

        $this->addFieldMapping('body', 'fic_description');
        $this->addFieldMapping('body:format')->defaultValue('filtered_html');
        $this->addFieldMapping('body:language')->defaultValue('en');

        $this->addFieldMapping('status')->defaultValue(1);

        //$this->addFieldMapping('path', 'path');
        $this->addFieldMapping('pathauto')->defaultValue(1);

        $this->addFieldMapping('translate')->defaultValue(0);

        // Taxonomy
        $this->addFieldMapping('field_nist_topics', 'taxonomy_ids')->separator(', ')->sourceMigration('NistTerm');
        $this->addFieldMapping('field_nist_topics:source_type')->defaultValue('tid');
        $this->addFieldMapping('field_nist_topics:create_term')->defaultValue('False');

        // Name
        $this->addFieldMapping('field_nemo_name', 'fic_name');
        $this->addFieldMapping('field_nemo_name:language')->defaultValue('en');

        // Location
        $this->addFieldMapping('field_location', 'fic_location');
        $this->addFieldMapping('field_location:language')->defaultValue('en');

        $this->addFieldMapping('field_supported_sample_sizes', 'fic_supportedsamplesizes');
        $this->addFieldMapping('field_supported_sample_sizes:language')->defaultValue('en');
        $this->addFieldMapping('field_supported_sample_sizes:format')->defaultValue('filtered_html');


        $this->addFieldMapping('field_typical_applications', 'fic_typicalapplications');
        $this->addFieldMapping('field_typical_applications:language')
          ->defaultValue('en');
        $this->addFieldMapping('field_typical_applications:format')
          ->defaultValue('filtered_html');

        // Operation Instructions
        $this->addFieldMapping('field_operating_instructions', 'fic_operatinginstructions')
            ->issueGroup(t('Issues'))
            ->description(t('Nearly all of these are PDFs. Is there a better way to do this?'))
            ->issuePriority(MigrateFieldMapping::ISSUE_PRIORITY_MEDIUM);
        $this->addFieldMapping('field_operating_instructions:language')
            ->defaultValue('en');

        $this->addFieldMapping('field_related_processes', 'fic_relatedprocesses');
        $this->addFieldMapping('field_related_processes:language')
            ->defaultValue('en');

        $this->addFieldMapping('field_legacy_url', 'path');
        $this->addFieldMapping('field_legacy_url:language')
            ->defaultValue('en');

        $this->addFieldMapping('field_legacy_owner_name', 'owner_name');
        $this->addFieldMapping('field_legacy_owner_name:language')
            ->defaultValue('en');

        $this->addFieldMapping('field_test_content')->defaultValue(0);

        $this->addFieldMapping('field_instrument_or_tool_select')->defaultValue('tool');

        // Image
        $this->addFieldMapping('field_image', 'migrate_fic_image_id')->sourceMigration('NistImage');
        $this->addFieldMapping('field_image:file_class')->defaultValue('MigrateFileFid');
        $this->addFieldMapping('field_image:preserve_files')->defaultValue(TRUE);
        $this->addFieldMapping('field_image:alt', 'migrate_fic_image_alt_title');
        $this->addFieldMapping('field_image:title', 'migrate_fic_image_alt_title');

        $this->addFieldMapping('field_campus')->defaultValue('Gaithersburg');
        $this->addFieldMapping('field_campus:create_term')->defaultValue(FALSE);
        $this->addFieldMapping('field_campus:ignore_case')->defaultValue(FALSE);

        $this->addFieldMapping('field_nist_org')->defaultValue('Center for Nanoscale Science and Technology');
        $this->addFieldMapping('field_nist_org:create_term')->defaultValue(FALSE);
        $this->addFieldMapping('field_nist_org:ignore_case')->defaultValue(FALSE);

        $this->addFieldMapping('field_specs_capabilities', 'fic_specificationscapabilities');
        $this->addFieldMapping('field_specs_capabilities:format')->defaultValue('filtered_html');
        $this->addFieldMapping('field_specs_capabilities:language')->defaultValue('en');

        // ********************** new stuff from JFA **********************

        // Hourly Rows 1- 4
        $this->addFieldMapping('field_hourly_rate_1_full_cost', 'fic_hourlyraterow1fullcost');
        $this->addFieldMapping('field_hourly_rate_1_full_cost:language')->defaultValue('en');
        $this->addFieldMapping('field_hourly_rate_1_label', 'fic_hourlyraterow1label');
        $this->addFieldMapping('field_hourly_rate_1_label:language')->defaultValue('en');
        $this->addFieldMapping('field_hourly_rate_1_shared_cost', 'fic_hourlyraterow1sharedcost');
        $this->addFieldMapping('field_hourly_rate_1_shared_cost:language')->defaultValue('en');
        $this->addFieldMapping('field_hourly_rate_2_full_cost', 'fic_hourlyraterow2fullcost');
        $this->addFieldMapping('field_hourly_rate_2_full_cost:language')->defaultValue('en');
        $this->addFieldMapping('field_hourly_rate_2_label', 'fic_hourlyraterow2label');
        $this->addFieldMapping('field_hourly_rate_2_label:language')->defaultValue('en');
        $this->addFieldMapping('field_hourly_rate_2_shared_cost', 'fic_hourlyraterow2sharedcost');
        $this->addFieldMapping('field_hourly_rate_2_shared_cost:language')->defaultValue('en');
        $this->addFieldMapping('field_hourly_rate_3_full_cost', 'fic_hourlyraterow3fullcost');
        $this->addFieldMapping('field_hourly_rate_3_full_cost:language')->defaultValue('en');
        $this->addFieldMapping('field_hourly_rate_3_label', 'fic_hourlyraterow3label');
        $this->addfieldMapping('field_hourly_rate_3_label:language')->defaultValue('en');
        $this->addFieldMapping('field_hourly_rate_3_shared_cost', 'fic_hourlyraterow3sharedcost');
        $this->addFieldMapping('field_hourly_rate_3_shared_cost:language')->defaultValue('en');
        $this->addFieldMapping('field_hourly_rate_4_full_cost', 'fic_hourlyraterow4fullcost');
        $this->addFieldMapping('field_hourly_rate_4_full_cost:language')->defaultValue('en');
        $this->addFieldMapping('field_hourly_rate_4_label', 'fic_hourlyraterow4label');
        $this->addFieldMapping('field_hourly_rate_4_label:language')->defaultValue('en');
        $this->addFieldMapping('field_hourly_rate_4_shared_cost', 'fic_hourlyraterow4sharedcost');
        $this->addFieldMapping('field_hourly_rate_4_shared_cost:language')->defaultValue('en');

        // field_legacy_contact
        $this->addFieldMapping('field_legacy_contact', 'contact_block');
        $this->addFieldMapping('field_legacy_contact:format')->defaultValue('filtered_html');
        $this->addFieldMapping('field_legacy_contact:language')->defaultValue('en');

        // field_hours_of_operation
        $this->addFieldMapping('field_hours_of_operation', 'fic_hoursofoperation');
        $this->addFieldMapping('field_hours_of_operation:format')->defaultValue('filtered_html');
        $this->addFieldMapping('field_hours_of_operation:language')->defaultValue('en');

        // fic_toolreservationrules
        $this->addFieldMapping('field_tool_reservation_rules', 'fic_toolreservationrules');
        $this->addFieldMapping('field_tool_reservation_rules:format')->defaultValue('filtered_html');
        $this->addFieldMapping('field_tool_reservation_rules:language')->defaultValue('en');


        // Training Rows 1 - 4
        $this->addFieldMapping('field_training_rate_1_group_cost', 'fic_trainingraterow1groupcost');
        $this->addFieldMapping('field_training_rate_1_group_cost:language')->defaultValue('en');
        $this->addFieldMapping('field_training_rate_1_individual', 'fic_trainingraterow1individualcost');
        $this->addFieldMapping('field_training_rate_1_individual:language')->defaultValue('en');
        $this->addFieldMapping('field_training_rate_1_label', 'fic_trainingraterow1label');
        $this->addFieldMapping('field_training_rate_1_label:language')->defaultValue('en');
        $this->addFieldMapping('field_training_rate_2_group_cost', 'fic_trainingraterow2groupcost');
        $this->addFieldMapping('field_training_rate_2_group_cost:language')->defaultValue('en');
        $this->addFieldMapping('field_training_rate_2_individual', 'fic_trainingraterow2individualcost');
        $this->addFieldMapping('field_training_rate_2_individual:language')->defaultValue('en');
        $this->addFieldMapping('field_training_rate_2_label', 'fic_trainingraterow2label');
        $this->addFieldMapping('field_training_rate_2_label:language')->defaultValue('en');
        $this->addFieldMapping('field_training_rate_3_group_cost', 'fic_trainingraterow3groupcost');
        $this->addFieldMapping('field_training_rate_3_group_cost:language')->defaultValue('en');
        $this->addFieldMapping('field_training_rate_3_individual', 'fic_trainingraterow3individualcost');
        $this->addFieldMapping('field_training_rate_3_individual:language')->defaultValue('en');
        $this->addFieldMapping('field_training_rate_3_label', 'fic_trainingraterow3label');
        $this->addFieldMapping('field_training_rate_3_label:language')->defaultValue('en');
        $this->addFieldMapping('field_training_rate_4_group_cost', 'fic_trainingraterow4groupcost');
        $this->addFieldMapping('field_training_rate_4_group_cost:language')->defaultValue('en');
        $this->addFieldMapping('field_training_rate_4_individual', 'fic_trainingraterow4individualcost');
        $this->addFieldMapping('field_training_rate_4_individual:language')->defaultValue('en');
        $this->addFieldMapping('field_training_rate_4_label', 'fic_trainingraterow4label');
        $this->addFieldMapping('field_training_rate_4_label:language')->defaultValue('en');

        // fic_trainingtime
        $this->addFieldMapping('field_training_time', 'fic_trainingtime');
        $this->addFieldMapping('field_training_time:language')->defaultValue('en');

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
        $this->addUnmigratedDestinations(array(
            'is_new',
            'revision_uid',
            'promote',
            'sticky',
            'revision',
            'log',
            'language',
            'tnid',
            'comment',
            'field_nist_topics:ignore_case',
            'body:summary',
            'field_campus:source_type',
            'field_nist_org:source_type'
        ));

        // unmapped source fields
        $this->addUnmigratedSources(array(
            'fic_address',
            'fic_email',
            'fic_phone',
            'fic_contact',
            'pagename',
            'page_description',
            'page_title',
            'contact_block',
            'parsedimageids'
        ));

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

      $row->path = $this->_process_path($row->path);
      $row->cs_uid = $this->_translate_user_id($row->cs_uid);

      // Translates CS Subsite to NIST organization
      $row->generatedOrg = $this->_get_org_chart_name($row->path);

      // add some time (one min) to the changed value so it is forced to be updated.
      $row->changed = $this->_get_updated_time_for_force_updating( $row->changed );

      // access rights taxonomy
      $theTID = $this->getPermissionTaxonomyDataBySubSiteURL( $this->getSubSiteFromPath($row->path) );
      $row->permissionTIDBasedOnSubSite = $theTID;

      $row->description = $this->_process_body_text_native($row->description, 'cnstool-migrate-image');
      $row->operatinginstructions = $this->_process_body_text_native($row->operatinginstructions);
      $row->specificationscapabilities = $this->_process_body_text_native($row->specificationscapabilities);
      $row->supportedsamplesizes = $this->_process_body_text_native($row->supportedsamplesizes);
      $row->typicalapplications = $this->_process_body_text_native($row->typicalapplications);
      $row->contact_block = $this->_process_body_text_native( $this->buildContactHTML( $row ) );

      // Get the image id and alt from the string
      $image_id_alt = explode('|', $row->fic_image);

      $row->migrate_fic_image_id = $image_id_alt[0];
      $row->migrate_fic_image_id = str_replace( 'CPIMAGE:', '', $row->migrate_fic_image_id );
      $row->migrate_fic_image_alt_title = $image_id_alt[1];
    }

    /**
     * I build the contact legacy HTML.
     * @param $row - I am the row of data.
     * @return string - I am the HTML string.
     */
    public function buildContactHTML( $row ) {

        $contact_block = '';

        // fic_contact
        // fic_email
        // fic_phone
        // fic_address

        if ( strlen( $row->fic_contact ) ) {
            $contact_block .= '<strong>Contact:</strong><br>' . $row->fic_contact . '<br>';
        }

        if ( strlen( $row->fic_email ) ) {
            $contact_block .= '<strong>Email:</strong> ' . $row->fic_email . '<br>';
        }

        if ( strlen( $row->fic_phone ) ) {
            $contact_block .= '<strong>Phone:</strong> ' . $row->fic_phone . '<br>';
        }

        if ( strlen( $row->fic_address ) ) {
            $contact_block .= '<strong>Address:</strong> ' . $row->fic_address;
        }

        return $contact_block;
    }

  /**
   * I am called by convention by the Migrate framework after an import has
   * run.
   */
  public function postImport() {

    $this->logMessage('CNSTTool Migrated', 'cnsttool.log');
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

    //$this->prepareRollback($nids, 'Award');
  }
}