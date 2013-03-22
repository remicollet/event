--TEST--
Check for EventListener error behaviour 
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

?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
