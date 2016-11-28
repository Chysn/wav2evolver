function WaveThumbnail(samples)
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
}

WaveThumbnail.prototype.drawInto = function(target_id) {
	target = document.getElementById(target_id);
	var topOffset = $(target).offset().top - 75;
	var leftOffset = $(target).offset().left + 15;
	for (var x = 0; x < this.samples.length; x++)
	{
		var s = this.samples[x];
		// Sample is a 16-bit sample, so first make it positive or zero, and then divide by
		// 1024 because the canvas is a 6-bit space
		var y = parseInt(((this.samples[x] + 32768) / 1024) + .5);
		
		// And, since the zero is on top, flip the value
		y = (64 - y);
		
		var py = parseInt(y) + parseInt(topOffset);
		var px = parseInt(x) + parseInt(leftOffset);
		
		var el = document.createElement('DIV');
		el.innerHTML = '&dot;';
		el.style.position = 'absolute';
		el.style.top = py + 'px';
		el.style.left = px + 'px';
		el.style.zIndex = '1000';
		el.className = 'thumb_dot';
		target.appendChild(el);
	}
};