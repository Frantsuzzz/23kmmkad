<?php
$disable_hooks = array(
  'comment_insert' => array(
    'commons_activity_streams',
    'commons_follow_node',
    'commons_notify',
    'commons_radioactivity',
    'search',
    'expire',
  ),
  'comment_publish' => array(
    'search',
    'xmlsitemap_node',
    'expire',
  ),
  'comment_update' => array(
    'commons_activity_streams',
    'commons_follow_node',
    'commons_notify',
    'commons_radioactivity',
    'search',
  ),
  'entity_insert' => array(
    'metatag',
    'og',
    'registration',
  ),
  'entity_presave' => array(
    'pathauto',
  ),
  'entity_update' => array(
    'metatag',
    'og',
    'registration',
  ),
  'flag' => array(
    'commons_radioactivity',
  ),
  'node_insert' => array(
    'commons_activity_streams',
    'commons_follow_group',
    'commons_follow_node',
    'commons_groups',
    'commons_notify',
    'commons_radioactivity_groups',
    'kwresearch',
    'pathauto',
    'xmlsitemap_node',
    'expire',
  ),
  'node_update' => array(
    'commons_activity_streams',
    'commons_follow_group',
    'commons_follow_node',
    'commons_groups',
    'commons_radioactivity_groups',
    'kwresearch',
    'pathauto',
    'xmlsitemap_node',
    'expire',
  ),
  'node_prepare' => array(
    'og_menu',
  ),
  'taxonomy_term_insert' => array(
    'metatag',
    'pathauto',
    'shs',
    'taxonomy_menu',
  ),
  'taxonomy_term_update' => array(
    'pathauto',
    'shs',
    'taxonomy_menu',
  ),
  'user_insert' => array(
    'pathauto',
  ),
  'user_update' => array(
    'pathauto',
  ),
  'votingapi_insert' => array(
    'expire',
  )
);


/**
 * Implement hook_migrate_api()
 */
function uber_migrate_migrate_api() {
  return array(
    'api' => 2,
    'groups' => array(
      'uber' => array(
        'title' => 'Uber',
      ),
    ),
    'migrations' => array(
      'ProductGO2' => array(
        'class_name' => 'ProductsMigration',
        'group_name' => 'uber',
       // 'disable_hooks' => $disable_hooks,
      ),
    ),
  );
}
