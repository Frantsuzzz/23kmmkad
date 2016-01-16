<?php

function uber_queue_node_update_form($form, &$form_state) {
  $form = array();

  if (!isset($form_state['build_info']['args'][0]) || !($node = $form_state['build_info']['args'][0])) {
    drupal_set_message(t('Error node_load'), 'error');
    return $form;
  }

  if ($node && $node->type != 'product') {
    drupal_set_message(t('Node type:!type no import', array('!type' => $node->type)), 'error');
    return $form;
  }

$form['node'] = array(
  '#type' => 'value',
  '#value' => $node,
);

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
  );

  return $form;
}

function uber_queue_node_update_form_submit($form, &$form_state) {

  if(isset($form_state['values']['node'])){
    $node = $form_state['values']['node'];
    $status_op = uber_queue_update_node($node);
    if($status_op){
      drupal_set_message($status_op);
    }else{
      drupal_set_message('Error');
    }
  }
}



