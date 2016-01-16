<div class="row">
  <div class="col-md-9">
    <form class="form-inline-search" role="form" action="search" metod="GET">
      <div class="row">
      <div class="col-sm-8">
        <label class="control-label" for="txt">Поиск</label>
        <input type="text" class="form-control" name="txt" id="txt"
               placeholder="Введите номер детали или его часть">
      </div>

      <div class="col-sm-4">
        <button type="submit" class="btn btn-danger"><span
            class="glyphicon glyphicon-search"></span>Найти
        </button>
      </div>
      </div>
    </form>

    <div class="uber-search-front-line row">
      <div class="col-sm-8">
        Поиск по контексту поможет Вам найти необходимые узлы и запчасти
      </div>
      <div class="col-sm-4">
        <a class="btn btn-default" href="/search">Перейти к поиску</a>
      </div>
    </div>
    <div class="uber-search-front-line row">
      <div class="col-sm-8">Продуманный и простой каталог в
        несколько кликов приведет Вас к необходимой детали
      </div>
      <div class="col-sm-4">
        <a class="btn btn-default" href="/catalog">Перейти к каталогу</a>
      </div>
    </div>
  </div>

  <div class="col-md-3">
    <?php if ($ticket): ?>
      <?php print $ticket; ?>
    <?php endif; ?>
  </div>
</div>

