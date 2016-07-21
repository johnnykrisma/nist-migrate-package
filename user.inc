<?php
/**
 * Implementation of migration from NIST peoplefinder and commonspot users
 * to Drupal users.
 *
 * The people_id is the CPR primary key and what ties the core Drupal User to
 * the Profile2 entity.
 *
 * @file
 * @author John Venable
 *
 * Modification Log:
 * Name               Date              Description
 * ============================================================================
 * John Venable       7/1/2014          Created
 * John Allen         2/1/2016          Modified for the new JSON being feed.
 */

class NistUserMigration extends NistMigration { 
  public function __construct($arguments) {

    parent::__construct($arguments);

    $this->description = t('Migrate NIST users from the PeopleFinder & 
         Commonspot. This will be the basic Drupal user profile.');

    $this->dependencies = array('NistUserImage');

    $fields = array(
      'changed' => 'I am the High Water mark for when a user last changed.',
      'email' => 'I am the users email address',
      'people_id' => 'I am the users CPR ID',
      'userid' => 'I am the users username',
      'active_yn' => 'is this user active? if N, then set status to 0'
    );
    
    $this->highwaterField = array(
      'name' => 'changed', // Column to be used as highwater mark
      'alias' => 'w',
    );

    $json_file = '';

    $this->source = new MigrateSourceList(
      new UserListJSON($json_file),
      new UserItemJSON($json_file, array()),
      $fields
    );

    // Destination uses a migrate user class
    $this->destination = new MigrateDestinationUser();

    $this->map = new MigrateSQLMap(
      $this->machineName,
      array(
        'people_id' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'PeopleFinder ID',
          'alias' => 'p',
        )
      ),
      MigrateDestinationUser::getKeySchema()
    );

    $this->addFieldMapping('name', 'userid')
      ->dedupe('users', 'name');
    $this->addFieldMapping('mail', 'email');
    $this->addFieldMapping('timezone')
      ->defaultValue('America/New_York');
    $this->addFieldMapping('pass', 'randompassword');
    $this->addFieldMapping('status', 'active_yn');
    $this->addFieldMapping('roles')
      ->defaultValue(DRUPAL_AUTHENTICATED_RID);
    $this->addFieldMapping('signature_format')
      ->defaultValue('filtered_html');
    $this->addFieldMapping('pathauto')
      ->defaultValue(0);

    // Unmapped destination fields shorthand for doing many at once
    $this->addUnmigratedDestinations(array(
      'signature',
      'created',
      'access',
      'role_names',
      'theme',
      'init',
      'data',
      'path',
      'login',
      'picture'
    ));
    
    // No unmapped source fields
    // $this->addUnmigratedSources(array('pagename'));
  }

  /**
   * Implementation of Migration::prepareRow($row).
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) :
      return FALSE;
    endif;

    // $this->drushLog($row->userid);

    $formattedDate = $this->convertColdFusionJSONDateTimeToPHPDateTime($row->changed);

    // add some time (one min) to the changed value so it is forced to be updated.
    $row->changed = $this->_get_updated_time_for_force_updating( $formattedDate );

    $chars = "";
    $password = substr( str_shuffle( $chars ), 0, 32 );
    $row->randompassword = $password;
  }

  /**
   * @param $node
   * @param \stdClass $row
   */
  public function prepare( $node, stdClass $row ) {
    // $this->drushLog('ENTRY~~~~~~~~~~~~~~~~~~~~~~~~~~~~START');

    $persistedNode = user_load_by_mail($node->mail);
    // $this->drushLog('INCOMING ROLES');
    // print_r($node->roles);

    // over write the updating nodes with the
    if (isset($persistedNode->roles)) {
      $node->roles = $persistedNode->roles;
    }
    
    $this->drushLog('SAVED ROLES');
    // print_r($node->roles);
    // print_r($row->randompassword);
    // $this->drushLog('ENTRY~~~~~~~~~~~~~~~~~~~~~~~~~~~~END');
  }
  
}
