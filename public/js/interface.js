function createWaveCanvas()
{
	var canvas = new WaveformEditor();
	$('#waveform_editor').append(canvas.getCanvas());
	return canvas;
}

function updateCurrentWaveform(samples)
{
	var data = 'samples=';
	for (var i = 0; i < samples.length; i++)
	{
		data += (samples[i] + ',');
	}
	$.post('services/data.php?job=update_wave', data, function(d, s) {waveformHasUpdated(d, s);});
}

function waveformHasUpdated(d, s)
{
	refreshAudioPlayer();
	$('.wavoption').removeAttr('disabled');
	$('.wavoption-share').removeAttr('disabled');
}

function fillFileName(el)
{
	var fn = $(el).val();
	if (fn != '') {
		$('#upload_button').removeAttr('disabled');
	}
	var elid = $(el).attr('id') + '_name';
	$('#' + elid).val(fn);
	$('#invalid_file').hide();
	
}

function badFileError()
{
	$('#invalid_file').show();
}

function refreshMyWaveforms()
{
	$('#invalid_file').hide();
	$.get('services/data.php?job=session_list', function(d, s) {populateMyWaveforms(d, s);});
}

function populateMyWaveforms(d, s)
{
	var data = JSON.parse(d);
	if (data !== null && data.length) {
		var table = jQuery('<table class="table table-condensed table-hover table-striped"></table>');
		var f = 0;
		for (var i = data.length - 1; i >= 0; i--)
		{
			var name = data[i]['name'];
			if (!name) {continue;}
			f++;
			var tr = jQuery('<tr id="my' + i + '"></tr>');
			var td = jQuery('<td class="col-md-9 col-lg-9"></td>');
			td.html(name);
			tr.append(td);
			
			var load = '<button data-loading-text="<span class=\'glyphicon glyphicon-refresh spinning\'></span>" class="btn btn-sm btn-primary start" title="Open in Waveform Generator" onclick="loadMyWaveform(' + i + ', this)"><span class="glyphicon glyphicon-export"></span> </button>';
			var del = '<button class="btn btn-sm btn-primary" onclick="deleteMyWaveform(' + i + ')" title="Remove from My Waveforms"><span class="glyphicon glyphicon-trash"></span> </button>';
			var td = jQuery('<td class="col-md-3 col-lg-3 align_right">' + load + del + '</td>');
			tr.append(td);
			
			table.append(tr);
		}
		if (!f) {return;}
		$('#session').empty().append(table);
	}
}

function deleteMyWaveform(id)
{
	$.get('services/data.php?job=delete_session&id=' + id);
	$('#my' + id).remove();
}

function loadMyWaveform(id, button)
{
	$(button).button('loading');
	$('#upload_file_name').val('');
	$('#upload_button').attr('disabled', 1);
	$.get('services/data.php?job=load_session&id=' + id, function(d, s) {loadIntoEditor(d, s, false);});
}

function loadSharedWaveform(id, button)
{
	$(button).button('loading');
	$('#upload_file_name').val('');
	$('#upload_button').attr('disabled', 1);
	$.get('services/data.php?job=load_shared&id=' + id, function(d, s) {loadIntoEditor(d, s, true);});
}

function loadIntoEditor(d, s, shared)
{
	var data = JSON.parse(d);
	if (data !== null) {
		var samples = data['samples'];
		var name = data['name'];
		
		// Get a head start on the player: 
		refreshAudioPlayer();
		
		$('.start').button('reset');
		$('#invalid_file').hide();
		$('.wavoption').removeAttr('disabled');
		if (shared) {$('.wavoption-share').attr('disabled', 1);}
		
		$('#current_waveform_name').html(name);
		waveCanvas.samples = samples;
		waveCanvas.draw();
	}
}

function refreshAudioPlayer()
{
	// The high pitch (A440) player
	var audio = jQuery('<audio oncanplay="enableAudioControls()" id="audio_player_A440"></audio>');
	// Add a random number to avoid stubborn caching
	var source = jQuery('<source src="services/wav.php?&random_thing=' + Math.random() + '">');
	audio.append(source);
	$('#player').empty().append(audio);
			
	// The low pitch (A220) player
	var audio = jQuery('<audio oncanplay="enableAudioControls()" id="audio_player_A220"></audio>');
	var source = jQuery('<source src="services/wav.php?low=1&random_thing=' + Math.random() + '">');
	audio.append(source);
	$('#player').append(audio);
}

