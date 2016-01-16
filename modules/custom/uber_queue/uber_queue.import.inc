<?php

function uber_queue_import(stdClass $node, stdClass $row) {
  $uber_queue_number = variable_get('uber_queue_number', 0);
  $migration_status = 'SOURCE';
  $return = 'error';

  if (isset($row->nid) && is_numeric($row->nid)) {

    //Если номер очереди обновления совпадает с текущей, значит товар уже обновляли.
    //Его стоит пропустить
    if (isset($row->queue_number) && $uber_queue_number == $row->queue_number) {
      $msg = t("Ignore double node update: !nid",
        array(
          '!nid' => $row->nid,
        ));
      _uber_queue_import_log($msg);
      return 'ignored';
    }

    // Make sure is_new is off
    $node->is_new = FALSE;
    if (isset($node->nid)) {
      if ($node->nid != $row->nid) {
        $msg = t("Incoming nid !nid and map destination nid !destid1 don't match",
          array(
            '!nid' => $node->nid,
            '!destid1' => $row->nid
          ));
        _uber_queue_import_log($msg);
        return 'error';
      }
    }
    else {
      $node->nid = $row->nid;
      if (_uber_queue_ignore_fields_status($node)) {
        $migration_status = 'DESTINATION';
      }
    }

    // Get the existing vid, tnid so updates don't generate notices
    $values = db_select('node', 'n')
      ->fields('n', array('vid', 'tnid'))
      ->condition('nid', $node->nid)
      ->execute()
      ->fetchAssoc();
    if (empty($values)) {
      $msg = t("Incoming node ID !nid no longer exists",
        array('!nid' => $node->nid));
      _uber_queue_import_log($msg);
      return 'error';
    }
    $node->vid = $values['vid'];
    if (empty($node->tnid)) {
      $node->tnid = $values['tnid'];
    }
  }


  if ($migration_status == 'DESTINATION') {
    if (!isset($node->nid)) {
      $msg = t('System-of-record is DESTINATION, but no destination nid provided');
      _uber_queue_import_log($msg);
      return 'error';
    }

    $old_node = node_load($node->nid);

    if (empty($old_node)) {
      $msg = t('System-of-record is DESTINATION, but node !nid does not exist',
        array('!nid' => $node->nid));
      _uber_queue_import_log($msg);
      return 'error';
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
    if (!isset($node->type)) {
      $node->type = $old_node->type;
    }
  }

  if (!isset($node->type)) {
    $node->type = 'product';
  }


  // Set some required properties.

  if ($migration_status == 'SOURCE') {
    if (empty($node->language)) {
      $node->language = LANGUAGE_NONE;
    }

    // Apply defaults, allow standard node prepare hooks to fire.
    // node_object_prepare() will blow these away, so save them here and
    // stuff them in later if need be.
    if (isset($node->created)) {
      $created = _uber_queue_timestamp($node->created);
    }
    else {
      // To keep node_object_prepare() from choking
      $node->created = REQUEST_TIME;
    }
    if (isset($node->changed)) {
      $changed = _uber_queue_timestamp($node->changed);
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
  //Загрузим весь массив строк с источника для данного HASH
  $rows = uber_queue_load_hash($row->HASH);

  _uber_queue_import_prepare($node, $row, $rows);

  if (!isset($node->revision)) {
    $node->revision = 0; // Saves disk space and writes. Can be overridden.
  }

  // Trying to update an existing node
  if ($migration_status == 'DESTINATION') {

    _uber_queue_ignore_fields($node, $old_node);

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

  $migration_status = 'SOURCE';

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
  _uber_queue_fieldAttachValidate('node', $node);

  node_save($node);


  if (isset($node->nid)) {
    if ($updating) {
      $return = 'updated';
    }
    else {
      $return = 'new';
    }

    //Запишем данные в таблицу соответствий
    db_merge('uber_queue_map')
      ->key(array('sourceid' => $row->HASH))
      ->fields(array(
        'nid' => $node->nid,
        'last_imported' => time(),
        'queue_id' => $uber_queue_number,
      ))
      ->execute();


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
    //$return = array($node->nid);
  }
  else {
    $return = 'error';
  }

  return $return;
}


/**
 * Список полей для редактирования
 */
function _uber_queue_ignore_fields(stdClass &$node, stdClass $old_node) {
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

/**
 * Метод, который определяет необходимость игнорирования обновления у полей.
 * Если есть поля, исправленные вручную, возвращает TRUE. Это в свою очередь приводит к вызову node_load ("node_load_old")
 */
function _uber_queue_ignore_fields_status(stdClass $node) {
  $status = FALSE;
  $query = new EntityFieldQuery();
  $query
    ->entityCondition('entity_type', 'node') //определим, какой тип сушности выводим
    ->propertyCondition('type', 'product') //ограничим словарь
    ->propertyCondition('nid', $node->nid) //ограничим словарь
    ->fieldCondition('field_ignore', 'value', NULL, 'IS NOT NULL'); //укажем значение поля
  $result = $query->execute();
  if (isset($result['node']) && is_array($result['node']) && count($result['node']) > 0) {
    $status = TRUE;
  }
  return $status;
}

/**
 * Convert an incoming string (which may be a UNIX timestamp, or an arbitrarily-formatted
 * date/time string) to a UNIX timestamp.
 *
 * @param string $value
 */
function _uber_queue_timestamp($value) {
  // Does it look like it's already a timestamp? Just return it
  if (is_numeric($value)) {
    return $value;
  }

  // Default empty values to now
  if (empty($value)) {
    return time();
  }

  $date = new DateTime($value);
  $time = $date->format('U');
  if ($time == FALSE) {
    // Handles form YYYY-MM-DD HH:MM:SS.garbage
    if (drupal_strlen($value) > 19) {
      $time = strtotime(drupal_substr($value, 0, 19));
    }
  }
  return $time;
}

/**
 * Perform field validation against the field data in an entity. Wraps
 * field_attach_validate to handle exceptions cleanly and provide maximum
 * information for identifying the cause of validation errors.
 *
 * @param $entity_type
 *   The type of $entity; e.g. 'node' or 'user'.
 * @param $entity
 *   The entity with fields to validate.
 */
function _uber_queue_fieldAttachValidate($entity_type, $entity) {
  field_attach_validate($entity_type, $entity);

  /*
    try {
      field_attach_validate($entity_type, $entity);
    }
    catch (FieldValidationException $e) {
      $migration = Migration::currentMigration();
      foreach ($e->errors as $field_name => $field_errors) {
        foreach ($field_errors as $langcode => $errors) {
          foreach ($errors as $delta => $error_list) {
            foreach ($error_list as $index => $error) {
              $message = $error['message'];
              $migration->saveMessage(t('Field validation error for !field_name: !message',
                array('!field_name' => $field_name, '!message' => $message)));
            }
          }
        }
      }
    }
  */

}

function _uber_queue_import_log($msg, $param = NULL) {
  $uber_queue_number = variable_get('uber_queue_number', 0);
  $params = array(
    'queue_id' => $uber_queue_number,
    'msg' => $msg,
  );

  if ($param !== NULL) {
    $params['param'] = $param;
  }
  try {
    uber_queue_log('uber_queue_log', $params);
  } catch (Exception $e) {
    // In case of exception log it and leave the item in the queue
    // to be processed again later.
    watchdog_exception('cron_import', $e);
  }
}


/**
 * Заполнение полями
 */
function _uber_queue_import_prepare(stdClass &$node, $row, $rows = array()) {
//Заголовок товара
  if (isset($row->NOMEN)) {
    $node->title = $row->NOMEN;
  }
  if (count($rows) <= 0) {
    $rows = array($row->ID => $row);
  }

  $node->status = 1;
  $node->uid = 1;

  //Цена
  if (isset($row->PRICE)) {
    $node->sell_price = $row->PRICE;
  }

  //Идентификатор
  if (isset($row->HASH)) {
    $node->model = $row->HASH;
  }

  $str_search = $row->NOMEN . ' ' . _uber_queue_implode_dop($rows);
  $str_search = str_replace(array('/', '\\'), '', $str_search);

  $voc = taxonomy_vocabulary_machine_name_load('catalog');
  $tids = _uber_queue_parsing_terms($voc->vid, 0, $str_search);
  _uber_queue_import_prepare_taxonomy_term($node, 'taxonomy_catalog', $tids);

  $voc_brand = taxonomy_vocabulary_machine_name_load('brand');
  $brand_tids = _uber_queue_parsing_terms($voc_brand->vid, 0, $str_search, TRUE);
  _uber_queue_import_prepare_taxonomy_term($node, 'field_brand', $brand_tids);

  $voc_marks = taxonomy_vocabulary_machine_name_load('marks');
  $marks_tids = _uber_queue_parsing_terms($voc_marks->vid, 0, $str_search, FALSE);
  _uber_queue_import_prepare_taxonomy_term($node, 'field_mark', $marks_tids);

  //Теперь список полей, которые могут зависеть от нескольких источников
  //Наличие в продаже
  _uber_queue_import_prepare_status_active($node, $rows);
  //Описание
  _uber_queue_import_prepare_body($node, $rows);
  //Картинка товара
  _uber_queue_import_prepare_image($node, $rows);

  return TRUE;
}

/**
 * Заполнение полями
 * $values - массив с tid терминов
 */
function _uber_queue_import_prepare_taxonomy_term(stdClass &$node, $field_name, $values) {
  $language = isset($node->language) ? $node->language : LANGUAGE_NONE;
  $node->{$field_name}[$language] = array();
  foreach ($values as $value) {
    $node->{$field_name}[$language][] = array('tid' => $value);
  }
  return TRUE;
}

/**
 * На основе rows заполняет поле field_status у ноды
 */
function _uber_queue_import_prepare_status_active(stdClass &$node, $rows) {
  $status_active = FALSE;

  foreach ($rows as $source) {
    if (isset($source->ACTIVE) && $source->ACTIVE) {
      $status_active = TRUE;
    }
  }

  $language = isset($node->language) ? $node->language : LANGUAGE_NONE;
  $node->field_status[$language] = array();
  $node->field_status[$language][0] = array(
    'value' => $status_active ? 1 : 0,
  );
  return TRUE;
}


/**
 * На основе rows заполняет поле field_status у ноды
 */
function _uber_queue_import_prepare_image($node, $rows) {
  $dirSource = 'public://import_images/p1/';
  $dirDest = 'public://content/p/1/';
  $images = array();


//Собираем данные по изображениям из всех возможных строк в массив $images
  foreach ($rows as $source) {
    if (isset($source->PICTURE) && !empty($source->PICTURE)) {
      $file_name = $source->PICTURE . '.jpg';
      $file_path = $dirSource . $file_name;
      if (file_exists($file_path)) {
        $images[] = $file_name;
      }
    }
  }

  $language = isset($node->language) ? $node->language : LANGUAGE_NONE;
  $node->uc_product_image[$language] = array();
  $file = FALSE;

  //если есть изображение, то записываем его в ноду
  if (count($images) > 0) {
    $file_name = $images[0];
    $token_md5 = _uber_tokens_file_md5($images[0]);
    $dirDestFile = $dirDest . $token_md5 . '/';
    $file_path_dest = $dirDestFile . $file_name;
    $file_path_source = $dirSource . $file_name;

    //ищем fid
    $fid = db_select('file_managed', 'f')
      ->fields('f', array('fid'))
      ->condition('f.uri', $file_path_dest)
      ->execute()
      ->fetchField();

    if ($fid) {
      //файл в БД
      //нужна проверка на актуальность файла
      //если не актуален, то просто переписываем оригинал на сервере
      $file = file_load($fid);
    }
    else {
      //файла нет в БД
      //нужна запись файла
      $file = _uber_queue_image_load($file_path_source, $file_path_dest);
    }

    if ($file) {
      $node->uc_product_image[$language][] = (array)$file;
    }

  }
  return TRUE;
}


/**
 * На основе rows заполняет поле body у ноды
 */
function _uber_queue_import_prepare_body(stdClass &$node, $rows) {
  $body_text = '';

  foreach ($rows as $source) {
    if (isset($source->DOP) && !empty($source->DOP) && drupal_strlen($source->DOP) > drupal_strlen($body_text)) {
      $body_text = $source->DOP;
    }
  }

  $language = isset($node->language) ? $node->language : LANGUAGE_NONE;
  $node->body[$language] = array();
  if ($body_text != '') {
    $node->body[$language][0] = array(
      'value' => $body_text,
      'format' => 'full_html',
    );
  }
  return TRUE;
}

/**
 * Склеивает поля DOP у одинаковых товаров для правильного парсинга
 */
function _uber_queue_implode_dop($rows) {
  $dop = array('');
  foreach ($rows as $row) {
    if (isset($row->DOP) && !empty($row->DOP)) {
      $dop[] = $row->DOP;
    }
  }

  return implode(' ', $dop);
}


function _uber_queue_parsing_terms($vid, $parent, $str, $lower = TRUE, $default_name = 'Разное') {

  $sinonims = _uber_queue_term_load_sinonim($vid, $parent);
  $tids = array();
  foreach ($sinonims as $tid => $array_value) {
    foreach ($array_value as $value) {
      $flag = _uber_queue_search_key($str, $value, $lower);

      if ($flag) {
        $tids[] = $tid;
        $tids = array_merge($tids, _uber_queue_parsing_terms($vid, $tid, $str, $lower));
      }
    }
  }


  if (count($tids) == 0) {
    if ($default_tid = _uber_queue_getTid($default_name, $vid, $parent)) {
      $tids[] = $default_tid;
    }
  }

  //Удалим дубли терминов
  if ($parent == 0) {
    $tids = array_unique($tids);
  }

  return $tids;
}


/*
 * search term for name
 */
function _uber_queue_getTid($name, $vid, $parent = 0) {
  $query = db_select('taxonomy_term_data', 'td');
  $query->innerJoin('taxonomy_term_hierarchy', 'th', 'th.tid = td.tid');
  $query->fields('td', array('tid'));
  $query->condition('td.name', $name);
  $query->condition('td.vid', $vid);
  $query->condition('th.parent', $parent);
  $query->range(0, 1);
  $tid = $query->execute()->fetchField();

  if ($tid && is_numeric($tid)) {
    return $tid;
  }
  else {
    if (count(taxonomy_get_children($parent, $vid)) > 0) {
      //нужно создать термин таксономии
      $term = array(
        'vid' => $vid,
        'name' => $name,
        'parent' => $parent,
      );
      $term = (object) $term;
      taxonomy_term_save($term);
      if ($term->tid) {
        return $term->tid;
      }
    }
  }

  return FALSE;
}


/**
 * Загружаем массив с синонимами терминов
 *
 */
function _uber_queue_term_load_sinonim($vid, $parent) {
  $terms = taxonomy_get_tree($vid, $parent, 1, TRUE);
  if ($cache = cache_get('uber_queue_sinonim_' . $vid . '_' . $parent . '_1')) {
    $items = $cache->data;
  }
  else {

    $items = array();
    foreach ($terms as $term) {
      $field_items = field_get_items('taxonomy_term', $term, 'field_key');
      if (isset($field_items[0]['value']) && !empty($field_items[0]['value'])) {
        $values = $field_items[0]['value'];
        $values = str_replace(',', ';', $values);
        $items[$term->tid] = explode(';', $values);
        foreach ($items[$term->tid] as $key => $val) {
          $txt = trim($val);
          if (empty($txt)) {
            unset($items[$term->tid][$key]);
          }
        }

      }
    }

    cache_set('uber_queue_sinonim_' . $vid . '_' . $parent . '_1', $items, 'cache', time() + 60 * 60);
  }

  return $items;
}

/**
 * Основная функция для парсинга товаров по словарям
 * $lower - если TRUE, то все строки приводятся к нижнему регистру
 */
function _uber_queue_search_key($str, $val, $lower = TRUE) {
  $val = trim($val);
  if ($lower) {
    $val = drupal_strtolower($val);
    $str = drupal_strtolower($str);
  }

  $flag = FALSE;

  try {
    if (!empty($val) && strpos($str, $val) !== FALSE) {
      if (preg_match('/([^\pL]|^)' . $val . '([^\pL]|$)/u', $str)) {
        $flag = TRUE;
      }
    }
  } catch (Exception $e) {
    // In case of exception log it and leave the item in the queue
    // to be processed again later.
    watchdog_exception('queue_import', $e);
  }

  return $flag;
}

/**
 * Скачиваем файл
 */
function _uber_queue_image_load($fileSource, $fileDest) {
  $file_path = file_create_url($fileSource);
  $file = FALSE;

// Получаем директорию и если ее еще нет, то создаем
  $real_url = file_destination($fileDest, FILE_EXISTS_REPLACE);
  $real_dir = drupal_dirname($real_url);
  if (!is_dir($real_dir)) {
    if (mkdir($real_dir, 0777)) {
      $msg = t("Folder '!name' create", array('!name' => $real_dir));
    }
    else {
      $msg = t("Error create folder: ") . $real_dir;
    }
    _uber_queue_import_log($msg);
  }

//Приготовления закончены, теперь приступим непосредственно к сохранению файла
//Укажем путь для сохранения файла вместе с именем файла.
  $destination = $fileDest;
  _uber_queue_import_log($file_path);
//Загружаем путь
  $data = drupal_http_request($file_path);
  $file = FALSE;
//Проверяем на успешную загрузку
  if ($data->code == 200) {
    //Сохраняем файл
    $file = file_save_data($data->data, $destination);
  }

  return $file;
}

function uber_queue_update_node($node) {
  $maps = array();
  $error = FALSE;
  $maps = db_select('uber_queue_map', 'n')
    ->fields('n')
    ->condition('nid', $node->nid)
    ->execute()
    ->fetchAllAssoc('nid');

  if (count($maps) <= 0) {
//  drupal_set_message(t('Node :!nid - no maps', array('!nid' => $node->nid)), 'error');
    return $error;
  }

  $map = array_shift($maps);
  $rows = uber_queue_load_hash($map->sourceid);

  if (count($rows) <= 0) {
//  drupal_set_message(t('Not source line'), 'error');
    return $error;
  }

  $row = array_shift($rows);
  $row->queue_number = 0;

  $node = new stdClass();
  $status_op = uber_queue_import($node, $row);
//  drupal_set_message($status_op);
  return $status_op;
}
