function WaveformEditor(samples)
{
	if (samples != null) {
		this.samples = samples;
	} else {
		// Initialize if not provided
		this.samples = [];
		for (var i = 0; i < 128; i++)
		{
			this.samples[i] = 0;
		}
	}
	return this;
}

WaveformEditor.prototype.getCanvas = function() {
	var canvas = '<table id="canvas" onmouseleave="saveWaveform()" onmouseup="toggleDrawState(0)" onmousedown="toggleDrawState(1)">';
	for (var y = 255; y >= 0; y--)
	{
		canvas += '<tr>';
		marker = (y == 128 || y == 0 || y == 255) ? 'zero_line' : '';
		
		for (var x = 0; x < 128; x++)
		{
			canvas += '<td onmouseenter="drawWaveform(' + x + ',' + y +')" class="wave_sample ' + marker + ' sample-' + x +'" id="canvas-' + x + '-' + y + '"></td>';
		}
		canvas += '</tr>\n';
	}
	canvas += '</table>\n';
	return jQuery(canvas);
};

WaveformEditor.prototype.draw = function() {
	$('.wave_sample').removeClass('on'); // Clear the canvas

	for (var x = 0; x < this.samples.length; x++)
	{
		// Sample is a 16-bit sample, so first make it positive or zero, and then divide by
		// 256 because the canvas is an 8-bit space
		var y = parseInt(((this.samples[x] + 32768) / 256) + .5);
		var loc = '#canvas-' + x + '-' + y;
		$(loc).addClass('on');
	}
};

var drawState = false;
var lastX = null;
var lastY = null;
var waveChanged = false;
function toggleDrawState(s)
{
	drawState = s;
	if (drawState) {
		$('#draw_state_indicator').show();
	} else {
		$('#draw_state_indicator').hide();
		lastX = null;
		lastY = null;
	}
}

function saveWaveform()
{
	if (waveChanged) {
		toggleDrawState(0);
		updateCurrentWaveform(waveCanvas.samples);
		refreshMyWaveforms();
		waveChanged = false;
	}
}

function drawWaveform(x, y)
{
	if (drawState) {
		$('.sample-' + x).removeClass('on');
		$('#canvas-' + x + '-' + y).addClass('on');
		var sample = y * 256 - 32768;
		if (waveCanvas.samples[x] != sample) {waveChanged = true;}
		waveCanvas.samples[x] = sample;
		
		if (lastX !== null && x != lastX) {
			var cx = lastX;
			var cy = lastY;
			var incx = x > lastX ? 1 : -1;
			var incy = ((y - lastY) / (x - lastX)) * incx;
			while (cx != x)
			{
				cx += incx;
				cy += incy;
				$('.sample-' + cx).removeClass('on');
				$('#canvas-' + cx + '-' + parseInt(cy)).addClass('on');
				waveCanvas.samples[cx] = parseInt(cy * 256) - 32768;
			}
		}

		lastX = x;
		lastY = y;
	}
}