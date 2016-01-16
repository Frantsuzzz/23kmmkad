<?php
//форма с основными настройками модуля
function uber_queue_form($form, &$form_state) {
  $form['uber_queue_number'] = array(
    '#markup' => '<div>' . variable_get('uber_queue_number', 0) . '</div>',
  );

  $form['uber_queue_count'] = array(
    '#default_value'    => variable_get('uber_queue_count', 10),
    '#title'            => t('Count'),
    '#type'             => 'textfield',
    '#description'      => 'Count items',
    '#element_validate' => array('element_validate_integer_positive'),
  );

  $form['uber_queue_limit'] = array(
    '#default_value'    => variable_get('uber_queue_limit', 0),
    '#title'            => t('Limit'),
    '#type'             => 'textfield',
    '#description'      => 'Max count items',
    '#element_validate' => array('element_validate_integer'),
  );

  $form['uber_queue_play'] = array(
    '#default_value'    => variable_get('uber_queue_play', 0),
    '#title'            => t('Play'),
    '#type'             => 'select',
    '#description'      => 'Play/Pause',
    '#element_validate' => array('element_validate_integer'),
    '#options'          => array('Stop', 'Play'),
  );

  $form['uber_queue_debug'] = array(
    '#default_value'    => variable_get('uber_queue_debug', 0),
    '#title'            => t('Debug'),
    '#type'             => 'select',
    '#description'      => 'Level debug',
    '#element_validate' => array('element_validate_integer'),
    '#options'          => array(
      0 => 'none',
      1 => 'uber_queue_history',
      2 => 'uber_queue_history_items',
    ),
  );

  return system_settings_form($form);
}
