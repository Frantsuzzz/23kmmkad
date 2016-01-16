<?php

/**
 * @file
 * Default simple view template to display a list of rows.
 *
 * - $title : The title of this group of rows.  May be empty.
 * - $options['type'] will either be ul or ol.
 * @ingroup views_templates
 */
$bp_grid_class = (strpos($list_type_prefix, 'class="bootstrap-grid"') !== FALSE) ? 'bp-grid-item' : '';

?>
<?php print $wrapper_prefix; ?>
<?php if (!empty($title)) : ?>
  <h3><?php print $title; ?></h3>
<?php endif; ?>
<?php print $list_type_prefix; ?>
<?php foreach ($rows as $id => $row): ?>
  <li class="<?php print $classes_array[$id] . ' ' . $bp_grid_class; ?>"><?php print $row; ?></li>
  <?php if ($bp_grid_class ==  'bp-grid-item'): ?>
    <li class="bp-grid-clearfix"></li>
  <?php endif; ?>
<?php endforeach; ?>
<?php print $list_type_suffix; ?>
<?php print $wrapper_suffix; ?>
