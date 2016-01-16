<?php
/**
 * Field API implementations.
 */

/**
 * Implements hook_field_formatter_info().
 */
function uber_field_formatter_info() {
  $formatters = array(
    'body_spoler' => array(
      'label' => "Спойлер",
      'field types' => array('text_with_summary'),
      'settings' => array(
        'body_size' => 300,
      )
    ),
  );
  return $formatters;
}

/**
 * Implements hook_field_formatter_settings_form().
 */
function uber_field_formatter_settings_form($field, $instance, $view_mode, $form, &$form_state) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings'];
  $element = array();

  // Category icon formatter.
  if ($display['type'] == 'body_spoler') {
    $element['body_size'] = array(
      '#type' => 'textfield',
      '#title' => t('Body size'),
      '#default_value' => $settings['body_size'],
    );
  }

  return $element;
}

/**
 * Implements hook_field_formatter_settings_summary().
 */
function uber_field_formatter_settings_summary($field, $instance, $view_mode) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings'];

  if ($display['type'] == 'body_spoler') {
    $summary = $settings['body_size'];
  }

  return $summary;
}

/**
 * Implements hook_field_formatter_view().
 */
function uber_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
  $element = array();
  $settings = $display['settings'];

  if ($display['type'] == 'body_spoler') {
    $bodys = array();
    foreach ($items as $delta => $item) {
      if (drupal_strlen($item['value']) > $settings['body_size']) {
        $part1 = drupal_substr($item['value'], 0, $settings['body_size']);
        $part1 = check_markup($part1 . '... ', $item['format']) . ' <a class="body-spoler-button" href=#>Показать подробнее</a>';
        $part2 = check_markup($item['value'], $item['format']);

        $bodys[$delta] = '<div class="body-spoler open"><span class="body-spoler-visible">' . $part1 . '</span><span class="body-spoler-hidden">' . $part2 . '<a href=# class="body-spoler-button">Скрыть информацию</a></span></div>';
        $element[0]['#attached']['js'] = array(
          drupal_get_path('module', 'uber') . '/js/uber_bodyfield.js',
        );
        $element[0]['#attached']['css'] = array(
          drupal_get_path('module', 'uber') . '/css/uber_field.css',
        );
      }
      else {
        $bodys[$delta] = check_markup($item['value'], $item['format']);
      }
    }
    $element[0]['#markup'] = implode('</ br>', $bodys);
  }

  return $element;
}
