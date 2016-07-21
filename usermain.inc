<?php
/**
 * Profile2 field migration
 *
 */
/**
 * @file
 * Examples and test fodder for migration into profile2 entities.
 */
 
class NistUserMainMigration extends NistMigration {
  public function __construct($arguments) {

    parent::__construct($arguments);

    $this->description = t('Migration of CPR profiles into profile2 entities');
    $this->dependencies = array('NistUser', 'NistOrgStructure', 'NistUserImage');

    $fields = array(
      "address1" => "Mailing address address 1",
      "address2" => "Mailing address address 2",
      "alt_first_name" => "I am the Users alternative first name.",
      "alt_last_name" => "I am the Users alternative last name.",
      "bio" => "I am the Users biographical information.",
      "campus" => "I am the campus the User is located in.",
      "city" => "Mailing address city",
      "displayname" => "I am the name to display of a User.",
      "division_code" => "I am the Division code of the user.",
      "division_name" => "I am the name of the division the user is in.",
      "education" => "I am the Users education history.",
      "email_address" => "I am the Users email address.",
      "employment" => "I am the Users employment history.",
      "fax" => "I am the Uses Facsimile number",
      "first_name" => "I am the users REAL first name.",
      "group_code" => "I am the Group Code that the User belongs to.",
      "group_name" => "I am the name of the group the User belongs to.",
      "id" => "I am the users ID",
      "imagealt" => "I am the alt text for a Users image.",
      "imageid" => "I am the image id of the User.",
      "last_name" => "I am the Users REAL last name.",
      "last_update" => "I am the last time the Users profile was changed.",
      "last_update_cs" => "I am the last time a Users content was updated in CommonSpot.",
      "mail_stop" => " I am the Users mailstop",
      "middle_name" => "I am the users Middle name.",
      "page_url" => "I am the Users CommonSpot Page URL if they have one.",
      "people_id" => "I am the Unique ID from the Central Peoples Repository.",
      "phone" => "I am the users phone number.",
      "selected_publications" => "I am a comma separated list of pub ids.",
      "staff_type" => "I am the staff type of the user.",
      "state" => "Mailing address state",
      "zip4" => "Mailing address last 4",
      "zip5" => "Mailing address zip first 5"
    );

    $this->highwaterField = array(
      'name' => 'last_update', // Column to be used as highwater mark
      'alias' => 'w',
    );

    $json_file = '';

    $this->source = new MigrateSourceList(
      new UserListJSON($json_file),
      new UserItemJSON($json_file, array()),
      $fields
    );

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'people_id' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null'    => TRUE,
          'description' => 'PeopleFinder ID'
        )
      ),
      MigrateDestinationProfile2::getKeySchema()
    );

    $this->destination = new MigrateDestinationProfile2('staff_profile');


    $this->addFieldMapping('uid', 'people_id')
      ->sourceMigration('NistUser')
      ->description(t('The assignment of profile2-items to the prev-imported users'));
    $this->addFieldMapping('language')
      ->defaultValue('en');
    $this->addFieldMapping('field_peoplefinder_id', 'people_id');
    $this->addFieldMapping('field_first_name', 'first_name');
    $this->addFieldMapping('field_alt_first_name', 'alt_first_name');
    $this->addFieldMapping('field_middle_name', 'middle_name');
    $this->addFieldMapping('field_last_name', 'last_name');
    $this->addFieldMapping('field_alt_last_name', 'alt_last_name');
    $this->addFieldMapping('field_fax', 'fax');
    $this->addFieldMapping('field_mail_stop', 'mail_stop');
    $this->addFieldMapping('field_phone', 'phone');
    $this->addFieldMapping('field_campus', 'campus');
    $this->addFieldMapping('field_staff_type', 'staff_type');
    $this->addFieldMapping('field_division_code', 'division_code');
    $this->addFieldMapping('field_division_name', 'division_name');
    $this->addFieldMapping('field_group_code', 'group_code');
    $this->addFieldMapping('field_group_name', 'group_name');
    $this->addFieldMapping('field_nist_org', 'group_name');
    $this->addFieldMapping('field_staff_bio', 'bio');
    $this->addFieldMapping('field_staff_bio:format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('field_staff_education', 'education');
    $this->addFieldMapping('field_staff_education:format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('field_staff_employment', 'employment');
    $this->addFieldMapping('field_staff_employment:format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('field_selected_publications_ref', 'selected_publications')
      ->separator(', ')
      ->sourceMigration('NistPublication');
    $this->addFieldMapping('field_display_name', 'displayname');
    $this->addFieldMapping('field_image', 'imageid')
      ->sourceMigration('NistUserImage');
    $this->addFieldMapping('field_image:file_class')
      ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_image:alt', 'imagealt');
    $this->addFieldMapping('field_contact_email_address', 'email_address');
    $this->addFieldMapping('field_address')
      ->defaultValue('US');
    $this->addFieldMapping('field_address:thoroughfare', 'address1');
    $this->addFieldMapping('field_address:premise', 'address2');
    $this->addFieldMapping('field_address:locality', 'city');
    $this->addFieldMapping('field_address:administrative_area', 'state');
    $this->addFieldMapping('field_address:postal_code', 'temp_mailing_address_postal_code');

    // Unmapped source fields
    $this->addUnmigratedSources(array(
      'id',
      'imagealt',
      'employment',
      'last_update_cs',
      'page_url',
      'zip4',
      'zip5'
    ));

    // Unmapped destination fields
    $this->addUnmigratedDestinations(array(
      'field_nist_org:source_type',
      'field_nist_org:create_term',
      'field_nist_org:ignore_case',
      'field_site',
      'field_site:format',
      'path',
      'field_campus:source_type',
      'field_campus:create_term',
      'field_campus:ignore_case',
      'revision_uid'
    ));
  }

  /**
   * @param $row
   * @return bool
   *
   * Implementation of Migration::prepareRow($row).
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) :
      return FALSE;
    endif;

    //$this->drushLog($row->people_id);

    $row->fic_staff_bio = $this->_process_body_text_native($row->fic_staff_bio);
    $row->fic_staff_education = $this->_process_body_text_native($row->fic_staff_education);
    $row->fic_staff_employment = $this->_process_body_text_native($row->fic_staff_employment);

    $row->temp_mailing_address_postal_code = $row->zip5 . '-' . $row->zip4;

    //$row->last_update = $this->_get_updated_time_for_force_updating($row->last_update);
  }

/*
  public function complete($node, stdClass $row) {

    $this->drushLog('IN THE COMPLETE!');

    if(strlen($row->page_url) > 0){

      $this->drushLog('PERSON GETTING A REDIRECT');

      // Setup url redirects for CSpot url to new url
      $redirect = new stdClass();
      $node_alias = url("user/{$node->uid}");
      $node_alias = ltrim($node_alias, '/');
      $oldPath = trim(str_replace("http://www.nist.gov/" , "" , $row->page_url));;

      drush_log('REDIRECT', $type = 'warning', $error = null);
      drush_log($row->page_url, $type = 'warning', $error = null);
      drush_log($oldPath, $type = 'warning', $error = null);
      drush_log($node_alias, $type = 'warning', $error = null);

      module_invoke(
        'redirect',
        'object_prepare',
        $redirect,
        array(
          'source' => $oldPath,
          'source_options' => array(),
          'redirect' => $node_alias,
          'redirect_options' => array(),
          'language' => LANGUAGE_NONE,
        )
      );

      module_invoke('redirect', 'save', $redirect);
    }

    return true;

    // save some memory
    //drupal_static_reset();
  }
  */
}
