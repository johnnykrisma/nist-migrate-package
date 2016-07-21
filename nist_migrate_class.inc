<?php
/**
 * @file
 * Base class for all migrations - encapsulates common behavior and provides
 * convenience methods to the real Migration classes.
 */
abstract class NistMigration extends Migration {

  /**
   * I am the Sub Site Org Chart Map variable.
   * @var string
   */
  private $subSiteOrgChartMap = '';

  /**
   * I am the Permission Taxonomy map variable.
   * @var string
   */
  private $permissionTaxonomy = '';

  /**
   * I am the Permission Taxonomy map variable.
   * @var string
   */
  private $cleanNewTopicsTaxonomy = '';

  /**
   * I am the Permission Taxonomy map tid lookup variable.
   * @var string
   */
  private $cleanNewTopicsTaxonomyTIDLookUp = '';

  /**
   * I am the Image Type map for Photo Gallery Migration
   * @var string
   */
  private $imageTypeMap = '';

  /**
   * I am the Sections look up JSON object
   * @var string
   */
  private $sectionLookUpMap = '';


  /**
   * I am the constructor
   * @param array $arguments
   */
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->team = array(
      new MigrateTeamMember('John Venable', 'john.venable@nist.gov', 'Migrator'),
      new MigrateTeamMember('Matthew Mendonca', 'matthew.mendonca@nist.gov', 'Migrator'),
      new MigrateTeamMember('John Allen', 'john.allen@nist.gov', 'Migrator'),
    );

    $this->setImageTypeMap();
    $this->setSubSiteOrgChartMap();
    $this->setPermissionTaxonomyMap();
    $this->setNewCleanTopicsTaxonomy();
    $this->setNewCleanTopicsTaxonomyTIDLookUp();
    $this->setSectionLookUpMap();

