<?php
/**
 * @file
 * Implementation of migration to import CommonSpot uploaded files into Drupal Files.
 */

class NistFileMigration extends NistMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Import of Commonspot files into Drupal files');
    $this->dependencies = array(
      'NistUser'
    );

    $fromdate = $this->getFromDate( 2 );

    // use this to get files from a specific date
    $list_url = "";

    $item_url = "";


    $fields = array(
      'changed' => 'When the file was last changed',
      'created' => 'When the file was created',
      'description' => 'File description',
      'dir' => 'Destination dir',
      'doctype' => 'File doctype',
      'filename' => 'File name',
      'name' => 'Name of the file',
      'originalfilename' => 'Commonspot\'s record of the original user\' filename ',
      'pageid' => 'Commonspot page ID of the document',
      'title' => 'File title',
      'uid' => 'User ID of the user that created the file',
      'url' => 'File url'
    );

    $this->highwaterField = array(
      'name' => 'changed', // Column to be used as highwater mark
      'alias' => 'f',
    );
/*
    $query = Database::getConnection('default')
      ->select('nist_files', 'f')
      ->fields('f', array('changed', 'created', 'description', 'dir', 'doctype',
        'filename', 'name', 'originalfilename', 'pageid', 'title', 'uid', 'url'))
      ->orderBy('changed', 'ASC');
    // Create a MigrateSource object, which manages retrieving the input data.
    $this->source = new MigrateSourceSQL($query);
*/
    $this->source = new MigrateSourceList(
      new MigrateListJSON($list_url),
      new MigrateItemJSON($item_url, array()),
      $fields
    );

    $this->destination = new MigrateDestinationFile();

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
    $this->addFieldMapping('value', 'url');
    $this->addFieldMapping('destination_dir', 'dir');
    $this->addFieldMapping('destination_file', 'migrate_filename');
    $this->addFieldMapping('preserve_files')
      ->defaultValue(true);
    $this->addFieldMapping('file_replace')
      ->defaultValue(MigrateFile::FILE_EXISTS_REUSE);
    $this->addFieldMapping('uid', 'uid')
      ->sourceMigration('NistUser');
    $this->addFieldMapping('timestamp', 'changed');


    $this->addUnmigratedDestinations(array(
      //'destination_file',
      'fid',
      'source_dir',
      'path',
      'urlencode'
    ));

    $this->addUnmigratedSources(array(

    ));
  }

  /**
   * Implementation of Migration::prepareRow($row).
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) :
      return FALSE;
    endif;

    //$row->title = str_replace('_', ' ', $row->title);

    // access rights taxonomy
    $theTID = $this->getPermissionTaxonomyDataBySubSiteURL( $row->dir );
    $row->permissionTIDBasedOnSubSite = $theTID;

    //$row->migrate_filename = strtolower($row->originalfilename);
    $row->migrate_filename = $row->originalfilename;

    //$section = ltrim($row->dir, '/');
    //$row->migrate_section = $this->_get_approval_section_from_url($section);

    $row->dir = rtrim($row->dir, '/');
    $row->dir = "public://documents{$row->dir}";

  }

  /**
   * Implementation of Migration::prepare
   */
  public function prepare($file, $row) {
    $file->field_access_rights = $this->_get_approval_section_fieldWithTID($row->permissionTIDBasedOnSubSite);

    //$file->uid = $this->_translate_user_id($row->uid);

    //print_r($row);
    //print_r($file);
  }

  public function complete($file, stdClass $row) {
    // Setup url redirects for CSpot url to new url
    $file_alias = url("file/{$file->fid}");
    $file_alias = ltrim($file_alias, '/');

    // subsite file url

    $dir = str_replace('public://documents', '', $row->dir);
    $old_url = "{$dir}/upload/{$row->originalfilename}";
    $old_url = ltrim($old_url, '/');
    $redirect = new stdClass();

    module_invoke(
      'redirect',
      'object_prepare',
      $redirect,
      array(
        'source' => $old_url,
        'source_options' => array(),
        'redirect' => $file_alias,
        'redirect_options' => array(),
        'language' => LANGUAGE_NONE,
      )
    );

    module_invoke('redirect', 'save', $redirect);
  }

  public function postRollback() {
    parent::postRollback();

    // Remove all redirects for files
    // $this->delete_redirects_from_path('cs_upload/');
    $this->delete_redirects_from_path('/upload/');
  }

  public function delete_redirects_from_path($path) {
    $query = db_select('redirect');
    $query->addField('redirect', 'rid');
    $query->condition('source', "%" . db_like($path) . "%", 'LIKE');
    $rids = $query->execute()->fetchCol();

    if ($rids) {
      return redirect_delete_multiple($rids);
    }
  }

  /**
   * I am called by convention by the Migrate framework after an import has
   * run.
   */
  public function postImport() {

    $this->logMessage('File Migrated', 'file.log');
  }
}
