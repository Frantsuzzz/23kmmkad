<?php

/**
 * Destination class implementing migration into nodes.
 */
class MigrateDestinationProductNode extends MigrateDestinationNode {

  public function fields($migration = NULL) {
    $fields = MigrateDestinationNode::fields($migration);
    $node_type = node_type_load($this->bundle);

    if ($node_type->type == 'product') {
      //Добавим базовые поля от ubercart
      $fields['model'] = t('Product SKU/model.');
      $fields['list_price'] = t('The listed MSRP.');
      $fields['cost'] = t("Your store's cost.");
      $fields['sell_price'] = t('Customer purchase price.');
    }

    return $fields;
  }

  public function ignore_fields(stdClass &$node, stdClass $old_node) {
    //найдем список полей для игнорирования
    $field_ignore = field_get_items('node', $old_node, 'field_ignore');
    if (is_array($field_ignore)) {
      foreach ($field_ignore as $key => $value) {
        if (isset($value['value']) && isset($node->$value['value'])) {
          unset($node->$value['value']);
        }
      }
    }
    return $node;
  }

  /*
  * Метод, который определяет необходимость игнорирования обновления у полей.
  * Если есть поля, исправленные вручную, возвращает TRUE. Это в свою очередь приводит к вызову node_load ("node_load_old")
  */
  public function ignore_fields_status(stdClass $node) {
    $status = FALSE;
    $query = new EntityFieldQuery();
    $query
      ->entityCondition('entity_type', 'node') //определим, какой тип сушности выводим
      ->propertyCondition('type', 'product') //ограничим словарь
      ->propertyCondition('nid', $node->nid) //ограничим словарь
      ->fieldCondition('field_ignore', 'value', NULL, 'IS NOT NULL'); //укажем значение поля
    $result = $query->execute();
    if (isset($result['node']) && is_array($result['node']) && count($result['node']) > 0) {
      migrate_instrument_start('ignore_fields_status_true');
      $status = TRUE;
      migrate_instrument_stop('ignore_fields_status_true');
    }
    return $status;
  }


