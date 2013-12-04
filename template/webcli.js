var __refresh = true;
var __fix_scroll = true;

$( document ).ready(function() {
	scrollToBottom();
	if(!__disabled) {
		//setInterval(function(){update_before()},1000);
		setInterval(function(){update_after() },1000);
	}

	$("#input-form").submit(function(event) {
		event.preventDefault();
		var url = "/?" + $(this).serialize() + "&source=js"
		$.getJSON(url, function(data, status) {
			if(!(status == "success" && data["success"] == "true")) {
				// TODO: Notify the user somehow! 
				$("#command").val("");
			} else {
				$("#command").val("");
			}
		});
	});
});

function update_before() {
	
}

function update_after(){
	if(!__refresh) {
		return;
	}
	
    if (Math.abs($('#output').prop("scrollHeight") - ($('#output').height() + $('#output').scrollTop())) < 50 ) {
        __fix_scroll = true;
    } else {
        __fix_scroll = false;
    }
    
    //var sum = $('#output').height() + $('#output').scrollTop();
    //$("#command").val("ScrollHeight: "+$('#output').prop("scrollHeight") + " ContentHeight: "+$('#output').height() + " ScrollTop: "+$('#output').scrollTop() + " Calculation: "+sum);
    
    $.getJSON(url_json_after + url_time_after, function( data, status ) {
    	$.each(data, function (i, value) {
		    var content = "<span class=\"time\">" + value[0].time + "</span>" + value[1].line;
		    url_time_after = value[0].time;
		    $("#output").append($(document.createElement("div")).addClass("line").html(content));
		});
	});
    
    scrollToBottom();
}

function scrollToBottom() {
	if(__fix_scroll && $('#output').length) {
		$('#output').scrollTop($('#output')[0].scrollHeight);
	}
}
