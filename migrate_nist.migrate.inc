<?php

/**
 * @file
 * Common code for all NIST migrations.
 */

/**
 * Implementation of hook_migrate_api().
 * @return array
 */
function migrate_nist_migrate_api() {
  $api = array(
    'api' => 2,
    
    'groups' => array(
      'nist' => array(
        'title' => t('NIST Imports'),
        'default_language' => 'en',
      ),
    ),
    'migrations' => array(
      'NistAward' => array(
        'class_name' => 'NistAwardMigration',
        'group_name' => 'nist',
      ),
      'NistBlankPages' => array(
        'class_name' => 'NistBlankPagesMigration',
        'group_name' => 'nist',
      ),
      'NistCnsttool' => array(
        'class_name' => 'NistCnsttoolMigration',
        'group_name' => 'nist',
      ),
      'NistCommonspotPages' => array(
        'class_name' => 'NistCommonspotPagesMigration',
        'group_name' => 'nist',
      ),
      'NistEvent' => array(
        'class_name' => 'NistEventMigration',
        'group_name' => 'nist',
      ),
      'NistFile' => array(
        'class_name' => 'NistFileMigration',
        'group_name' => 'nist',
      ),
      'NistInstrument' => array(
        'class_name' => 'NistInstrumentMigration',
        'group_name' => 'nist',
      ),
      'NistImage' => array(
        'class_name' => 'NistImageMigration',
        'group_name' => 'nist',
      ),
      'NistKalturaVideo' => array(
        'class_name' => 'NistKalturaVideoMigration',
        'group_name' => 'nist',
      ),
      'NistMmlPages' => array(
        'class_name' => 'NistMmlPagesMigration',
        'group_name' => 'nist',
      ),
      'NistNews' => array(
        'class_name' => 'NistNewsMigration',
        'group_name' => 'nist',
      ),
      'NistNikeAuthors' => array(
        'class_name' => 'NistNikeAuthorsMigration',
        'group_name' => 'nist',
      ),
      'NistOrgStructure' => array(
        'class_name' => 'NistOrgStructureMigration',
        'group_name' => 'nist',
      ),
      'NistPhotoGallery' => array(
        'class_name' => 'NistPhotoGalleryMigration',
        'group_name' => 'nist',
      ),
      'NistProject' => array(
        'class_name' => 'NistProjectMigration',
        'group_name' => 'nist',
      ),
      'NistPublication' => array(
        'class_name' => 'NistPublicationMigration',
        'group_name' => 'nist',
      ),
      'NistSoftware' => array(
        'class_name' => 'NistSoftwareMigration',
        'group_name' => 'nist',
      ),
      'NistTerm' => array(
        'class_name' => 'NistTermMigration',
        'group_name' => 'nist',
      ),
      'NistUser' => array(
        'class_name' => 'NistUserMigration',
        'group_name' => 'nist',
      ),
      'NistUserImage' => array(
        'class_name' => 'NistUserImageMigration',
        'group_name' => 'nist',
      ),
      'NistUserMain' => array(
        'class_name' => 'NistUserMainMigration',
        'group_name' => 'nist',
      ),
    ),
  );
  return $api;
}
