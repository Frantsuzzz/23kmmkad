<?php
if($media){
if($right){
  $class = 'pull-right';
}else{
  $class = 'pull-left';
}
}else{
  $class = 'pull';
}

?>
<div class="uber-ticket">
  <div class="media">

    <?php if(!$media): ?>
    <div class="media-body">
      <?php if ($msg): ?>
        <div class="msg"><?php print $msg; ?></div>
      <?php endif; ?>
    </div>
    <?php endif; ?>

    <div class="<?php print $class; ?>">
      <?php if ($img): ?>
        <div class="img"><?php print $img; ?></div>
      <?php endif; ?>
      <?php if ($name): ?>
        <div class="name"><?php print $name; ?></div>
      <?php endif; ?>
    </div>

    <?php if($media): ?>
    <div class="media-body">
      <?php if ($msg): ?>
        <div class="msg"><?php print $msg; ?></div>
      <?php endif; ?>
    </div>
    <?php endif; ?>

  </div>
</div>
