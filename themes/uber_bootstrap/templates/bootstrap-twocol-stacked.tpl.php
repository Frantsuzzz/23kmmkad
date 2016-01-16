<div class="<?php print $classes ?>" <?php if (!empty($css_id)) {
  print "id=\"$css_id\"";
} ?>>

  <?php print $content['top']; ?>

  <div class="bootstrap-column-wrapper">
    <div class="container">
      <div class="row">
        <?php print $content['left']; ?>
        <?php print $content['right']; ?>
      </div>
    </div>
  </div>

    <?php print $content['bottom']; ?>
</div>
