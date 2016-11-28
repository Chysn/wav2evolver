<?php
session_start();
require '../../private/IO.php';
$WAV_file = IO::file('wav');

// Basic errors
if (!is_array($WAV_file)) {
	print "<script>window.top.window.badFileError();</script>";
	exit;
}
if ($WAV_file['size'] > 60000) {
	print "<script>window.top.window.badFileError();</script>";
	exit;
}

// File metadata
$path = $WAV_file['tmp_name'];
$name = $WAV_file['name'];
$name = preg_replace('/\.wav$/i', '', $name);

// Try to convert the file into RAW
$cmd = "../../private/bin/wav2raw128 < {$path}";
$pcm = shell_exec($cmd);

// Could not be converted to RAW, so raise an error in the interface
if (strlen($pcm) != 256) {
	print "<script>window.top.window.badFileError();</script>";
	exit;
}

$names = IO::session('names');
$pcms = IO::session('waves');

// Initialize if first call
if (!is_array($names)) {
	$names = array();
	$pcms = array();
}

$names[] = $name;
$pcms[] = $pcm;

IO::setSession('names', $names);
IO::setSession('waves', $pcms);
IO::setSession('current_name', $name);
IO::setSession('current_wave', $pcm);
IO::setSession('current_shared', null);

print "<script>window.top.window.refreshMyWaveforms(); window.top.window.loadMyWaveform('NEW');</script>";
exit;