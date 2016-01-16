<?php
function uber_queue_report_form($form, &$form_state) {
  $form = array();
  // $debug = variable_get('uber_queue_debug', 0);
  // $activ = variable_get('uber_queue_number', 0);

  // Загружаем все сохраненные очереди
  $options = uber_queue_report_load_all_queues();
  $form['queue_id'] = array(
    '#type'    => 'select',
    '#title'   => t('Queue id'),
    '#options' => $options,
    '#ajax'    => array(
      'callback' => 'form_ajax_uber_queue_report_form',
      'wrapper'  => 'form-ajax-queue',
      'method'   => 'replace',
      'effect'   => 'fade',
    ),
  );
  // Если сработал AJAX - в $form_state будут переданы все значения формы.
  if (isset($form_state['values']['queue_id'])) {
    $queue_id = $form_state['values']['queue_id'];
  }
  else {
    $queue_id = array_shift($options);
  }
// Устанавливаем значение по умолчанию для выпадающего списка.
  $form['queue_id']['#default_value'] = $queue_id;

  $log_options = array(
    0 => t('Hidden log'),
    1 => t('Show log'),
  );
  $form['log'] = array(
    '#type'    => 'select',
    '#title'   => t('Log'),
    '#options' => $log_options,
    '#ajax'    => array(
      'callback' => 'form_ajax_uber_queue_report_form',
      'wrapper'  => 'form-ajax-queue',
      'method'   => 'replace',
      'effect'   => 'fade',
    ),
  );
  // Если сработал AJAX - в $form_state будут переданы все значения формы.
  if (isset($form_state['values']['log'])) {
    $log = $form_state['values']['log'];
  }
  else {
    $log = 0;
  }

// Устанавливаем значение по умолчанию для выпадающего списка.
  $form['log']['#default_value'] = $log;

  $q = uber_queue_report_load($queue_id, $log);
  if (!$q) {
    drupal_set_message(t('No queue: !activ', array('!activ' => $queue_id)), 'error');
    return $form;
  }

  $form['cont'] = array(
    '#type'   => 'container',
    '#title'  => '',
    '#prefix' => '<div id = "form-ajax-queue">',
    '#suffix' => '</div>',
  );

  $form['cont']['progressbar'] = array(
    '#markup' => _uber_queue_render_report_progressbar($q),
  );

  $form['cont']['table_stats'] = array(
    '#markup' => _uber_queue_render_report_table($q),
  );

  if (count($q->log) > 0) {
    $form['cont']['table_log_cont'] = array(
      '#type'        => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed'   => TRUE,
      '#title'       => t('Log'),
    );

    $form['cont']['table_log_cont']['table_log'] = array(
      '#markup' => _uber_queue_render_report_log_table($q),
    );
  }

  return $form;
}

/**
 * AJAX callback for loading statistics
 */
function form_ajax_uber_queue_report_form($form, $form_state) {
  // Возвращаем элемент формы, который должен быть перезагружен.
  // В данном случае надо перезагрузить выпадающий список с материалами.
  return $form['cont'];
}


/**
 * Return status queue
 */
function _uber_queue_report_status($q) {
  $activ = variable_get('uber_queue_number', 0);
  if ($activ == $q->id) {
    $queue = DrupalQueue::get('uber_main_queue');
    $count = $queue->numberOfItems();
    if ($count > 0) {
      return 'active';
    }
    else {
      return 'close';
    }
  }
  return 'old';
}

/**
 * Return speed queue
 * @param $q
 * @return float
 */
function _uber_queue_report_speed($q) {
  //Время с начала обновления
  if ($q->status == 'active') {
    $q->last = time();
  }
  $time = ($q->last - $q->start) / 60;
  $count = _uber_queue_report_count($q);
  $speed = $count / $time;

  return round($speed, 1);
}

/**
 * Return real speed queue
 * @param $q
 * @return float
 */
function _uber_queue_report_speed_real($q) {
  //Время с начала обновления
  $speed_all = 0;
  $count = 0;
  foreach ($q->items as $item) {
    $time_inc = ($item->end - $item->start);

    if ($time_inc > 0) {
      $speed_all = $speed_all + ($item->count / $time_inc);
      $count++;
    }

  }

  $speed_sr = $count > 0 ? $speed_all / $count : 0;
  return round($speed_sr, 2);
}

/**
 * Return count all items for queue
 * @param $q
 * @return int
 */
function _uber_queue_report_count($q) {
  $count = 0;
  foreach ($q->items as $item) {
    $count += $item->count;
  }
  return $count;
}

/**
 * Возвращает массив со статусами по каждой ноде
 * @param $q
 * @return array
 */
