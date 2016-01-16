<?php

$plugin = array(
  'title'           => t('Uber ticket'),
  // Название плагина, отображаемое при его выборе.
  'description'     => t('Ticket block for 23kmmkad.ru'),
  // Описание плагина, отображаемое при его выборе.
  'render callback' => 'uber_ticket_content_type_render',
  // Функция, которая будет формировать и выводить содержимое плагина.
  'edit form'       => 'uber_ticket_edit_form',
  // Форма с настройками плагина.
  'category'        => t('Uber'),
  // Категория, в которой будет находиться плагин.
  'single'          => TRUE,
  // Флаг, указывающий является ли плагин подтипом другого.
);

/**
 * 'Edit form' callback for the content type.
 */
function uber_ticket_edit_form($form, &$form_state) {
  // Получаем ранее введенные настройки.
  $conf = $form_state['conf'];

  $form['text'] = array(
    '#type'          => 'textarea',
    '#title'         => t('Custom text'),
    '#default_value' => !empty($conf['text']) ? $conf['text'] : '',
  );


  $options = array('none' => 'none');
  if ($field = field_info_instance('taxonomy_term', 'field_ticket', 'team')) {
    if (isset($field['widget']['settings']['first']['select']['allowed_values'])) {
      foreach ($field['widget']['settings']['first']['select']['allowed_values'] as $key => $value) {
        $options[$key] = $value;
      }
    }
  }

  $form['type'] = array(
    '#type'          => 'select',
    '#title'         => t('Type ticket'),
    '#default_value' => !empty($conf['type']) ? $conf['type'] : '',
    '#options'       => $options,
  );

  return $form;
}

/**
 * Submit callback for the example_text_edit_form().
 */
function uber_ticket_edit_form_submit($form, &$form_state) {
  // Сохраняем настройки.
  $form_state['conf'] = array(
    'text' => $form_state['values']['text'],
    'type' => $form_state['values']['type'],
  );
}

/**
 * Render callback for the content type.
 */
function uber_ticket_content_type_render($subtype, $conf, $args, $context) {
  $output = '';
  // Если в настройках плагина не ввели текст, то ничего не выводим.
  if (!empty($conf['text'])) {
    $output .= '<div class="uber-ticked-block-text">' . check_markup($conf['text'], 'full_html') . '</div>';
  }

  if (!empty($conf['type'])) {
    $output .= '<div class="uber-ticked-block-ticked">' . uber_load_ticket($conf['type']) . '</div>';
  }

  if ($output == '') {
    return;
  }

  $block = new stdClass();
  $block->content = $output;
  return $block;
}
