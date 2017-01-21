<?php

require 'config.php';

class IO
{
	static $db;
	
	public static function session($key)
	{
		return isset($_SESSION[$key]) ? $_SESSION[$key] : '';
	}
	
	public static function setSession($key, $value)
	{
		$_SESSION[$key] = $value;
	}
	
	public static function post($key)
	{
		return isset($_POST[$key]) ? $_POST[$key] : '';
	}
	
	public static function get($key)
	{
		return isset($_GET[$key]) ? $_GET[$key] : '';
	}
	
	public static function getInt($key)
	{
		return preg_replace('/[^0-9]/', '', IO::get($key));
	}
	
	public static function file($key)
	{
		return isset($_FILES[$key]) ? $_FILES[$key] : null;
	}
	
	public static function server($key)
	{
		return isset($_SERVER[$key]) ? $_SERVER[$key] : null;
	}
	
	/* MySQL functions */
	public static function connect()
	{
		if (IO::$db) {
			return IO::$db;
		} else {
			$db = mysqli_connect(CONFIG_HOSTNAME, CONFIG_USERNAME, CONFIG_PASSWORD, CONFIG_DATABASE);
			if (!$db) {
			    throw new Exception("Cannot connect to {$db_name}");
		    }
			return $db;
		}
	}	
	
	public static function query($sql)
	{
		$db = IO::connect();
		$res = $db->query($sql);
		if (!$res) {
			throw new Exception(mysqli_error($db));
		}
		return $res;
	}
	
	public static function getValue($sql)
	{
		$db = IO::connect();
		$res = $db->query($sql);
		if ($res) {
			$row = $res->fetch_array(MYSQLI_NUM);
			if (isset($row[0])) {return $row[0];}
		}
		return null;
	}
	
	public static function getNextRow($res)
	{
		return $res->fetch_assoc();
	}
	
	public static function getFirstRow($sql)
	{
		$res = IO::query($sql);
		return IO::getNextRow($res);
	}
	
	public static function sanitize(&$value)
	{
		$value = addslashes($value);
		return $value;
	}
}