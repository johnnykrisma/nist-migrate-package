<?php
/**
 * @file
 * Base class to handle JSON User sources
 *
 */
class UserListJSON extends MigrateListJSON {
  /**
   * The default implementation assumes the IDs are top-level array elements,
   * but the array elements are the data items - we need to look inside them
   * for the IDs.
   * @param array $data
   * @return array
   */
  protected function getIDsFromJSON(array $data) {
    $ids = array();

    foreach ( $data as $item ) {
      $ids[] = $item['people_id'];
    }

    return $ids;
  }
}


class UserItemJSON extends MigrateItemJSON {
  protected $data = array();

  /**
   * Two problems with the default getItem() - it reads and parses the JSON on
   * each call, which we don't want to do for multiple items in the file; and,
   * we need to retrieve a given item from the file according to its 'id' element.
   * @param mixed $people_id
   * @return null
   */
  public function getItem( $people_id ) {

    // We cache the parsed JSON at $this->data.
    if (empty($this->data)) {

      $data = $this->loadJSONUrl( $this->itemUrl );

      if ($data) {
        // Let's index the array by the ID for easy retrieval.
        foreach ($data as $item) {

          $this->data[$item->people_id] = $item;

        }
      }
      else {
        print_r($people_id);
      }
    }
    // Return the requested item
    if (isset($this->data[$people_id])) {

      return $this->data[$people_id];
    }
    else {
      return NULL;
    }
  }
}



/**
 * Base class to handle JSON PAGE sources
 *
 */
class PageListJSON extends MigrateListJSON {
  /**
   * The default implementation assumes the IDs are top-level array elements,
   * but the array elements are the data items - we need to look inside them
   * for the IDs.
   */
  protected function getIDsFromJSON(array $data) {
    $ids = array();

    foreach ($data as $item) {
      if (!isset($item['pageid'])) :
        //dpm($item);
      else :
        $ids[] = $item['pageid'];
      endif;
    }
    return $ids;
  }
}

class PageItemJSON extends MigrateItemJSON {
  protected $data = array();
  /**
   * Two problems with the default getItem() - it reads and parses the JSON on
   * each call, which we don't want to do for multiple items in the file; and,
   * we need to retrieve a given item from the file according to its 'id' element.
   */
  public function getItem($pageid) {
    // We cache the parsed JSON at $this->data.
    if (empty($this->data)) {
      $data = $this->loadJSONUrl($this->itemUrl);
      if ($data) {
        // Let's index the array by the ID for easy retrieval.
        foreach ($data as $item) {
          $this->data[$item->pageid] = $item;
        }
      }
      else {
        // Error-handling here....
      }
    }
    // Return the requested item
    if (isset($this->data[$pageid])) {
      return $this->data[$pageid];
    }
    else {
      return NULL;
    }
  }
}

/**
 * Base class to handle JSON TERMS sources
 *
 */
class TermListJSON extends MigrateListJSON {
  /**
   * The default implementation assumes the IDs are top-level array elements,
   * but the array elements are the data items - we need to look inside them
   * for the IDs.
   */
  protected function getIDsFromJSON(array $data) {
    $ids = array();
    foreach ($data as $item) {
      $ids[] = $item['termid'];
    }
    return $ids;
  }
}

class TermItemJSON extends MigrateItemJSON {
  protected $data = array();
  /**
   * Two problems with the default getItem() - it reads and parses the JSON on
   * each call, which we don't want to do for multiple items in the file; and,
   * we need to retrieve a given item from the file according to its 'id' element.
   */
  public function getItem($termid) {
    // We cache the parsed JSON at $this->data.
    if (empty($this->data)) {
      $data = $this->loadJSONUrl($this->itemUrl);
      if ($data) {
        // Let's index the array by the ID for easy retrieval.
        foreach ($data as $item) {
          $this->data[$item->termid] = $item;
        }
      }
      else {
        // Error-handling here....
      }
    }
    // Return the requested item
    if (isset($this->data[$termid])) {
      return $this->data[$termid];
    }
    else {
      return NULL;
    }
  }
}