    //$this->logger = new MigrateLogger();
  }

  /**
   * I increment a date so Migrate will FORCE the updating of a migration
   * @param $time - I am the date/time to add to
   * @return DateTime
   */
  function _get_updated_time_for_force_updating( $time ) {

    $test = DateTime::createFromFormat('F, d Y H:i:s', $time);

    if ($test !== 0) {

      $newDate = date_add(
        date_create( $time ),
        date_interval_create_from_date_string('1750 seconds')
        //date_interval_create_from_date_string('0 seconds')
      );

      return  date_timestamp_get( $newDate );
      //return  $time;
    }
    else {
      return $time;
    }
    
  }

  /**
   * I return a list of TIDs based on 'classic' (old) NIST Taxonomy terms
   * @param $list - I am the list of terms to find TIDs of
   * @return string - I am the list of TIDs
   */
  public function getPublicationTIDListFromPublicationResearchField( $list ) {

    $termArray = $this->getPublicationPreparedResearchFieldArray( $list );
    $newTermArray = array();
    $TIDTermArray = array();

    // get the new terms as an array
    foreach ( $termArray as $item ) {

      $test = $this->getNewTopicTermFromOldTaxonomyTerm($item);

      if ( strlen( $test ) ) {
        array_push( $newTermArray, $test );
      }
    }

    // make an array of the TIDs of the new terms
    foreach ( $newTermArray as $term ) {
      array_push($TIDTermArray, $this->getTopicTIDByTerm($term));
    }

    // return them as a list of TIDs for insert
    return implode( ',', $TIDTermArray );
  }

  /**
   * I return the new taxonomy term based on the Old 'classic' term.
   * @param $term - I am the term to find the new term of
   * @return string
   */
  private function getNewTopicTermFromOldTaxonomyTerm( $term ) {

    $map = $this->getNewCleanTopicsTaxonomy();
    $result = '';

    foreach ( $map as $lookup ) {

      if ( $lookup['currentTerm'] === trim($term) ) {

        $result = $lookup['newMapping'];
        break;
      }
    }

    return $result;
  }

  /**
   * I return an array of cleaned up terms. I'm very specific for the
   * publications migration source.
   * @param $list - I am the list to clean up and explode
   * @return array - I am the array of the cleaned up list
   */
  private function getPublicationPreparedResearchFieldArray( $list ) {

    $list = str_replace( ',&nbsp;', ',', $list );
    $result = explode( ',', $list );

    return $result;
  }

  /**
   * I return true if I find the org in the org map array, false if not.
   * @param $org - I am the org name
   * @return bool
   */
  public function checkPublicationOrgIsValid( $org ) {
    $map = $this->getSubSiteOrgChartMap();
    $result = FALSE;

    foreach ( $map as $lookUp ) {

      if ( $lookUp['OrgChart Org'] === trim( $org ) ) {

        $result = TRUE;
        break;
      }
    }

    return $result;
  }

  /**
   * I read, parse and set the photogallerytypemap.json file to an internal
   * variable.
   */
  private function setImageTypeMap() {
    $string = file_get_contents("photogallerytypemap.json", TRUE);
    $this->imageTypeMap = json_decode( $string, TRUE );
  }

  /**
   * I return the photo gallery type map array data.
   * @return string
   */
  private function getImageTypeMap() {
    return $this->imageTypeMap;
  }

  /**
   * I return a new term from the photo gallery type data based on an old term
   * @param $term
   * @return string
   */
  public function getImageTypeFromMap( $term ) {

    $map = $this->getImageTypeMap();
    $result = '';

    foreach ($map as $lookUp) {

      if ($lookUp['oldTerm'] === $term) {

        $result = $lookUp['newTerm'];
        break;
      }
    }

    return $result;
  }

  /**
   * I log a message to the log files
   * @param $message
   * @param $file
   */
  public function logMessage($message, $file) {

    //$this->logger->logMessage($message, $file);
  }

  /**
   * I return a list of new taxonomy terms (NIST Topics) by old CommonSpot
   * taxonomy ID's
   * @param $idList
   * @return NULL|string
   */
  function getNewMappedTaxonomyTermsByOldIDList( $idList) {

    $idArray = explode(',', $idList);
    $map = $this->getNewCleanTopicsTaxonomy();

    // keeping this for no reason... were going to use
    // TID's from the clean_taxonomuy_crosswalk_tid_lookup.json
    // instead of terms directly
    $termArray = array();
    $termList = NULL;

    // these are the real things to use now.
    $TIDArray = array();
    $TIDList = NULL;
    $TID = NULL;

    foreach ($map as $lookUp) {

      foreach ($idArray as $commonSpotID) {

        if ( $lookUp['currentId'] === $commonSpotID ) {

          // always push the current new term
          array_push($termArray, $lookUp['newMapping']);

          // stupid code i know...
          $TID = $this->getTopicTIDByTerm($lookUp['newMapping']);

          if ($TID != NULL) {
            array_push($TIDArray, $this->getTopicTIDByTerm($lookUp['newMapping']));
          }

          $TID = NULL;

          // if there is a secondary new term push that to
          if ( strlen($lookUp['otherNewMapping']) !== 0 ) {

            array_push($termArray, $lookUp['otherNewMapping']);


            // stupid code i know...
            $TID = $this->getTopicTIDByTerm($lookUp['newMapping']);

            if ($TID != NULL) {
              array_push($TIDArray, $this->getTopicTIDByTerm($lookUp['newMapping']));
            }

            $TID = NULL;


            //array_push($TIDArray, $this->getTopicTIDByTerm($lookUp['otherNewMapping']));
          }

          break;
        }
      }
    }

    // bug fix, sometimes we were getting duplicate values.
    $cleanTIDArray = array_unique($TIDArray);

    $termList = implode( ',', $termArray );
    $TIDList = implode( ',', $cleanTIDArray );

    //drush_log($TIDList, $type = 'warning', $error = NULL);

    return $TIDList;
  }

  /**
   * I return a TID of a NIST Topic taxonomy term.
   * @param $term
   * @return NULL
   */
  private function getTopicTIDByTerm($term) {
    $map = $this->getNewCleanTopicsTaxonomyTIDLookup();
    $result = NULL;

    foreach ($map as $lookUp) {
      if ( $lookUp['name'] === $term ) {
        $result = $lookUp['tid'];
        break;
      }
    }

    return $result;
  }


  /**
   * I set the New Topics Taxonomy json map.
   */
  private function setNewCleanTopicsTaxonomy() {

    $string = file_get_contents("clean_taxonomy_crosswalk.json", TRUE);
    $this->cleanNewTopicsTaxonomy = json_decode( $string, TRUE );
  }

  /**
   * I return the New Topics Taxonomy that was cleaned up.
   * @return mixed
   */
  private function getNewCleanTopicsTaxonomy() {

    return $this->cleanNewTopicsTaxonomy;
  }


  /**
   * I set the New Topics Taxonomy json map.
   */
  private function setNewCleanTopicsTaxonomyTIDLookup() {

    $string = file_get_contents("clean_taxonomy_crosswalk_tid_lookup.json", TRUE);
    $this->cleanNewTopicsTaxonomyTIDLookUp = json_decode( $string, TRUE );
  }

  /**
   * I return the New Topics Taxonomy that was cleaned up.
   * @return mixed
   */
  private function getNewCleanTopicsTaxonomyTIDLookup() {

    return $this->cleanNewTopicsTaxonomyTIDLookUp;
  }


  /**
   * I insert a redirect into the redirect module
   * @param $node
   * @param \stdClass $row
   */
  public function addRedirect($node, stdClass $row) {

    // Setup url redirects for CSpot url to new url
    $redirect = new stdClass();
    $node_alias = url("node/{$node->nid}");
    $node_alias = ltrim($node_alias, '/');
    $oldPath = trim($row->path);

    // if we don't have a .cfm add it.
    if ( substr($oldPath, -4) != '.cfm') {
      $oldPath = trim($row->path . '.cfm');
    }

    // if there is a leading '/' remove it
    if ( $oldPath[0] === '/') {
      $oldPath = ltrim($oldPath, '/');
    }

    drush_log('REDIRECT', $type = 'warning', $error = NULL);
    drush_log($row->path, $type = 'warning', $error = NULL);
    drush_log($oldPath, $type = 'warning', $error = NULL);
    drush_log($node_alias, $type = 'warning', $error = NULL);


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

    return TRUE;
  }


  /**
   * I add an entry into the workbench_moderation_node_history table. This is
   * necessary because with out it, and revisions and workbench being enabled
   * on migrated content users cant revert without createing a new draft.
   * @param $node
   */
  public function addWorkBenchModerationNodeHistoryEntry( $node ) {

    global $user;

    // Build a history record.
    if (isset($node->workbench_moderation_state_new)) {
      $new_revision = (object) array(
        'from_state' => $node->workbench_moderation_state_current,
        'state' => $node->workbench_moderation_state_new,
        'nid' => $node->nid,
        'vid' => $node->vid,
        'uid' => $node->uid,
        'current' => 1,
        'published' => 1,
        'stamp' => $_SERVER['REQUEST_TIME'],
      );

      // Save the node history record.
      drupal_write_record('workbench_moderation_node_history', $new_revision);
    }
  }


  /**
   * I load an entities by node ids and delete their redirects form the redirect
   * table.
   * @param $nids - I am the array of node IDs
   * @param $type - I am what content type
   */
  public function prepareRollback($nids = array(), $type = '') {
    //parent::prepareRollback();

    /*
    $entityArray = entity_load($type, $nids);

    foreach ($entityArray as &$entity) {
      // Remove all redirects for events
      $this->_delete_redirects_from_path($entity->path);
    }
    */
  }

  /**
   * I set the Sub Site Org Chart map as a variable.
   */
  private function setSubSiteOrgChartMap() {

    $string = file_get_contents("map.json", TRUE);
    $this->subSiteOrgChartMap = json_decode( $string, TRUE );
  }

  /**
   * I return the Sub Site Org Chart map JSON
   * @return mixed
   */
  private function getSubSiteOrgChartMap() {

    return $this->subSiteOrgChartMap;
  }


  /**
   * I set the Permission Taxonomy Map
   */
  private function setPermissionTaxonomyMap() {

    $jsonString = file_get_contents("permissionSubSiteCrosswalk.json", TRUE);
    $this->permissionTaxonomy = json_decode( $jsonString, TRUE );
  }

  /**
   * I return the Permission Taxonomy SubSite mapping object
   * @return mixed
   */
  function getPermissionTaxonomyMap() {

    return $this->permissionTaxonomy;
  }

  /**
   * I return the Drupal Taxonomy ID of from the Permissions taxonomy based on
   * what sub site was associated with it when it was originally created.
   * @param $subSiteURL - string - I am the CommonSpot Sub Site URL
   * @return string - I am the Taxonomy ID found
   */
  function getPermissionTaxonomyDataBySubSiteURL( $subSiteURL ) {

    $map = $this->getPermissionTaxonomyMap();
    $result = NULL;

    $lastChar = substr($subSiteURL, -1, 1);
    if ($lastChar != '/') {
      $subSiteURL = $subSiteURL . '/';
    }

    // if were getting '/' then assign to pao...
    if ($subSiteURL === '/') {
      drush_log('ITS EMPTY!', $type = 'warning', $error = NULL);
      drush_log($subSiteURL, $type = 'warning', $error = NULL);
      drush_log('will assign to PAO', $type = 'warning', $error = NULL);

      return '247491';
    }

    foreach ( $map as $key => $value ) {

      $valueToCheck = $key . '/';

      if ( trim($valueToCheck) === trim($subSiteURL) ) {
        $result = $value['tid'];
        break;
      }
    }

    // if we didn't find it recursively call ourselves till we do.
    if ($result === NULL) {

      $newCheck = $this->listDeleteLastElement($subSiteURL, '/', FALSE);

      drush_log('NEW CHECK', $type = 'warning', $error = NULL);
      drush_log($newCheck, $type = 'warning', $error = NULL);

      if ( $newCheck === ' ' || $newCheck != '/' || strlen($newCheck) != 0) {

        return ( $this->getPermissionTaxonomyDataBySubSiteURL($this->listDeleteLastElement($newCheck, '/', TRUE)) );

      }
      else {

        drush_log('WEIRD', $type = 'warning', $error = NULL);
        drush_log($newCheck, $type = 'warning', $error = NULL);

        return '';
      }
    }
    else { // we found the match so return it.

      drush_log('FOUND IT', $type = 'warning', $error = NULL);
      drush_log($subSiteURL, $type = 'warning', $error = NULL);
      drush_log($result, $type = 'warning', $error = NULL);

      return $result;
    }
  }

  /**
   * I return the sub site path of a CommonSpot URL
   * @param $name
   * @return string
   */
  function getSubSiteFromPath( $name ) {

    // didn't get anything so abort and return an empty string.
    if ( $name === '/' or $name === '' ) {
      return '';
    }

    // Be defensive. Need to add a '/' to the top of the path.
    if (substr($name, 0, 1) != '/') {
      $name = '/' . $name;
    }

    $result = $this->listDeleteLastElement( $name, '/', TRUE );

    return $result;
  }

  /**
   * I return a Organization, Division or Group name based on sub site path, or
   * I crawl up the /org/div/group URL string till I find one. Else I will return
   * an empty string if nothing found
   * @param $name - I am the CommonSpot Sub Site Path to find.
   * @return string - I am the mapped Organization, Division or Group name.
   */
  function _get_org_chart_name( $name ) {

    // didn't get anything so abort and return an empty string.
    if ( $name === '/' or $name === '' ) {
      return '';
    }

    // Be defensive. Need to add a '/' to the top of the path.
    if (substr($name, 0, 1) != '/') {
      $name = '/' . $name;
    }

    $check = $this->listDeleteLastElement( $name, '/', TRUE );
    $result = $this->getValueFromMap( $check  );

    // if NULL from the result, recursively call myself after removeing the last
    // element in the string.
    if ( $result === NULL ) {

      $newCheck = $this->listDeleteLastElement( $check, '/', FALSE );
      return ( $this->_get_org_chart_name(  $this->listDeleteLastElement( $newCheck, '/', TRUE )  ) );

    }
    else { // we found the match so return it.

      return $result;
    }
  }

  /**
   * I return a Organization, Division or Group name based on sub site path.
   * @param $value string - I am the CommonSpot Sub Site Path to find.
   * @return NULL|string
   */
  private function getValueFromMap( $value ) {

    $map = $this->getSubSiteOrgChartMap();
    $result = NULL;

    foreach ( $map as $key => $val ) {
      if ( $value === $val['SubSiteURL'] ) {
        $result = trim( $val['OrgChart Org'] );
        break;
      }
    }

    return $result;
  }

  /**
   * I remove the last element in a string list.
   * @param $list string - I am the list to work with.
   * @param $delimiter string - I am the delimiter of the list.
   * @param $addTrailingDelimiter boolean - should I add the delimiter to the end?
   * @return string
   */
  private function listDeleteLastElement( $list, $delimiter, $addTrailingDelimiter ) {

    $checkArray = explode( $delimiter, $list  );
    $pop = array_pop( $checkArray );
    $result = trim(implode( $delimiter, $checkArray ));

    if ( $addTrailingDelimiter ) {
      $result = $result . $delimiter;
    }

    return $result;
  }

  /**
   * @param $uid
   * @return people_id from a commonspotid
   */

  // Should probably prefix this with an _ to follow convention
  public function _translate_user_id($uid) {
    $people_id_query = db_select('nist_people_commonspot_users', 'u')
      ->fields('u', array('people_id'))
      ->condition('commonspotid', $uid, '=')
      ->execute()
      ->fetchAssoc();

    return $people_id_query['people_id'];
  }

  /**
   * Helper function to cleanup cspot url's.
   */
  public function _process_path($path) {
    if (empty($path) || !isset($path)) :
      return '';
    endif;

    $path = ltrim($path, '/');
    $path = str_replace('.cfm', '', $path);
    //$path = str_replace('/index', '', $path);

    return $path;
  }

  /**
   * @param $ou
   * @return a comma-delim list
   *
   * Takes the short form OU syntax like mml or pml and turns it into the proper
   * name for inputting into the organization taxonomy.
   */
  public function _process_ou($ou) {
    if (empty($ou) || !isset($ou)) :
      return '';
    endif;

    $aorg = explode(',', $ou);
    $orglist = '';

    foreach ($aorg as $value) {
      switch ($value) {
        case 'cnst':
          $orglist .= 'Center for Nanoscale Science and Technology';
          break;
        case 'ctl':
          $orglist .= 'Communications Technology Laboratory';
          break;
        case 'el':
        case 'bfrl':
        case 'mel':
          $orglist .= 'Engineering Laboratory';
          break;
        case 'itl':
          $orglist .= 'Information Technology Laboratory';
          break;
        case 'mml':
        case 'cstl':
        case 'mgi':
        case 'msel':
          $orglist .= 'Material Measurement Laboratory';
          break;
        case 'ncnr':
          $orglist .= 'NIST Center for Neutron Research';
          break;
        case 'pml':
        case 'physlab':
        case 'eeel':
          $orglist .= 'Physical Measurement Laboratory';
          break;
        default:
          $orglist = '';
      }

      // put a comma between names
      $orglist .= ",";
    }

    // remove trailing delimter
    $orglist = rtrim($orglist, ',');

    return $orglist;
  }

  /**
   * @param $path
   * @return takes a url path from commonspot and returns the topmost subsite.
   */
  public function _org_from_subsite($path) {
    if (empty($path) || !isset($path)) :
      return '';
    endif;

    $pieces = explode("/", $path);

    $top_subsite = $pieces[1];

    return $this->_process_ou($top_subsite);
  }


  /**
   * @param $campus
   * @return a comma-delim list of campi
   *
   * Much like the above function, this takes the short form OU syntax like JILA
   * or HML and turns it into proper campus names.
   */
  public function _process_campus($campus) {
    if (empty($campus) || !isset($campus)) :
      return '';
    endif;

    $acampus = explode(',', $campus);
    $campuslist = '';

    foreach ($acampus as $value) {
      switch ($value) {
        case 'Boulder':
          $campuslist .= 'Boulder';
          break;
        case 'BNL':
          $campuslist .= 'Brookhaven National Laboratory';
          break;
        case 'Gaithersburg':
        case 'Gathithersburg':
          $campuslist .= 'Gaithersburg';
          break;
        case 'HML':
          $campuslist .= 'Hollings Marine Laboratory';
          break;
        case 'CARB/UMBI':
        case 'IBBR':
        case 'IBBR/UMBI':
          $campuslist .= 'Institute for BioScience and BioTech Research';
          break;
        case 'JILA':
          $campuslist .= 'Joint Institute for Laboratory Astro-Physics';
          break;
        case 'JQI':
          $campuslist .= 'Joint Quantum Institute';
          break;
        default:
          $campuslist = '';
      }

      // put a comma between names
      $campuslist .= ",";
    }

    // remove trailing delimter
    $campuslist = rtrim($campuslist, ',');

    return $campuslist;
  }


  /**
   * Parse a block of html to fix img src's, files, etc. This version uses php's
   * DOMDocument (from dom extension)
   * @param $html
   * @param string $contentType
   * @return mixed|string
   *
   */
  public function _process_body_text_native( $html, $contentType = "" ) {

    if (empty($html)) :
      return $html;
    endif;

    // Build out migrate map for files
    $migrate_map_files = array();

    $migrate_map_files_query = db_select('migrate_map_nistfile', 'mm_nf')
      ->fields('mm_nf', array('sourceid1', 'destid1'))
      ->execute();

    while ($record = $migrate_map_files_query->fetchAssoc()) :
      $migrate_map_files[$record['sourceid1']] = $record['destid1'];
    endwhile;

    libxml_use_internal_errors(TRUE);

    $dom = new DOMDocument();
    $dom->loadHTML($html);


    // ************************ TEST CODE ************************//
    //$pathOne = realpath(dirname(__FILE__)). '/' . 'htmlout1.htm';
    //$pathTwo = realpath(dirname(__FILE__)). '/' . 'htmlout2.htm';
    //file_put_contents( $pathOne, $dom->saveHTML() );


    //  ************ IMGAGE PROCESSING ************** //
    $img_node_list = $dom->getElementsByTagName("img");
    foreach ($img_node_list as $img_node) {

      $src = $img_node->getAttribute('src');

      if (substr($src, 0, strlen('/')) !== '/') :
        continue;
      endif;

      $src = str_replace('images/', '', strtolower($src));
      $src = "/sites/default/files/images{$src}";

      $img_node->setAttribute('src', $src);

      // ********* Try to get a fid... somehow... *********** //

      // first by id attribute
      $imageFID = NULL;
      if ($img_node->hasAttribute('id')) {

        $imageFID = $this->getFidByCommonSpotImageID($img_node->getAttribute('id'));

        $this->drushLog("OMG OMG OMG OMG!!!!!! FID FROM ATTRIBUTE!!! {$imageFID}");
      }

      // second, take a stab at finding a single image form the db by name
      if (!$imageFID) {
        $this->drushLog('IM NOT SO COOL!!!! TO TRY TO GET FID FROM IMAGE FILE NAME!');
        $imageFID = $this->attemptToGetImageIDByFilePath($src);
        $this->drushLog("EAT IT!!!!!! FID FROM FILE NAME!!! {$imageFID} EAT IT!!!!!");
      }

      // if any of the two above method calls set a fid, but it on the node as
      // an attribute
      if ($imageFID) {
        $this->drushLog("WE HAVE A FID!!!!!!!!!!!!!! WE HAVE AN FID!!!!!!!!!!");
        $img_node->appendChild($dom->createAttribute('fid'));
        $img_node->setAttribute('fid', $imageFID);
      }
    }



    //  ************ LINK PROCESSING ************** //
    $link_node_list = $dom->getElementsByTagName("a");

    foreach ($link_node_list as $link_node) :

      $href = $link_node->getAttribute('href');

      // Process JS links
      if (strpos($href, 'javascript:HandleLink') !== FALSE) :
        $href_url = explode('@', $href);

        if (isset($href_url[1])) :
          $href_url = $href_url[1];

          if (strpos($href_url, "CP___PAGEID") === 0) :
            if (strpos($href_url, ".cfm") !== FALSE) :
              $href_url = explode(',', $href_url);
              $href_url = $href_url[1];
              $link_node->setAttribute('href', $href_url);
            else :
              $href_url = explode(',', $href_url);
              $link_node->setAttribute('id', $href_url[0]);
              $href_url = $href_url[1];
            endif;
          endif;

          $href = $href_url;
        endif;
      endif;

      $link_node->setAttribute('href', $href);

      // Skip external links
      if (substr($href, 0, strlen('/')) !== '/') :
        continue;
      endif;

      $id = $link_node->getAttribute('id');

      // File && Commonspot Internal Links links -
      // replace the old link with the new drupal link
      if (strpos($id, 'CP___PAGEID=') !== FALSE) :
        $page_id = $id;
        $page_id = str_replace('CP___PAGEID=', '', $page_id);
        $page_id = str_replace('|', '', $page_id);

        // if is a FILE link...
        if ( isset($migrate_map_files[$page_id]) ) {

          $file_url = "file/{$migrate_map_files[$page_id]}";
          $file_url = url($file_url);
          $link_node->setAttribute('href', $file_url);

          $link_node->removeAttribute('id');

        }
        else {

          // this link is a 'normal' (if there can be such a thing)
          // CommonSpot link
          $commonSpotPageID = explode(',', $page_id)[0];
          $nodeID = $this->getNodeIDFromMigrateByCommonSpotPageID($commonSpotPageID);

          // if we got a node id from migrate...
          if ( isset($nodeID) ) {

            // was it a user or node? SET THE HREF
            if ($this->getMachineName() === 'NistUser') {
              $href = drupal_get_path_alias('user/' . $nodeID);
            }
            else {
              $href = drupal_get_path_alias('node/' . $nodeID);
            }

            // is there an anchor link? \#(.*?)\,
            $pattern = '/\#(.*?)\,/';
            $subject = $page_id;
            preg_match($pattern, $subject, $matches);

            // we had a match APPEND THE HREF
            if ( isset($matches[1]) ) {
              $href = $href . '#' . $matches[1];
            }

          }
          else {

            // we didn't get a NodeID from migrate so check the redirect table
            $redirectToCheck = explode('#', $href)[0];
            $redirectToCheck = ltrim($redirectToCheck, '/');

            // fist try to get the redirect
            $theRedirect = $this->getRedirectFromCommonSpotPath( $redirectToCheck );

            if (isset($theRedirect)) {
              $href = $theRedirect;
            }
            else { // if this happens, we will have a broken link
              $href = $redirectToCheck;
            }
          }

          $link_node->setAttribute('href', '/' . $href);
          $link_node->removeAttribute('id');
        }

      // All other links
      else :
        $node_id = $this->_get_node_id_from_legacy_url($href);

        if ($node_id) :
          $href = "/node/{$node_id}";
        else :

          $href = str_replace('#__topdoc__', '#main-content', $href);
        endif;

        $link_node->setAttribute('href', $href);
      endif;
    endforeach;

    //NEW - changing table w/ image captions to figures
    $dom = $this->changeImageTableToFigure( $dom, $contentType );

    // change images to media tokens
    $dom = $this->changeImageNodeToMediaToken( $dom );

    $html = $dom->saveHTML();

    // Decode characters
    $html = $this->_decode_characters($html);

    $html = str_replace("<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\" \"http://www.w3.org/TR/REC-html40/loose.dtd\">\n", '', $html);
    $html = str_replace('<html>', '', $html);
    $html = str_replace('</html>', '', $html);
    $html = str_replace('<body>', '', $html);
    $html = str_replace('</body>', '', $html);
    $html = str_replace('<p>&nbsp;</p>', '', $html);
    $html = str_replace("<br></br>", "<br>", $html);

    $html = trim($html);

    // ************************ TEST CODE ************************//
    //file_put_contents( $pathTwo, $html );

    return $html;
  }


  /**
   * I return a files ID if you give me the path
   * @param $filePath - path to the file or the file name
   * @return int
   */
  private function attemptToGetImageIDByFilePath($filePath) {

    // get the file name of the file path...
    $fileName = end(explode('/', strtolower($filePath)));

    $query = db_select('file_managed');
    $query->addField('file_managed', 'fid');
    $query->condition('filename', (trim($fileName)), '=');

    $result = $query->execute()->fetchAllAssoc('fid');

    // if we only have one record COOL!
    if (count($result) === 1) {

      // dynamic keys on the query object... pita... but set it to the first one
      foreach ($result as $theID) {
        $result = $theID->fid;
        break;
      }

      return $result;

    }
    else { // multiples were found e.g.: 1.jpg, can't say which one is right.

      return NULL;
    }
  }








  /**
   * I return the CommonSpot Image ID from the CommonSpot generated id attribute
   * @param string $attributeString
   * @return string
   */
  public function getFidByCommonSpotImageID($attributeString = '') {

    $result = NULL;
    $pattern = '/(?<=CPIMAGE:)(.*?)(?=\|)/';
    $foo = preg_match($pattern, $attributeString, $matches);

    if ( $foo ) {

      $commonSpotImageID = $matches[1];

      $query = db_select('migrate_map_nistimage', 'n')
        ->fields('n')
        ->condition('sourceid1', $commonSpotImageID, '=')
        ->execute()
        ->fetchAssoc();

      $result = $query['destid1'];
    }

    return $result;
  }


  /**
   * I replace all image nodes that have a proper FID attribute with a media
   * token.
   * @param $dom
   * @return mixed
   */
  public function changeImageNodeToMediaToken( $dom ) {

    $this->drushLog('WE ARE GOING TO CHANGE............');

    $imgNodeArray = $dom->getElementsByTagName("img");

    $i = $imgNodeArray->length - 1;

    while ($i > -1) {

      $image = $imgNodeArray->item($i);

      if ( $image->hasAttribute('fid') ) {

        $altText = $image->getAttribute('alt');
        $fid = $image->getAttribute('fid');
        $view_mode = 'medium';
        $height = '';
        $width = '';


        // if image width is less than 220 make the media token small
        if ($image->hasAttribute('width') && $image->getAttribute('width') < 220) {
          $view_mode = 'small';
        }

        // if image width is larger than 480 make the media token large
        if ($image->hasAttribute('width') && $image->getAttribute('width') > 480) {
          $view_mode = 'large';
        }

        // if image width is larger than 960 make the media token EXTRA LARGE
        if ($image->hasAttribute('width') && $image->getAttribute('width') > 960) {
          $view_mode = 'extra_large';
        }

        if ($image->hasAttribute('width') && strlen( $image->getAttribute('width') )) {
          $width =  $image->getAttribute('width');
        }

        if ($image->hasAttribute('height') && strlen( $image->getAttribute('height') )) {
          $height =  $image->getAttribute('height');
        }

        $mediaToken = $this->getMediaToken($fid, $altText, '' , $view_mode, $height, $width);

        $this->drushLog('::::::::::::::::::THE MEDIA TOKEN:::::::');
        $this->drushLog($mediaToken);
        $this->drushLog('::::::::::::::::::THE MEDIA TOKEN:::::::');

        $mediaTokenNode = $dom->createTextNode($mediaToken);
        $image->parentNode->replaceChild($mediaTokenNode, $image);
      }

      $i--;
    }

    return $dom;
  }


  /**
   * @param $fid
   * @param string $altText
   * @return string
   */
  public function getMediaToken($fid,
                                $altText = "",
                                $captionText = "",
                                $view_mode = "medium",
                                $height = "",
                                $width = "") {

    /* here is what the toke should look like...
    [
      [
        {
          "fid":"259041",
          "view_mode":"medium",
          "fields":
          {
            "format":"medium",
            "field_image_caption[und][0][value]":"Ion Storage Group members Spring 2016"
          },
          "type":"media",
          "link_text":NULL,
          "attributes":
          {
            "alt":"Ion Storage Group members Spring 2016",
            "title":"Ion Storage Group members Spring 2016",
            "height":"274",
            "width":"480",
            "class":"float-left media-element"
          }
        }
      ]
    ]
    */

    $class = 'float-left media-element migrated';

    if ($view_mode === 'large' ||  $view_mode === 'extra_large') {
      $class = 'media-element migrated';
    }

    if ($view_mode === 'small') {
      $class = 'media-element migrated';
    }

    if (strlen($height) && strlen($width)) {

      $result = array(
        array(
          array(
            'fid' => $fid,
            'view_mode' => $view_mode,
            'fields' => array(
              'format' => $view_mode,
              'field_image_caption[und][0][value]' => trim($captionText)
            ),
            'type' => 'media',
            'link_text' => NULL,
            'attributes' => array(
              'alt' => $altText,
              'title' => $altText,
              'height' => $height,
              'width' => $width,
              'class' => $class
            ),
          )
        )
      );

    }
    elseif ( strlen($width) && !strlen($height)) { // no height

      $result = array(
        array(
          array(
            'fid' => $fid,
            'view_mode' => $view_mode,
            'fields' => array(
              'format' => $view_mode,
              'field_image_caption[und][0][value]' => trim($captionText)
            ),
            'type' => 'media',
            'link_text' => NULL,
            'attributes' => array(
              'alt' => $altText,
              'title' => $altText,
              'width' => $width,
              'class' => $class
            ),
          )
        )
      );

    }
    else { // no width or height given

      $result = array(
        array(
          array(
            'fid' => $fid,
            'view_mode' => $view_mode,
            'fields' => array(
              'format' => $view_mode,
              'field_image_caption[und][0][value]' => trim($captionText)
            ),
            'type' => 'media',
            'link_text' => NULL,
            'attributes' => array(
              'alt' => $altText,
              'title' => $altText,
              'class' => $class
            ),
          )
        )
      );

    }

    $result = json_encode($result);

    return $result;
  }

  /**
   * I am a utility function for more HTML cleanup.
   * @param $html
   * @return mixed
   */
  public function _decode_characters($html) {
    if (empty($html)) :
      return $html;
    endif;

    $html = str_replace('Â', '', $html);
    $html = str_replace('<p>&nbsp;</p>', '', $html);
    $html = str_replace('<p> </p>', '', $html);
    $html = str_replace("<p>\n</p>", '', $html);
    $html = str_replace('&#10;', ' ', $html);
    $html = str_replace('&#160;', ' ', $html);
    $html = str_replace('&#8211;', '–', $html);
    $html = str_replace('&#8212;', '—', $html);
    $html = str_replace('&#252;', 'ü', $html);
    $html = str_replace('&#233;', 'é', $html);

    return $html;
  }

  /**
   * I return a Node ID based on its corresponding CommonSpot Page ID
   *
   * @param int $pageID
   * @return mixed
   */
  public function getNodeIDFromMigrateByCommonSpotPageID($pageID = 0) {

    // no matter what were going to need to loop over all the migrate tables
    // to find the node ID. So we put them in some order that we think will
    // the least amount of db queries.
    $migrateMap = [
      'NistBlankPages',
      'NistCommonspotPages',
      'NistOu',
      'NistDivision',
      'NistGroup',
      'NistNews',
      'NistEvent',
      'NistAward',
      'NistInstrument',
      'NistSoftware',
      'NistTimeline',
      'NistUser'
    ];

    $result = FALSE;

    foreach ($migrateMap as $migrate) {
      $tableName = 'migrate_map_' . strtolower( $migrate );

      $query = db_select($tableName, 'n')
        ->fields('n')
        ->condition('sourceid1', $pageID, '=')
        ->execute()
        ->fetchAssoc();

      $result = $query['destid1'];

      if (isset($result)) {
        break;
      }
    }

    return $result;
  }

  /**
   * Helper function to return the node id from legacy cspot url.
   */
  public function _get_node_id_from_legacy_url($href) {
    $node_id = FALSE;
    $query = new EntityFieldQuery();

    $query->entityCondition('entity_type', 'node')
      ->fieldCondition('field_legacy_url ', 'value', $href, '=')
      ->addMetaData('account', user_load(1));

    $result = $query->execute();

    if (isset($result['node'])) :
      // doing a loop since it's an array, but there should
      // only ever be one node for a given path
      // make more robust if need be
      foreach ($result['node'] as $nid => $node_data) :
        $node_id = $nid;
      endforeach;
    endif;

    return $node_id;
  }

  /**
   * Helper function to return the approval section based on URL string.
   */
  public function _get_approval_section_from_url( $url ) {
    $approval_section = '';

    // Should change this - something like WWW
    // See INET-202 issue in inet jira project
    $default_approval_group = 'INET';

    if (empty($url) || !isset($url)) {
      return $approval_section;
    }

    $url = explode('/', $url);

    if (is_array($url) && isset($url[0]) && is_string($url[0])) {
      $approval_section = $url[0];
    }

    //$approval_section_vocab = taxonomy_vocabulary_machine_name_load('permissions_groups');
    $approval_section_vocab = taxonomy_vocabulary_machine_name_load('permissions');
    $approval_section_terms = entity_load('taxonomy_term', FALSE, array('vid' => $approval_section_vocab->vid));

    $url_approval_section_is_in_vocab = FALSE;

    foreach ($approval_section_terms as $tid => $term) {

      $term_name = strtolower($term->name);

      if ($term_name === $approval_section) {
        $url_approval_section_is_in_vocab = TRUE;
      }
    }

    if ($url_approval_section_is_in_vocab) {

      return $approval_section;

    }
    else {

      return $default_approval_group;
    }
  }

  /**
   * Helper function to return the field structure for the approval section taxonomy field.
   */
  public function _get_approval_section_field($approval_section) {
    $approval_tid = $this->_get_approval_section_tid_from_name($approval_section);

    return array(
      LANGUAGE_NONE => array(
        array(
          'tid' => $approval_tid,
        )
      )
    );
  }

  /**
   * Helper function to return the field structure for the approval section taxonomy field using the TID.
   */
  public function _get_approval_section_fieldWithTID($approvalTID) {

    return array(
      LANGUAGE_NONE => array(
        array(
          'tid' => $approvalTID,
        )
      )
    );
  }

  /**
   * Helper function to get a term id from name.
   */
  public function _get_approval_section_tid_from_name($term_name, $vocab = 'permissions_groups') {
    $term = taxonomy_get_term_by_name($term_name, $vocab);

    // only get the first value
    $term = reset($term);

    return $term->tid;
  }

  /**
   * Helper function to get drupal approval section from org term.
   */
  public function _get_section_from_org($ids) {
    $ids = explode(',', $ids);
    $sections = '';

    foreach ($ids as $key => $id) :
      if (isset($this->_org_to_section[$id])) :
        $sections .= "{$this->_org_to_section[$id]['section']},";
      endif;
    endforeach;

    if (empty($sections)) :
      return FALSE;
    else :
      return rtrim($sections, ',');
    endif;
  }

  /**
   * Helper function to delete redirects based on path.
   */
  public function _delete_redirects_from_path($path) {
    $query = db_select('redirect');
    $query->addField('redirect', 'rid');
    $query->condition('source', "%" . db_like($path) . "%", 'LIKE');
    $rids = $query->execute()->fetchCol();

    if ($rids) {
      return redirect_delete_multiple($rids);
    }
  }

  /**
   * Helper function to get the last json decoder error.
   */
  public function _get_json_error() {
    $json_error = '';

    switch (json_last_error()) :
      case JSON_ERROR_NONE:
        break;

      case JSON_ERROR_DEPTH:
        $json_error = 'JSON error: Maximum stack depth exceeded';
        break;

      case JSON_ERROR_STATE_MISMATCH:
        $json_error = 'JSON error: Underflow or the modes mismatch';
        break;

      case JSON_ERROR_CTRL_CHAR:
        $json_error = 'JSON error: Unexpected control character found';
        break;

      case JSON_ERROR_SYNTAX:
        $json_error = 'JSON error: Syntax error, malformed JSON';
        break;

      case JSON_ERROR_UTF8:
        $json_error = 'JSON error: Malformed UTF-8 characters, possibly incorrectly encoded';
        break;

      default:
        $json_error = 'JSON error: Unknown error';
        break;
    endswitch;

    return $json_error;
  }

  /**
   * Helper function to clean up json from cold fusion web service and save as file.
   * This will ideally be implemented on the CF side in the future.
   *
   * Returns the file path for the saved file or FALSE on failure.

  public function _save_clean_json($json_file, $destination_file) {
  $json = file_get_contents($json_file);
  $saved_file = FALSE;

  if ($json) :
  $json = utf8_encode($json);
  $data = drupal_json_decode($json);

  if ($data && !$this->_get_json_error()) :
  $saved_file = __DIR__."/{$destination_file}";
  file_put_contents($saved_file, $json, FILE_USE_INCLUDE_PATH | LOCK_EX);
  endif;
  endif;

  return $saved_file;
  }*/


  /**
   * I change the table->tdbody->tr-td photo and caption HTML into a <div>
   * with child elements.
   * @param $dom
   * @param string $contentType
   * @return mixed
   */
  function changeImageTableToFigure( $dom, $contentType = "" ) {

    $classNameToFlagForDelete = 'table-to-delete';

    $clonedImageObject = array();
    $clonedImageObject['image'] = NULL;
    $clonedImageObject['caption']= NULL;
    $clonedImageObject['id']= NULL;

    $replacementArray = array();
    $numberOfTablesToReplace = 0;

    $tableListObject = $dom->getElementsByTagName( "table" );

    foreach ( $tableListObject as $tableElement  ) {

      $imageCheck = FALSE;

      foreach ($tableElement->childNodes as $children => $firstChild) {

        // find and figure out if this is the 'common' image in news items
        if ($firstChild->tagName === 'tbody') {

          // add a class so we can replace this table later with the picked out
          // fig element
          $tableElement->setAttribute('class', $classNameToFlagForDelete);

          // we have a tbody with 2 trs
          if ($firstChild->childNodes->length === 2) {

            // make a list of the TR's
            $trList = $firstChild->childNodes;

            foreach ($trList as $row) {

              // check that we only have ONE td
              $td = $row->getElementsByTagName('td');

              // if we have 1 check that a child is an image.
              if ($td->length === 1) {

                $td = $row->firstChild;

                $imgCheck = $td->getElementsByTagName('img');

                // bang! we have an image in the first td in the first tr. lets now
                // assume that this is a table that is wrapping images and their
                // captions.
                if ( $imgCheck->length === 1 ) {

                  //$this->drushLog("BURP BURP BURP***********");
                  //print_r($imageCheck->length);

                  $img = $td->firstChild;
                  // cant seem to over write once the above image node is set to
                  // a variable... so do this. DOMDocument is a
                  $check = $td->firstChild;

                  // did they put a href around the image???? if so get the image
                  // it is sounding.
                  if ($check->tagName === 'a') {
                    foreach ($img->childNodes as $foo) {
                      if ($foo->tagName === 'img') {
                        $img = $foo;
                        break;
                      }
                    }
                  }

                  if (method_exists($img, 'hasAttribute')) {
                    $img->removeAttribute('vspace');
                    $img->removeAttribute('hspace');
                    $img->removeAttribute('border');

                    /*
                    if ($contentType === 'news-migrate-image') {
                      $img->removeAttribute('height');
                      $img->removeAttribute('width');
                    }
                    */
                  }

                  $clonedImageObject['image'] = $img;

                  // set to true so the next pass in the loop we know to get the caption
                  $imageCheck = TRUE;
                }

                // we have flagged that this is a caption by what we have done in the
                // first drop through the loop so assume this is the caption row.
                if ($imageCheck === TRUE) {

                  // create the caption html entity node
                  $theCaption = $dom->createElement('div');
                  $theCaption->appendChild($dom->createAttribute('class'));
                  $theCaption->setAttribute('class', 'field field--image-caption');
                  $theCaption->nodeValue = trim($trList->item(1)->nodeValue);
                  $clonedImageObject['caption'] = $theCaption;
                }

                $classesToAdd = 'media-element-container migrated';

                if (strlen($contentType) > 0) {
                  $classesToAdd = $classesToAdd . ' ' . $contentType;
                }

                if ($clonedImageObject['image'] != NULL && $clonedImageObject['caption'] != NULL) {

                  /*
                  $this->drushLog("IMMA HERE BIG BOY!!!!");
                  print_r($clonedImageObject['image']->NextSibling);
                  print_r($clonedImageObject['caption']);
                  */

                  $newDiv = $dom->createElement('div');
                  $newDiv->appendChild($dom->createAttribute('class'));
                  $newDiv->setAttribute('class', $classesToAdd);

                  $newDiv->appendChild($clonedImageObject['image']);
                  $newDiv->appendChild($clonedImageObject['caption']);

                  // new stuff to accomplish the media token stuff.
                  if (
                    method_exists($clonedImageObject['image'], 'hasAttribute')
                    &&
                    $clonedImageObject['image']->hasAttribute('fid')) {

                    // default our arguments...
                    $fid = $clonedImageObject['image']->getAttribute('fid');
                    $altText = '';
                    $caption = $clonedImageObject['caption']->nodeValue;
                    $view_mode = 'medium';
                    $height = '';
                    $width = '';

                    // check for alt
                    if ($clonedImageObject['image']->hasAttribute('alt')) {
                      $altText = $clonedImageObject['image']->getAttribute('alt');
                    }

                    // check for height
                    if ($clonedImageObject['image']->hasAttribute('height') && strlen( $clonedImageObject['image']->getAttribute('height') )) {
                      $height = $clonedImageObject['image']->getAttribute('height');
                    }

                    // check for width
                    if ($clonedImageObject['image']->hasAttribute('width') && strlen( $clonedImageObject['image']->getAttribute('width') )) {
                      $width = $clonedImageObject['image']->getAttribute('width');
                    }

                    // if image width is less than 220 make the media token small
                    if ($clonedImageObject['image']->hasAttribute('width') && $clonedImageObject['image']->getAttribute('width') < 220) {
                      $view_mode = 'small';
                    }

                    // if image width is larger than 480 make the media token large
                    if ($clonedImageObject['image']->hasAttribute('width') && $clonedImageObject['image']->getAttribute('width') > 480) {
                      $view_mode = 'large';
                    }

                    // if image width is larger than 960 make the media token EXTRA LARGE
                    if ($clonedImageObject['image']->hasAttribute('width') && $clonedImageObject['image']->getAttribute('width') > 960) {
                      $view_mode = 'extra_large';
                    }

                    // make the media token
                    $mediaToken = $this->getMediaToken($fid, $altText, $caption, $view_mode, $height, $width);

                    $this->drushLog('############ MEDIA TOKEN IN TABLE CONVERT ############ ');
                    $this->drushLog($mediaToken);
                    $this->drushLog('############ MEDIA TOKEN IN TABLE CONVERT ############ ');

                    $mediaTokenNode = $dom->createTextNode($mediaToken);
                  }
                  else {

                    // THIS IS A DEFENSIVE MOVE! If for some reason the node that
                    // is supposed to be the image doesn't have the FID attribute
                    // or for some unknown reason it's just plane borked... can't
                    // check for everything lets just use the original cloned
                    // IMG tag.
                    $mediaTokenNode = $clonedImageObject['image'];
                  }

                  $containerObject = array(
                    'newDiv' => $newDiv,
                    'mediaTokenNode' => $mediaTokenNode
                  );

                  $tableElement->appendChild($dom->createAttribute('jfa-delete'));
                  $tableElement->setAttribute('jfa-delete', '0');

                  $tableElement->appendChild($dom->createAttribute('replace-index'));
                  $tableElement->setAttribute('replace-index', $numberOfTablesToReplace);

                  $numberOfTablesToReplace++;

                  array_push($replacementArray, $containerObject);

                  break;
                }
              }
            }
          }
        }
      }
    }

    // loop over and delete
    $tableListToDelete = $dom->getElementsByTagName( "table" );

    for ($i = $tableListToDelete->length; $i >= 0; $i--) {

      $tableElementToKill = $tableListToDelete->item($i);

      if (method_exists($tableElementToKill, 'hasAttribute')) {

        if ($tableElementToKill->hasAttribute('jfa-delete')) {

          $indexOfImageCaptionToGet = $tableElementToKill->getAttribute('replace-index');

          $theParent = $tableElementToKill->parentNode;
          //$theParent->replaceChild($replacementArray[$indexOfImageCaptionToGet], $tableElementToKill);
          $theParent->replaceChild($replacementArray[$indexOfImageCaptionToGet]['mediaTokenNode'], $tableElementToKill);
        }
      }
    }

    return $dom;
  }

  /**
   * I return a Section Name based on a Access Taxonomy ID.
   * @param $id string = I am the access taxonomy ID to find the section by.
   * @return string
   */
  public function getGeneratedSectionFromPath( $id ) {
    $map = $this->getSectionLookUpMap()['nodes'];
    $refrencedEntityTitle = '';
    $result = '';

    foreach ($map as $node) {
      if ( $node['node']['TermID'] === $id ) {
        $refrencedEntityTitle = trim($node['node']['Default section']);
      }
    }

    return $this->getSectionEntityIDByTitle($refrencedEntityTitle);
  }

  /**
   * I return a lab/ou entity ID based on it's title.
   * @param $title
   * @return string
   */
  public function getSectionEntityIDByTitle($title) {

    $query = new EntityFieldQuery();
    $query->entityCondition('entity_type', 'node');
    $query->entityCondition('laboratory_organizational_unit', 'phat');
    $query->propertyCondition('title', trim($title), '=');
    $result = $query->execute();

    // i detest having to do it this way.
    if (is_array($result) && sizeof($result) === 1) {

      return array_keys($result['node'])[0];

    }
    else {

      return '';
    }
  }

  /**
   * I set the Sections Lookup Map json object
   */
  private function setSectionLookUpMap() {

    $string = file_get_contents("sectionsJSONLookup.json", TRUE);
    $this->sectionLookUpMap = json_decode( $string, TRUE );
  }

  /**
   * I return the Sections Lookup Map json object
   */
  private function getSectionLookUpMap() {
    return $this->sectionLookUpMap;
  }

  /**
   * I should be called by migration scripts Prepare() method.
   * @param $node I am the node of the migrate
   */
  public function globalPrepare($node) {
    // For whatever reason menu links for migrated node are getting
    // deleted when the migrates are run as updates (not rollback reimport)
    // see WWW-334
    if ( isset($node->menu) && !empty($node->menu) && is_array($node->menu) ) {
      $node->menu['enabled'] = 1;
    }
  }

  /**
   * I convert a (Month, 01 2016 hh:mm:ss) formatted date to the expected one for
   * adding time to.
   *
   * If given a borked string that can't be converted to a dateTime object I
   * return NOW in the proper fomat.
   *
   * @param $dateString
   * @return string
   */
  public function convertColdFusionJSONDateTimeToPHPDateTime( $dateString ) {

    $date = DateTime::createFromFormat('F, d Y H:i:s', $dateString);

    if ($date !== FALSE) {
      return $date->format('Y-m-d H:i:s');
    }
    else {
      $nowResult = new DateTime('now');
      return $nowResult->format('Y-m-d H:i:s');
    }
  }

  /**
   * I return a formatted date to use in the getIDList() URL formdate methods
   * called on the remote CommonSpot server. Pass me an offset, e.g.: the amount
   * of days to subtract from NOW and I'll return NOW - the offset.
   *
   * @param $offset - sting
   * @return bool|string
   */
  public function getFromDate( $offset = '1' ) {

    $nowDate = date('o-m-d');
    $offset = '-' . $offset . ' day';
    $fromDate = date('o-m-d', strtotime($offset, strtotime($nowDate)));

    return $fromDate;
  }

  /**
   * I return a redirect from a commonspot URL
   * @param string $path
   * @return mixed
   */
  public function getRedirectFromCommonSpotPath( $path = '' ) {

    $query = db_select('redirect');
    $query->addField('redirect', 'redirect');
    $query->condition('source', '%' . db_like(trim($path)) . '%', 'LIKE');
    $result = $query->execute()->fetchAssoc();

    return $result['redirect'];
  }


  /**
   * I take a date with hours and mins and make the hours and mins 00:00:00
   * @param $date
   * @return mixed
   */
  public function zeroOutDateTime($date) {

    $theTime = substr($date, -8); ;
    $newDate = str_replace($theTime, '00:00:00', $date);
    
    return $newDate;
  }

  /**
   * I drush_log stuff for debugging.
   * @param $message
   */
  public function drushLog($message) {
    drush_log($message, $type = 'warning', $error = NULL);
  }
}