  /**
   * Import a single node.
   *
   * @param $node
   *  Node object to build. Prefilled with any fields mapped in the Migration.
   * @param $row
   *  Raw source data object - passed through to prepare/complete handlers.
   * @return array
   *  Array of key fields (nid only in this case) of the node that was saved if
   *  successful. FALSE on failure.
   */
  public function import(stdClass $node, stdClass $row) {
    // Updating previously-migrated content?
    $migration = Migration::currentMigration();
    if (isset($row->migrate_map_destid1) && !$this->bypassDestIdCheck) {
      // Make sure is_new is off
      $node->is_new = FALSE;
      if (isset($node->nid)) {
        if ($node->nid != $row->migrate_map_destid1) {
          throw new MigrateException(t("Incoming nid !nid and map destination nid !destid1 don't match",
            array('!nid'     => $node->nid,
                  '!destid1' => $row->migrate_map_destid1
            )));
        }
      }
      else {
        $node->nid = $row->migrate_map_destid1;
        /*
         * Start hack
         */
        if ($this->ignore_fields_status($node)) {
          $migration->setSystemOfRecord(Migration::DESTINATION);
        }
        /*
        * End hack
        */
      }
      // Get the existing vid, tnid so updates don't generate notices
      $values = db_select('node', 'n')
        ->fields('n', array('vid', 'tnid'))
        ->condition('nid', $node->nid)
        ->execute()
        ->fetchAssoc();
      if (empty($values)) {
        throw new MigrateException(t("Incoming node ID !nid no longer exists",
          array('!nid' => $node->nid)));
      }
      $node->vid = $values['vid'];
      if (empty($node->tnid)) {
        $node->tnid = $values['tnid'];
      }
    }


    if ($migration->getSystemOfRecord() == Migration::DESTINATION) {
      if (!isset($node->nid)) {
        throw new MigrateException(t('System-of-record is DESTINATION, but no destination nid provided'));
      }
      migrate_instrument_start('node_load_old');
      $old_node = node_load($node->nid);
      migrate_instrument_stop('node_load_old');

      if (empty($old_node)) {
        throw new MigrateException(t('System-of-record is DESTINATION, but node !nid does not exist',
          array('!nid' => $node->nid)));
      }
      if (!isset($node->created)) {
        $node->created = $old_node->created;
      }
      if (!isset($node->vid)) {
        $node->vid = $old_node->vid;
      }
      if (!isset($node->status)) {
        $node->status = $old_node->status;
      }
      if (!isset($node->uid)) {
        $node->uid = $old_node->uid;
      }
    }

    if (!isset($node->type)) {
      // Default the type to our designated destination bundle (by doing this
      // conditionally, we permit some flexibility in terms of implementing
      // migrations which can affect more than one type).
      $node->type = $this->bundle;
    }

    // Set some required properties.

    if ($migration->getSystemOfRecord() == Migration::SOURCE) {
      if (empty($node->language)) {
        $node->language = $this->language;
      }

      // Apply defaults, allow standard node prepare hooks to fire.
      // node_object_prepare() will blow these away, so save them here and
      // stuff them in later if need be.
      if (isset($node->created)) {
        $created = MigrationBase::timestamp($node->created);
      }
      else {
        // To keep node_object_prepare() from choking
        $node->created = REQUEST_TIME;
      }
      if (isset($node->changed)) {
        $changed = MigrationBase::timestamp($node->changed);
      }
      if (isset($node->uid)) {
        $uid = $node->uid;
      }
      if (isset($node->revision)) {
        $revision = $node->revision;
      }

      node_object_prepare($node);

      if (isset($created)) {
        $node->created = $created;
      }
      // No point to resetting $node->changed here, node_save() will overwrite it
      if (isset($uid)) {
        $node->uid = $uid;
      }
      if (isset($revision)) {
        $node->revision = $revision;
      }
    }

    // Invoke migration prepare handlers
    $this->prepare($node, $row);

    if (!isset($node->revision)) {
      $node->revision = 0; // Saves disk space and writes. Can be overridden.
    }

    // Trying to update an existing node
    if ($migration->getSystemOfRecord() == Migration::DESTINATION) {

      /*
       * Start hack
       */
      $this->ignore_fields($node, $old_node);
      /*
       * End hack
       */


      // Incoming data overrides existing data, so only copy non-existent fields
      foreach ($old_node as $field => $value) {
        // An explicit NULL in the source data means to wipe to old value (i.e.,
        // don't copy it over from $old_node)
        if (property_exists($node, $field) && $node->$field === NULL) {
          // Ignore this field
        }
        elseif (!isset($node->$field)) {
          $node->$field = $old_node->$field;
        }
      }
    }

    /*
     * Start hack
     */
    $migration->setSystemOfRecord(Migration::SOURCE);
    /*
     * End hack
     */


    if (isset($node->nid) && !(isset($node->is_new) && $node->is_new)) {
      $updating = TRUE;
    }
    else {
      $updating = FALSE;
    }

    // Make sure that if is_new is not TRUE, it is not present.
    if (isset($node->is_new) && empty($node->is_new)) {
      unset($node->is_new);
    }



    // Validate field data prior to saving.
    MigrateDestinationEntity::fieldAttachValidate('node', $node);

    migrate_instrument_start('node_save');
    node_save($node);
    migrate_instrument_stop('node_save');

    if (isset($node->nid)) {
      if ($updating) {
        $this->numUpdated++;
      }
      else {
        $this->numCreated++;
      }

      // Unfortunately, http://drupal.org/node/722688 was not accepted, so fix
      // the changed timestamp
      if (isset($changed)) {
        db_update('node')
          ->fields(array('changed' => $changed))
          ->condition('nid', $node->nid)
          ->execute();
        $node->changed = $changed;
      }

      // Potentially fix uid and timestamp in node_revisions.
      $query = db_update('node_revision')
        ->condition('vid', $node->vid);
      if (isset($changed)) {
        $fields['timestamp'] = $changed;
      }
      $revision_uid = isset($node->revision_uid) ? $node->revision_uid : $node->uid;
      if ($revision_uid != $GLOBALS['user']->uid) {
        $fields['uid'] = $revision_uid;
      }
      if (!empty($fields)) {
        // We actually have something to update.
        $query->fields($fields);
        $query->execute();
        if (isset($changed)) {
          $node->timestamp = $changed;
        }
      }
      $return = array($node->nid);
    }
    else {
      $return = FALSE;
    }

    $this->complete($node, $row);
    return $return;
  }
}


