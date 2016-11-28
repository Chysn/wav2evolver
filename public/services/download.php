<?php
session_start();
require '../../private/IO.php';

$type = IO::get('type');
$name = IO::session('current_name');
$wave = IO::session('current_wave');

// Format name for download
if (!$name) {$name = date('Ymd_His');}
$name = str_replace(' ', '_', $name);
$name = preg_replace('/\..+$/', '', $name);
$name = preg_replace('/[^A-Za-z0-9_]/', '', $name);
$name = preg_replace('/__+/', '_', $name);

$output = '';
$len = 0;
if ($type == 'raw') {
	$output = $wave;
	$len = 256;
	$name .= ".{$type}";
}

if ($type == 'wav') {
	// All of the waveforms generated from raw data by this application will share the
	// same header, because they have the same metadata of 16-bit signed PCM, single-channel,
	// 256 bytes.
	$generic_header  = "\x52\x49\x46\x46\x24\x01\x00\x00\x57\x41\x56\x45\x66\x6d\x74\x20";
	$generic_header .= "\x10\x00\x00\x00\x01\x00\x01\x00\x01\xb9\x00\x00\x02\x72\x01\x00";
	$generic_header .= "\x02\x00\x10\x00\x64\x61\x74\x61\x00\x01\x00\x00";
	$output = $generic_header . $wave;
	$len = 300;
	$name .= ".{$type}";
}

if ($type == 'syx') {
	$number = IO::getInt('number');
	if ($number > 128 or $number < 97) {$number = 128;}
	$name .= "-{$number}.syx";
	$hex = '';
	for ($i = 0; $i < 256; $i++)
	{
		$byte = substr($wave, $i, 1);
		$h = dechex(ord($byte));
		$hex .= '\x' . $h;
	}
	$cmd = "echo -en '{$hex}' | ../../private/bin/raw2evolver {$number}";
	$output = shell_exec($cmd);
	$len = 300;
}

if ($output and strlen($output) == $len) {
	header("Content-Length: {$len}");
    header("Pragma: ");
    header("Cache-Control: ");
    header("Content-type: application/octet-stream");
    header("Content-disposition: attachment; filename={$name}");
    print $output;
    
    // Log the download, if it's a shared wave
    if (IO::session('current_shared')) {
    	$waveform_id = IO::session('current_shared');
    	$request_ip = IO::server('REMOTE_ADDR');
	    $sql = "INSERT INTO `download` (`waveform_id`, `type`, `download_time`, `request_ip`)
	    		VALUES ({$waveform_id}, '{$type}', NOW(), '{$request_ip}')";
	    IO::query($sql);
    }	    
}