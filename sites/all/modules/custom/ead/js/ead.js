Drupal.behaviors.myModuleBehavior = function(context) {
    // for EAD Collection views, add highlight class to 
    // ead_component specified by url anchor is anchor is present

    var anchor = jQuery.url.attr("anchor");
    var ead_component = $("#" + anchor);
    if (ead_component.length){
	ead_component.addClass('highlight');
    }
};