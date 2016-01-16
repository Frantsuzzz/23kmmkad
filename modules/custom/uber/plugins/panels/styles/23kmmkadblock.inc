<?php


/**
 * @file
 * Definition of the 'niceblock' panel style.
 */

// Plugin definition
$plugin = array(
  'title' => t('23kmmkad block'),
  'description' => t('23kmmkad block (alpha-avto).'),
  'render pane' => 'uber_panels_23kmmkad_style_render_pane',
  'pane settings form' => 'uber_panels_23kmmkad_style_settings_form',
);

/**
 * Render callback.
 *
 * @ingroup themeable
 */
function theme_uber_panels_23kmmkad_style_render_pane($vars) {
 // dpm($vars);
  $content = $vars['content'];
  $pane = $vars['pane'];
  $display = $vars['display'];

  if (empty($content->content)) {
    return;
  }


//  $display = $vars['display'];
//  $region_id = $vars['region_id'];
//  $panes = $vars['panes'];
 $title = $vars['content']->title;
 $desc = $vars['settings']['desc'];
 $class = isset($vars['content']->css_class) ? $vars['content']->css_class : '';
 $css_id  = isset($vars['content']->css_id) ? 'id="' . $vars['content']->css_id . '"' : FALSE;

  unset($vars['content']->title);
  unset($vars['content']->css_class);
  unset($vars['content']->css_id);
  $content = theme('panels_pane', array('content' => $content, 'pane' => $pane, 'display' => $display));



  $output = '';
  $output .= '<div class="kmmkad-block clearfix ' . $class . '" ' . $css_id . '><div class="block-content">';

  if(!empty($title)){
    $selector = !empty($vars['settings']['selector']) ? $vars['settings']['selector'] : 'div';
    $output .= '<'. $selector . ' class="block-title">' . $title . '</' . $selector . '>';
  }
  if(!empty($desc)){
    $output .= '<div class="block-desc">' . $desc . '</div>';
  }
  $output .= '<div class="kmmkad-content">'. $content .'</div>';

  $output .= '</div></div>';


  return $output;
}

/**
 * Settings form callback.
 */
function uber_panels_23kmmkad_style_settings_form($style_settings) {

  $form['desc'] = array(
    '#type' => 'textfield',
    '#title' => t('Description'),
    '#size' => 255,
    '#default_value' => $style_settings['desc'],
  );
  $form['selector'] = array(
    '#type' => 'select',
    '#title' => t('Selector title'),
    '#default_value' => $style_settings['selector'],
    '#options' => array(
      'div' => 'div',
      'h1' => 'h1',
      'h2' => 'h2',
      'h3' => 'h3',
    ),
  );

  return $form;
}