class ProductsMigration extends Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $options = array();
    $options['track_changes'] = 1;
    $query = db_select('go2', 'g')->fields('g')->condition('ID', '', '<>');
    //$this->source = new MigrateSourceSQL($query, array('ID','NOMEN'), NULL, $options);
    $this->source = new MigrateSourceSQL($query, array(), NULL, $options);

    // Destination
    $this->destination = new MigrateDestinationProductNode('product');

    // Key schema
    $source_key_schema = array(
      'ID' => array(
        'type'        => 'varchar',
        'length'      => 255,
        'not null'    => TRUE,
        'description' => 'Source ID',
      )
    );

    $this->map = new MigrateSQLMap($this->machineName, $source_key_schema, MigrateDestinationProductNode::getKeySchema());

    // Mapping
    $this->addFieldMapping('title', 'NOMEN');
    $this->addFieldMapping('body', 'DOP')->defaultValue(NULL);
    $this->addFieldMapping('body:format')->defaultValue('full_html');
    $this->addFieldMapping('status')->defaultValue(TRUE);
    //   $this->addFieldMapping('promote')->defaultValue(FALSE);
    //   $this->addFieldMapping('sticky')->defaultValue(FALSE);
    $this->addFieldMapping('uid')->defaultValue(1);

    $this->addFieldMapping('taxonomy_catalog', 'CATALOG');
    $this->addFieldMapping('taxonomy_catalog:source_type')->defaultValue('tid');
    $this->addFieldMapping('taxonomy_catalog:create_term')->defaultValue(FALSE);

    $this->addFieldMapping('field_brand', 'PROIZ');
    $this->addFieldMapping('field_brand:source_type')->defaultValue('tid');
    $this->addFieldMapping('field_brand:create_term')->defaultValue(FALSE);

    $this->addFieldMapping('field_supplier', 'SUPPLIER');
    $this->addFieldMapping('field_supplier:create_term')->defaultValue(TRUE);
    $this->addFieldMapping('sell_price', 'PRICE')->defaultValue(0);
    $this->addFieldMapping('model', 'ID')->defaultValue(NULL);
    $this->addFieldMapping('field_art', 'ARTICUL')->defaultValue(NULL);

    $this->addFieldMapping('field_mark', 'BRAND');
    $this->addFieldMapping('field_mark:source_type')->defaultValue('tid');
    $this->addFieldMapping('field_mark:create_term')->defaultValue(FALSE);

  }

  public function prepareRow($row) {

    $str_search = drupal_strtolower($row->NOMEN . ' ' . $row->DOP);
    $voc = taxonomy_vocabulary_machine_name_load('catalog');
    $tids = _uber_migrate_parsing_terms($voc->vid, 0, $str_search);
    $row->CATALOG = $tids;

    $voc_brand = taxonomy_vocabulary_machine_name_load('brand');
    $brand_tids = _uber_migrate_parsing_terms($voc_brand->vid, 0, $str_search);
    $row->PROIZ = $brand_tids;

	$voc_marks = taxonomy_vocabulary_machine_name_load('marks');
    $marks_tids = _uber_migrate_parsing_terms($voc_marks->vid, 0, $str_search);
    $row->BRAND = $marks_tids;

//  dpm($row);
  }
}


function _uber_migrate_parsing_terms($vid, $parent, $str) {
  migrate_instrument_start('parsing_terms');
  $sinonims = _uber_migrate_term_load_sinonim($vid, $parent);
  $tids = array();
  foreach ($sinonims as $tid => $array_value) {
    foreach ($array_value as $value) {
      $flag = _uber_migrate_search_key($str, $value);
      if ($flag) {
        $tids[] = $tid;
        $tids = array_merge($tids, _uber_migrate_parsing_terms($vid, $tid, $str));
      }
    }
  }
  migrate_instrument_stop('parsing_terms');
  return $tids;
}


function _uber_migrate_search_key($str, $val) {
  $val = trim($val);
  $values = array(
    $val,
  );

  $flag = FALSE;
  for ($i = 0; $i < count($values) && !$flag; $i++) {
    if (strpos($str, $values[$i]) !== FALSE) {
      $flag = TRUE;
    }
  }
  return $flag;
}


function _migrate_taxonomy_term_tree($namestxt, $vid) {
  $names = explode(',', $namestxt);
  $namesmd5 = md5($vid . '_' . $namestxt);
  $tids = array();


  if ($cache = cache_get('migrate_term_tree_' . $namesmd5)) {
    $tids = explode(',', $cache->data);
  }
  else {

    foreach ($names as $depth => $name) {
      $name = trim(check_plain($name));

      $parent = ($depth > 0) ? $tids[$depth - 1] : 0;

      $query = db_select('taxonomy_term_data', 't');
      $query->innerJoin('taxonomy_term_hierarchy', 'th', 't.tid = th.tid');
      $query->fields('t', array('tid'));
      $query->condition('t.vid', $vid);
      $query->condition('t.name', $name);
      $query->condition('th.parent', $parent);
      $tid = $query->execute()->fetchField();

      if ($tid) {
        //термин уже в БД
        $tids[$depth] = $tid;
      }
      else {
        //нужно создавать термин
        $term = array(
          'vid'    => $vid,
          'name'   => $name,
          'parent' => $parent,
        );
        $term = (object) $term;
        taxonomy_term_save($term);
        if ($term->tid) {
          $tids[$depth] = $term->tid;
        }
        else {
          $tids[$depth] = 0;
          drupal_set_message(t('Query not successful.'), 'error');
        }
      }
    }

    foreach ($tids as $key => $tid) {
      if ($tid == 0) {
        unset($tids[$key]);
      }
    }

    cache_set('migrate_term_tree_' . $namesmd5, implode(',', $tids), 'cache', time() + 3600);
  }
  return $tids;
}
