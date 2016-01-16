<?php

class frz_tweaks_helper {

  /**
   * Функция склонения слов.
   * Функция предназначена для вывода численных результатов с учетом
   * склонения слов, например: "1 ответ", "2 ответа", "13 ответов" и т.д.
   * $digit — целое число
   * можно вместе с форматированием, например "<b>6</b>"
   * $expr — массив, например: array("ответ", "ответа", "ответов").
   * можно указывать только первые 2 элемента, например для склонения английских слов
   * (в таком случае первый элемент - единственное число, второй - множественное)
   * $expr может быть задан также в виде строки: "ответ ответа ответов", причем слова разделены символом "пробел"
   * $onlyword - если true, то выводит только слово, без числа;необязательный параметр
   */
  public static function declension($digit, $expr, $onlyword = FALSE) {
    if (!is_array($expr)) {
      $expr = array_filter(explode(' ', $expr));
    }
    if (empty($expr[2])) {
      $expr[2] = $expr[1];
    }
    $i = preg_replace('/[^0-9]+/s', '', $digit) % 100;
    if ($onlyword) {
      $digit = '';
    }
    if ($i >= 5 && $i <= 20) {
      $res = $digit . ' ' . $expr[2];
    }
    else {
      $i %= 10;
      if ($i == 1) {
        $res = $digit . ' ' . $expr[0];
      }
      elseif ($i >= 2 && $i <= 4) {
        $res = $digit . ' ' . $expr[1];
      }
      else {
        $res = $digit . ' ' . $expr[2];
      }
    }
    return trim($res);
  }


  /**
   * Счетчик обратного отсчета (для комментариев).
   */
  public static function downcounter($date, $time_this = NULL) {
    if ($time_this == NULL) {
      $time_this = time();
    }
    if ($time_this > $date) {
      $check_time = $time_this - $date;
    }
    else {
      $check_time = $date - $time_this;
    }

    $days = floor($check_time / 86400);
    $hours = floor(($check_time % 86400) / 3600);
    $minutes = floor(($check_time % 3600) / 60);
    $seconds = $check_time % 60;
    $str = '';
    if ($days > 0) {
      $str .= frz_tweaks_helper::declension($days, array(
          'день',
          'дня',
          'дней'
        )) . ' ';
    }
    if ($hours > 0) {
      $str .= frz_tweaks_helper::declension($hours, array(
          'час',
          'часа',
          'часов'
        )) . ' ';
    }
    if ($minutes > 0) {
      $str .= frz_tweaks_helper::declension($minutes, array(
          'минуту',
          'минуты',
          'минут'
        )) . ' ';
    }
// if($seconds > 0) $str .= declension($seconds,array('секунду','секунды','секунд'));
    return $str;
  }


  /*
  * Выводит аватар пользователя
  */
  public static function userpicture_default($name) {
    $output = '';

    $classes = array(
      'label-default',
      'label-primary',
      'label-success',
      'label-info',
      'label-warning',
      'label-danger',
    );

    $class = $classes[rand(0, count($classes) - 1)];

    $name = check_plain($name);
    $name = drupal_strtoupper(trim($name));
    $name = drupal_substr($name, 0, 1);

    if (!empty($name)) {
      $output .= '<div class="userpicture-default ' . $class . '">';
      $output .= $name;
      $output .= '</div>';
    }
    return $output;
  }

  /**
   * @param tid
   *   Term ID
   * @param child_count
   *   TRUE - Also count all nodes in child terms (if they exists) - Default
   *   FALSE - Count only nodes related to Term ID
   */
  public static function term_nc($tid, $child_count = TRUE) {
    $tids = array($tid);
    if ($child_count) {
      $tids = array_merge($tids, frz_tweaks_helper::term_get_children_ids($tid));
    }

    global $language;
    $langs = array($language->language);
    $langs[] = 'und';
    $query = db_select('taxonomy_index', 't');
    $query->condition('tid', $tids, 'IN');
    $query->join('node', 'n', 't.nid = n.nid');
    $query->condition('n.status', 1, '=');
    $query->condition('n.language', $langs, 'IN');
    $count = $query->countQuery()->execute()->fetchField();
    return $count;
  }

  /**
   * Retrieve ids of term children .
   *
   * @param $tid
   *   The term's ID.
   * @param $tids
   *   An array where ids of term children will be added
   */
  public static function term_get_children_ids($tid) {
    $children = taxonomy_get_children($tid);
    $tids = array();
    if (!empty($children)) {
      foreach ($children as $child) {
        $tids[] = $child->tid;
        $tids = array_merge($tids, frz_tweaks_helper::term_get_children_ids($child->tid));
      }
    }
    return $tids;
  }
}



