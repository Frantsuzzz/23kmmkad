<?php

/**
 * Заменяем локальные задачи нод (табы) контекстными ссылками
 * @link http://xandeadx.ru/blog/drupal/350
 */
/*
function uber_bootstrap_menu_local_task($variables) {
  $link = $variables['element']['#link'];
  if ($link['path'] == 'node/%/view') {
    return FALSE;
  }
  $link['localized_options']['html'] = TRUE;
  return '<li>' . l($link['title'], $link['href'], $link['localized_options']) . '</li>';
}

function uber_bootstrap_menu_local_tasks($variables) {
  $output = '';
  $has_access = user_access('access contextual links');

  if (!empty($variables['primary'])) {
    if ($has_access) {
      drupal_add_library('contextual', 'contextual-links');
    }
    $variables['primary']['#prefix'] = $has_access ? '<div class="contextual-links-wrapper"><ul class="contextual-links">' : '<ul class="tabs primary">';
    $variables['primary']['#suffix'] = $has_access ? '</ul></div>' : '</ul>';
    $output .= drupal_render($variables['primary']);
  }
  if (!empty($variables['secondary'])) {
    $variables['secondary']['#prefix'] = '<ul class="tabs secondary">';
    $variables['secondary']['#suffix'] = '</ul>';
    $output .= drupal_render($variables['secondary']);
  }

  return $output;
}
*/

/**
 * Override or insert variables into the node template.
 */
function uber_bootstrap_preprocess_html(&$vars) {
  $class_line = implode(' ', $vars['classes_array']);
  if (strpos($class_line, 'page-panel-fluid') === FALSE) {
    $vars['classes_array'][] = 'page-nopanel-container';
  }
}

/**
 * Override or insert variables into the node template.
 */
function uber_bootstrap_preprocess_node(&$vars) {
  if ($vars['view_mode'] == 'full') {
    // Получаем регион и ложим его в массив переменных для ноды
    $vars['node_bottom'] = block_get_blocks_by_region('node_bottom');

    if ($vars['type'] == 'product') {
      drupal_add_js(path_to_theme() . '/js/flex.js');
      // Получаем регион и ложим его в массив переменных для ноды
      $vars['product_right'] = block_get_blocks_by_region('product_right');
    }

  }
  else {
    $vars['node_bottom'] = '';
  }
}

/**
 * Override or insert variables for the page templates.
 */
function uber_bootstrap_preprocess_page(&$vars) {
 // drupal_add_js(libraries_get_path('flexslider') . '/jquery.flexslider-min.js');

  $arg = arg();
  if (count($arg) == 1 && $arg[0] == 'tags') {
    drupal_add_js(path_to_theme() . '/js/page_tags.js');
  }
  if (count($arg) == 1 && $arg[0] == 'contacts') {
    drupal_add_js(path_to_theme() . '/js/jquery.nested.js');
  }

}

/**
 * Override or insert variables into the node template.
 */