function shareWaveform()
{
	// Get an encode field values
	var n = $('#share_name').val();
	var d = $('#share_desc').val();
	var s = $('#share_sig').val();
	n = encodeURI(n);
	d = encodeURI(d);
	s = encodeURI(s);
	
	// Reset for the next share
	$('#share_name').val('');
	$('#share_desc').val('');
	
	$('#share_wave').modal('hide');
	$('.wavoption-share').attr('disabled', 1);
	
	$.get('services/data.php?job=share_wave&name=' + n + '&desc=' + d + '&sig=' + s, function(d, s) {showThanksBox(d);refreshSharedWaveforms();});
}

function showThanksBox(d)
{
	var data = JSON.parse(d);
	if (data !== null) {
		if (data['status'] == 'Duplicate') {
			$('#duplicate').show();
			$('#thanks').hide();
		} else if (data['status'] == 'OK') {
			$('#thanks').show();
			$('#duplicate').hide();
		}
	}
}

function refreshSharedWaveforms()
{	
	$.get('services/data.php?job=shared_list', function(d, s) {populateSharedWaveforms(d, s);});
}

function populateSharedWaveforms(d, s)
{
	var data = JSON.parse(d);
	if (data !== null && data.length) {
		var table = jQuery('<table id="shared_waveform_table" class="table table-condensed table-hover table-striped"></table>');
		var f = 0;
		for (var i = 0; i < data.length; i++)
		{
			var name = data[i]['name'];
			var id = data[i]['id'];
			if (!name) {continue;}
			f++;
			var tr = jQuery('<tr id="shared' + id + '"></tr>');
			var td = jQuery('<td class="waveform_name col-md-9 col-lg-9"></td>');
			td.html(name);
			tr.append(td);
			
			var load = '<button data-loading-text="<span class=\'glyphicon glyphicon-refresh spinning\'></span>" class="btn btn-sm btn-primary start" title="Open in Waveform Generator" onclick="loadSharedWaveform(' + id + ', this)"><span class="glyphicon glyphicon-export"></span> </button>';
			var info = '<button class="btn btn-sm btn-primary" onclick="getInformation(' + id + ')" title="Waveform Information"><span class="glyphicon glyphicon-info-sign"></span> </button>';
			var td = jQuery('<td class="align_right col-md-3 col-lg-3">' + load + info + '</td>');
			tr.append(td);
			
			table.append(tr);
		}
		if (!f) {return;}
		$('#shared').empty().append(table);
	}
}

function showAllWaveforms()
{
	$('#shared_query').val('');
	$('#shared_waveform_table TR').show();
}

function searchWaveforms()
{
	var q = new RegExp($('#shared_query').val(), 'i');
	$('#shared_waveform_table TR').hide();
	$('.open_description').remove();
	$.each($('#shared_waveform_table TR'), function() 
			{
				var n = $(this).find('.waveform_name').html();
				if (n.match(q)) {$(this).show();}
			}
	);
}

function enableAudioControls()
{
	$('.audio_control').removeAttr('disabled');
}

function play(pitch)
{
	document.getElementById('audio_player_' + pitch).play();
}

function getInformation(id)
{
	$.get('services/data.php?job=info&id=' + id, function(d, s) {displayInformation(d, s);});
}

function displayInformation(d, s)
{
	var data = JSON.parse(d);
	if (data !== null) {
		$('.open_description').remove();
		var id = data['id'];
		var desc = data['description'];
		var time = data['share_time'];
		var signature = data['signature'];
		var samples = data['samples'];
		var sigline = signature ? ('<strong>@' + signature + '</strong> ') : '';
		
		var div = jQuery('<div class="open_description panel panel-default"></div>');
		var body = jQuery('<div class="panel-body"></div>');
		var close = jQuery('<p><button class="close" type="button" onclick="$(\'.open_description\').remove()" class="pull-right">&times;</button></p>');
		body.append(close);
		var desc = jQuery('<p>' + desc + '</p>');
		body.append(desc);
		var thumb = jQuery('<div id="wave_thumb"></div>');
		body.append(thumb);
		var time = jQuery('<p><i>' + sigline + 'shared on ' + time + '</i></p>');
		body.append(time);
		div.append(body);
		
		$('#shared' + id + ' .waveform_name').append(div);
		var wavethumb = new WaveThumbnail(samples);
		wavethumb.drawInto('wave_thumb');
	}
	
}

function maximize()
{
	$.get('services/data.php?job=maximize', function(d, s) {loadIntoEditor(d, s, 0);refreshMyWaveforms();});
}