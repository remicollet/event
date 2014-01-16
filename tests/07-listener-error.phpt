--TEST--
Check for EventListener error behaviour
--SKIPIF--
<?php
if (!class_exists("EventListener")) die("skip Event extra functions are disabled");
if (substr(PHP_OS, 0, 3) == "WIN") die('skip Not for Windows');
?>
--FILE--
<?php
$base = new EventBase();

$sock_paths = array (
	"unix:/tmp/".mt_rand().".sock" => TRUE,
	"UNIX:/tmp/".mt_rand().".sock" => TRUE,
	":/tmp/".mt_rand().".sock"     => FALSE,
	"/tmp/".mt_rand().".sock"      => FALSE,
);

foreach ($sock_paths as $path => $expect) {
	$listener = @new EventListener($base, function() {}, NULL, 0, -1, $path);
	if (file_exists($path)) unlink($path);

	var_dump(is_null($listener) != $expect);
}

$l = new EventListener(new EventBase(), function() {}, NULL, EventUtil::AF_UNIX, 0, 'unix:/tmp/1.sock');
?>
--EXPECTF--
bool(true)
bool(true)
bool(true)
bool(true)

Fatal error: EventListener::__construct(): EventBase must be passed by reference in %s on line %d
