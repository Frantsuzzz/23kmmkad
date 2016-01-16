(function ($) {
    Drupal.behaviors.uber_tags = {
        attach : function(context, settings) {

            //создаем меню с якорями
            if($('.itemlist-terms').length > 0){
               var itemList = $('.itemlist-terms');
               var items_wrap = jQuery('<div/>').addClass('itemlist-terms-wrapper').prependTo('.itemlist-terms');
               var sub_ul = jQuery('<ul>/').addClass('nav nav-pills').appendTo(items_wrap);

                itemList.find('h3').each(function (){
                    var name = $(this).html();
                    $(this).prepend('<a name="' + name + '">');

                    var sub_li = jQuery('<li>/').appendTo(sub_ul);
                    var sub_label = jQuery('<a/>').attr('href', "#" + name).html(name);
                    sub_label.appendTo(sub_li);


            });
        }

    }
    };
})(jQuery);
