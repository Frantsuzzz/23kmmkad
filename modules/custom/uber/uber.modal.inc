<?php

/**
 * Determines whether the current user may perform the operation on the node.
 *
 * @param $node
 *   The node object on which the operation is to be performed.
 *
 * @return bool
 *   TRUE if the operation may be performed, FALSE otherwise.
 */
function _uber_ajaxwebform_node_access($node) {
  if (!is_object($node) || $node->type != 'webform') {
    return FALSE;
  }

  return node_access('view', $node);
}

/*
 * Show webform in popup.
 */
function uber_webform_order_ajax($js = NULL, $node, $product) {
  if (!$js) {
    drupal_goto('node/' . $node->nid);
  }
  ctools_include('modal');
  $submission = (object) array();
  $submission = FALSE;
  $form_state = array(
    'title' => $node->title,
    'ajax' => TRUE,
    'build_info' => array(
      'args' => array($node, $submission),
    ),
  );

  $commands = ctools_modal_form_wrapper('webform_client_form_' . $node->nid, $form_state);
  if (!empty($form_state['executed'])) {
    ctools_include('ajax');
    $commands = array();
    // $commands[] = ctools_ajax_command_redirect('node/' . $node->nid . '/done');
  }
  return array('#type' => 'ajax', '#commands' => $commands);
}

/**
 * Add js and css files.
 */
function _uber_include_modal() {
  static $added = FALSE;
  if (!$added) {
    // Подключаем библиотеки только один раз.
    $added = TRUE;
    // Подключаем библиотеки.
    ctools_include('modal');
    ctools_include('ajax');
    ctools_modal_add_js();

    // Создаем массив с настройками для модального окна.
    $script = array(
      'uber' => array(
        'modalSize' => array(
          'type' => 'fixed',
          'width' => 450,
          'height' => 'auto',
          'contentRight' => 0,
          'contentBottom' => 0,
        ),
        'modalOptions' => array(
          'opacity' => (float) 0.4,
          'background-color' => '#000000',
        ),
        'closeText' => '',
        'animation' => 'fadeIn',
        'animationSpeed' => 'fast',
      ),
    );
    drupal_add_js($script, 'setting');
  }
}

/**
 * Implements hook_init().
 */
function uber_init() {
  // Подключаем библиотеки только на главной странице.
//  if (drupal_is_front_page()) {
  _uber_include_modal();
//  }
}
