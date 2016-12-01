<?php
session_start();

ini_set('display_errors', 1);
error_reporting(E_ALL);

require '../../private/IO.php';
require '../../private/custom_json.php';

$job = IO::get('job');

switch ($job)
{
	case 'shared_list':
		$data = getSharedList();
		break;
	
	case 'load_shared':
		$id = IO::getInt('id');
		$data = getShared($id);
		break;
		
	case 'info':
		$id = IO::getInt('id');
		$data = getSharedInfo($id);
		break;

	case 'share_wave':
		$data = shareWave();
		break;	
		
	case 'session_list':
		$data = getSessionList();
		break;
		
	case 'load_session':
		$id = IO::get('id');
		$data = getSession($id);
		break;
		
	case 'delete_session':
		$id = IO::getInt('id');
		deleteSession($id);
		break;
		
	case 'update_wave':
		$samples = IO::post('samples');
		updateWave($samples);
		break;
		
	// Effects
	case 'maximize':
		$data = effect_Maximize();
		break;
}

print custom_json::encode($data);
//print json_encode($data);
exit;

function getSharedList()
{
	$data = array();
	
	$sql = "SELECT * FROM `waveform`
			WHERE `shared` = 1
			ORDER BY `share_time` DESC";
	$res = IO::query($sql);
	if ($res) {
		while ($row = $res->fetch_assoc())
		{
			$data[] = array('id' => $row['id'], 'name' => $row['name']);
		}
	}
	return $data;
}

function getShared($id)
{
	$data = array();
	
	$sql = "SELECT * FROM `waveform`
			WHERE `shared` = 1
			    AND `id` = {$id}";
	$res = IO::query($sql);
	$row = $res->fetch_assoc();
	if ($row['id']) {
		$data = array('samples' => convertStringToNumericArray($row['pcm']),
				      'name' => $row['name'],
				     );
		IO::setSession('current_name', $row['name']);
		IO::setSession('current_wave', $row['pcm']);
		IO::setSession('current_shared', $row['id']);
	}
	
	return $data;
}

function getSharedInfo($id)
{
	$data = array();
	
	$sql = "SELECT * FROM `waveform`
			WHERE `shared` = 1
			    AND `id` = {$id}";
	$res = IO::query($sql);
	$data = $res->fetch_assoc();

	if ($data['id']) {
		$data['samples'] = convertStringToNumericArray($data['pcm']);
		unset($data['pcm']); // Don't want to send this via JSON because it can be weird stuff
		$data['share_time'] = date('d M Y', strtotime($data['share_time']));
		return $data;
	}
}

function shareWave()
{
	$name = IO::get('name');
	$description = IO::get('desc');
	$signature = IO::get('sig');
	$pcm = IO::session('current_wave');
	
	IO::sanitize($name);
	IO::sanitize($description);
	IO::sanitize($pcm);
	IO::sanitize($signature);
	
	$name = strip_tags($name);
	$description = strip_tags($description);
	$signature = strip_tags($signature);
	
	// Verify that the exact wave hasn't already been shared
	$sql = "SELECT `share_time` FROM `waveform` 
			WHERE `pcm` = '{$pcm}'";
	$share_time = IO::getValue($sql);
	if ($share_time) {
		return (array('status' => 'Duplicate'));
	}
	
	$sql = "INSERT INTO `waveform` (`share_time`, `name`, `description`, `signature`, `pcm`, `shared`)
			VALUES (NOW(), '{$name}', '{$description}', '{$signature}', '{$pcm}', 1)";
	IO::query($sql);
	return array('status' => 'OK');
}

function getSessionList()
{
	$data = array();
	$names = IO::session('names');
	$waves = IO::session('waves');
	if (!isset($names)) {return array();}

	for ($id = 0; $id < sizeof($names); $id++)
	{
		$data[] = array('name' => $names[$id], 'id' => $id);
	}
	return $data;
}

function deleteSession($id)
{
	$names = IO::session('names');
	$names[$id] = null;
	IO::setSession('names', $names);
}

function getSession($id)
{
	$waves = IO::session('waves');
	if (!is_array($waves)) {return array();}
	if ($id == 'NEW') {
		$id = sizeof($waves) - 1;
	}
	$id = intval($id);
	
	$names = IO::session('names');
	IO::setSession('current_name', $names[$id]);
	IO::setSession('current_wave', $waves[$id]);
	IO::setSession('current_shared', null);
	
	$data = array('samples' => convertStringToNumericArray($waves[$id]),
				  'name' => $names[$id],
				 );
	return $data;
}

function convertStringToNumericArray($string)
{	
	$data = array();
	for ($i = 0; $i < 128; $i++)
	{
		$sample = ord(substr($string, (2 * $i) + 1, 1)) * 256 + ord(substr($string, (2 * $i), 1));
		if ($sample & 0x8000) {
			$sample = - (0x010000 - $sample);
		}
		$data[$i] = $sample;
	}
	return $data;
}

function convertNumericArrayToString($samples)
{
	$string = '';
	for ($i = 0; $i < 128; $i++)
	{
		$sample = $samples[$i];
		$high = ($sample & 0xff00) >> 8;
		$low = $sample & 0x00ff;
		$string .= chr($low) . chr($high);
	}
	return $string;
}

function updateWave($input)
{	
	$samples = explode(',', $input);
	$pcm = convertNumericArrayToString($samples);
	
	$name = getEditedName();
	addWaveToMyWaves($name, $pcm);			     
}

function effect_Maximize()
{
	$pcm = IO::session('current_wave');
	$hex = '';
	for ($i = 0; $i < strlen($pcm); $i++)
	{
		$hex .= '\x' . dechex(ord(substr($pcm, $i, 1)));
	}
	$cmd = "echo -n -e '{$hex}' | ../../private/bin/raw128maximize 2>&1";
	$pcm = shell_exec($cmd);

	$name = getEditedName();
	
	$data = array('samples' => convertStringToNumericArray($pcm),
			      'name' => $name,
			     );

	addWaveToMyWaves($name, $pcm);			     
	return $data;	
}

function addWaveToMyWaves($name, $pcm)
{
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
}

function getEditedName()
{
	$name = IO::session('current_name');
	if ($name) {
		$name = preg_replace('/ \d+:\d\d\.\d\d$/', '', $name); // Strip off previous time
		$name .= ' ' . date('h:i.s');
	} else {
		$name = 'New Waveform ' . date('h:i.s');
	}
	return $name;
}
