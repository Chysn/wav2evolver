<?php

session_start();
require '../../private/IO.php';

$current_wave = IO::session('current_wave');

// $rate is (Freq. in Hz * 128) / 256, and is a string converted to hex
// Examples: A440 = 'dc', A220 = '6e'
// The low pitch is half the rate of the high, so all the lengths are cut in half
if (IO::get('low')) {
	$rate = "\x6e";
} else {
	$rate = "\xdc";
}

// Header for a 65536 byte file
$wav  = "\x52\x49\x46\x46\x24\x01\x00\x00\x57\x41\x56\x45\x66\x6d\x74\x20";
$wav .= "\x10\x00\x00\x00\x01\x00\x01\x00\x00{$rate}\x00\x00\x02\x72\x01\x00";
$wav .= "\x02\x00\x10\x00\x64\x61\x74\x61\x00{$rate}\x00\x00";

// Append copies of the current wave so it can be heard
$copies = ord($rate);
for ($i = 0; $i <= $copies; $i++)
{
	$wav .= $current_wave;
}

$content_length = ($copies * 256) + 44;
header("Content-Length: {$content_length}");
header("Pragma: no-cache");
header("Cache-Control: ");
header("Content-type: audio/x-wav");

print $wav;
exit;