function uber_bootstrap_preprocess_taxonomy_term(&$vars) {

  //Добавляем контекстные ссылки для taxonomy_term display
  if ($vars['view_mode'] == 'teaser' || $vars['view_mode'] == 'teaser_mini') {
    $vars['teaser'] = TRUE;
    $vars['classes_array'][] = 'contextual-links-region';
    $vars['classes_array'][] = 'taxonomy-term-' . $vars['view_mode'];


    $vars['title_suffix'] = array(
      'contextual_links' => array(
        '#type'             => 'contextual_links',
        '#contextual_links' => array(
          'taxonomy_term' => array('taxonomy/term', array($vars['tid'])),
        ),
      ),
    );
  }


  //Бонусы для продуктов
  if ($vars['view_mode'] == 'teaser' && $vars['vocabulary_machine_name'] == 'bonus') {
    $vars['content']['title'] = array(
      '#markup' => '<div class="title">' . $vars['name'] . '</div>',
      '#weight' => -10,
    );
    $vars['content']['field_image']['#weight'] = -20;
  }

  //Команда
  if ($vars['view_mode'] == 'teaser_mini' && $vars['vocabulary_machine_name'] == 'team') {
    $vars['content']['title'] = array(
      '#markup' => '<div class="title">' . $vars['name'] . '</div>',
      '#weight' => -10,
    );
    $vars['content']['field_image']['#weight'] = -20;

    $class = array(
      'ctools-use-modal',
      'ctools-modal-uber',
      'btn',
      'btn-default',
      'btn-lg'
    );
    $menager_link = l('Связаться с менеджером', '/ajax/nojs/41559/' . $vars['tid'], array('attributes' => array('class' => $class)));
    $vars['content']['menager_link'] = array(
      '#markup' => '<div class="btn-menager-link">' . $menager_link . '</div>',
      '#weight' => 100,
    );
  }

  //МАрки авто и категории
  if ($vars['view_mode'] == 'teaser' && array_search($vars['vocabulary_machine_name'], array(
      'marks',
      'catalog'
    )) !== FALSE
  ) {
    $title = l(check_plain($vars['name']), 'taxonomy/term/' . $vars['tid']);
    $btn = l('Перейти', 'taxonomy/term/' . $vars['tid']);
    $vars['content']['title'] = array(
      '#markup' => '<div class="title">' . $title . '</div>',
      '#weight' => -10,
    );
    $vars['content']['field_image']['#weight'] = -20;

    $node_count = frz_tweaks_helper::term_nc($vars['tid'], FALSE);
    $txt = format_plural($node_count, '@count product', '@count products');
    //$txt = t('!nc products', array('!nc' => $node_count));
    if (isset($vars['field_cargo'][LANGUAGE_NONE][0]['value']) && $vars['field_cargo'][LANGUAGE_NONE][0]['value'] == 1) {
      $tooltip = t('There are spare parts for trucks');
      $field_cargo_icon = '<div class="cargo-icon" data-toggle="tooltip" data-placement="top" title="' . $tooltip . '" ></div>';
    }
    else {
      $field_cargo_icon = '';
    }

    $vars['content']['node_count'] = array(
      '#markup' => '<div class="node-count">' . $txt . $field_cargo_icon . '</div>',
      '#weight' => 1,
    );
    $vars['content']['btn-link'] = array(
      '#markup' => '<div class="btn-link">' . $btn . '</div>',
      '#weight' => 50,
    );
  }

  //Категории авто
  if ($vars['view_mode'] == 'teaser' && array_search($vars['vocabulary_machine_name'], array('catalog')) !== FALSE) {

    $childrens = taxonomy_get_children($vars['tid']);
    $list_children = array();
    foreach ($childrens as $children) {
      $list_children[] = l($children->name, 'taxonomy/term/' . $children->tid);
    }
    $count_childrens = count($list_children);
    $title = format_plural($count_childrens, '@count category', '@count categories');
    $options = array(
      'title' => $title,
      'items' => $list_children,
    );
    $vars['content']['subcategories'] = array(
      '#markup' => '<div class="subcategories-list">' . theme('bootstrap_dropdown_button', $options) . '</div>',
      '#weight' => -10,
    );

    // dpm($vars);
  }
}


/*
 * Bootstrap override
 * make dropdown menus do dropdown on hover,
 */

function uber_bootstrap_menu_link(array $variables) {
  $element = $variables['element'];
  $sub_menu = '';
  $element['#attributes']['class'][] = ($element['#below']) ? 'menu-dropdown' : 'menu-nodropdown';

  if ($element['#below']) {
    // Prevent dropdown functions from being added to management menu so it
    // does not affect the navbar module.
    if ((($element['#original_link']['menu_name'] == 'management') && (module_exists('navbar'))) || $element['#original_link']['menu_name'] == 'menu-catalog') {
      $sub_menu = drupal_render($element['#below']);
    }
    elseif ((!empty($element['#original_link']['depth'])) && ($element['#original_link']['depth'] == 1)) {
      // Add our own wrapper.
      unset($element['#below']['#theme_wrappers']);
      $sub_menu = '<ul class="dropdown-menu">' . drupal_render($element['#below']) . '</ul>';
      // Generate as standard dropdown.
      $element['#title'] .= ' <span class="caret"></span>';
      $element['#attributes']['class'][] = 'dropdown';
      $element['#localized_options']['html'] = TRUE;

      // Set dropdown trigger element to # to prevent inadvertant page loading
      // when a submenu link is clicked.
      $element['#localized_options']['attributes']['data-target'] = '#';
      $element['#localized_options']['attributes']['class'][] = 'dropdown-toggle';
      $element['#localized_options']['attributes']['data-toggle'] = 'dropdown';
    }
  }
  // On primary navigation menu, class 'active' is not set on active menu item.
  // @see https://drupal.org/node/1896674
  if (($element['#href'] == $_GET['q'] || ($element['#href'] == '<front>' && drupal_is_front_page())) && (empty($element['#localized_options']['language']))) {
    $element['#attributes']['class'][] = 'active';
  }
  $output = l($element['#title'], $element['#href'], $element['#localized_options']);
  return '<li' . drupal_attributes($element['#attributes']) . '>' . $output . $sub_menu . "</li>\n";
}

function uber_bootstrap_menu_tree__main_menu($variables) {
  return '<ul class="menu nav navbar-nav nav-tabs nav-justified">' . $variables['tree'] . '</ul>';
}
