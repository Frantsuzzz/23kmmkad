(function ($) {

    $(document).ready(function () {
        /* Скролл страницы */
        $(window).scroll(function () {
            if ($(this).scrollTop() > 100) {
                $('#scrollup').fadeIn();
            } else {
                $('#scrollup').fadeOut();
            }
        });

        $('body').append('<a href="#" id="scrollup" title="Top page" class="img-circle"><span class="glyphicon glyphicon-chevron-up"></span></a>');
        $('body').on('click', 'a#scrollup', function () {
            $("html, body").animate({ scrollTop: 0 }, 600);
            return false;
        });

        $('#block-menu-menu-catalog ul.menu>li>a').click(function () {
            if ($(this).parent().find('ul').length) {
                $(this).parent().find('ul').first().slideToggle(200);
                return false;
            }
        });

        $('#block-menu-menu-catalog li.active-trail ul').show();

    });

})(jQuery);

(function ($) {

    Drupal.behaviors.frz_bootstrap = {
        attach: function (context, settings) {
            //создаем меню с якорями

            if ($('#modalContent').length > 0) {
                $('#modalContent').each(function () {
                    if ($(this).find('form').length > 0) {
                        $(this).addClass('modal-webform');
                    } else {
                        $(this).removeClass('modal-webform');
                    }
                });
            }

            $('.block-readmore .pane-content').readmore({
                speed: 75,
                maxHeight: 180,
                moreLink: '<a href="#" class="link-read-more btn btn-primary">Показать остальное...</a>',
                lessLink: '<a href="#" class="link-read-more unmore btn btn-default">Скрыть</a>'
            });

            //редактируем bootstrap-сетку для списков ul>li
            if ($('#gallery-nested').length > 0) {
                var ul_nested = $('#gallery-nested .view-content .item-list>ul');


                ul_nested.nested(
                    {
                        selector: 'li',
                        minWidth: 200,
                        gutter: 10
                    }
                );



            }

        }
    };


    Drupal.behaviors.AlingnPopup = {
        attach: function () {
            popup = $('#modalContent');
            // Function align element in the middle of the screen.
            function alignCenter() {
                // Popup size.
                var wpopup = popup.width();
                var hpopup = popup.height();

                // Window size.
                var hwindow = $(window).height();
                var wwindow = $(window).width();
                if (wpopup >= wwindow) {

                }

                var Left = Math.max(40, parseInt($(window).width() / 2 - wpopup / 2));
                var Top = Math.max(40, parseInt($(window).height() / 2 - hpopup / 2));

                if (hwindow < hpopup) {
                    popup.css({'position': 'absolute'});
                }
                else {
                    popup.css({'position': 'fixed'});
                }
                popup.css({'left': Left, 'top': Top});
            }

            alignCenter();

            $(window).resize(function () {
                alignCenter();
            });

            popup.resize(function () {
                alignCenter();
            });
        }
    };
})(jQuery);
