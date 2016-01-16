(function ($) {
    $(document).ready(function() {
$('.body-spoler').removeClass('open');
            $(".body-spoler-button").click(function () {
                var body = $(this).parents('.body-spoler');
                body.toggleClass("open");
               return false;
                });
         });
})(jQuery);