function _uber_queue_report_count_status($q) {
  $status_ops = array(
    'ignored' => 0,
    'updated' => 0,
    'new'     => 0,
    'error'   => 0,
    'all'     => 0,
  );

  foreach ($q->items as $item) {
    if (isset($item->ignored)) {
      $status_ops['ignored'] += $item->ignored;
      $status_ops['all'] += $item->ignored;
    }
    if (isset($item->updated)) {
      $status_ops['updated'] += $item->updated;
      $status_ops['all'] += $item->updated;
    }
    if (isset($item->new)) {
      $status_ops['new'] += $item->new;
      $status_ops['all'] += $item->new;
    }
    if (isset($item->error)) {
      $status_ops['error'] += $item->error;
      $status_ops['all'] += $item->error;
    }
  }
  return $status_ops;
}


/**
 * ВОзвращает список всех очередей, записанных в БД
 * @return array
 */
function uber_queue_report_load_all_queues() {
  $ids = array();
  $result = db_select('uber_queue_history', 'n')
    ->fields('n', array('id'))
    ->orderBy('n.id', 'DESC')
    ->execute();

  foreach ($result as $item) {
    $ids[$item->id] = $item->id;
  }

  return $ids;
}

/**
 * Печатает основную таблицу со статистикой
 * @param $q
 * @return string
 */
function _uber_queue_render_report_table($q) {
  $output = '';
  $rows = array();
  $header = array(
    t('ID'),
    t('Status'),
    t('Created'),
    t('Last update'),
    t('Count nodes'),
    t('Count nodes (i/u/n/e)'),
    t('Count lines complete'),
    t('Speed, node/min'),
    t('Speed real, node/s'),
  );

  $rows[] = array(
    $q->id,
    $q->status,
    format_date($q->start, 'short'),
    format_date($q->last, 'short'),
    $q->report_count . '/' . $q->count . '(' . $q->report_percent . '%)',
    implode('/', $q->status_items),
    count($q->items),
    $q->speed,
    $q->speed_real,
  );

  $output .= theme('table', array(
    'header' => $header,
    'rows'   => $rows
  ));

  return $output;
}

/**
 * Печатает таблицу с логами по очередям
 * @param $q
 * @return mixed
 */
function _uber_queue_render_report_log_table($q) {
  $header_log = array(
    t('id'),
    t('Message'),
    t('Param'),
  );
  $rows_log = array();
  foreach ($q->log as $log_item) {
    $rows_log[] = array(
      $log_item->id,
      $log_item->msg,
      $log_item->param,
    );
  }
  $output = theme('table', array(
    'header' => $header_log,
    'rows'   => $rows_log
  ));
  return $output;
}

/**
 * Рисует прогрессбар для активной очереди
 * @param $q
 * @return string
 */
function _uber_queue_render_report_progressbar($q) {
  $output = '';
  if ($q->status == 'active') {
    $output .= theme_progress_bar(array(
        'percent' => $q->report_percent,
        'message' => t('Complete !count to !count_all', array(
          '!count'     => $q->report_count,
          '!count_all' => $q->count
        )),
      )) . '<br />';
  }
  return $output;
}

/**
 * Загружает объект очереди по номеру.
 * @param $num - номер очереди
 * @param int $log - загружать логи или нет
 * @return  - объект очереди или FALSE
 */
function uber_queue_report_load($num, $log = 0) {

  $q = db_select('uber_queue_history', 'q')
    ->fields('q')
    ->condition('q.id', $num)
    ->execute()
    ->fetchObject();

  if (!$q) {
    //drupal_set_message(t('No queue: !activ', array('!activ' => $activ)),'error');
    return FALSE;
  }

  $q->status = _uber_queue_report_status($q);
  if (empty($q->last)) {
    $q->last = $q->start;
  }

  $items = db_select('uber_queue_history_items', 'n')
    ->fields('n')
    ->condition('n.queue_id', $q->id)
    ->execute()
    ->fetchAllAssoc('id');
  $q->items = $items;
  $q->status_items = _uber_queue_report_count_status($q);

  $q->report_count = _uber_queue_report_count($q);
  $q->report_percent = round($q->report_count / $q->count * 100, 2);
  $q->speed = _uber_queue_report_speed($q);
  $q->speed_real = _uber_queue_report_speed_real($q);

  if ($log) {
    $q->log = db_select('uber_queue_log', 'q')
      ->fields('q')
      ->condition('q.queue_id', $num)
      ->orderBy('q.id', 'DESC')
      ->execute()
      ->fetchAllAssoc('id');
  }
  else {
    $q->log = array();
  }
  return $q;
}
