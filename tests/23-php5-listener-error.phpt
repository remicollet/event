--TEST--
Check for EventListener error behaviour in PHP5
--SKIPIF--
<?php
if (version_compare(PHP_VERSION, '7.0.0') >= 0) {
	die('skip target is PHP version < 7');
}
if (!class_exists("EventListener")) die("skip Event extra functions are disabled");
if (substr(PHP_OS, 0, 3) == "WIN") die('skip Not for Windows');
?>
--FILE--
<?php
$l = new EventListener(new EventBase(), function() {}, NULL, EventUtil::AF_UNIX, 0, 'unix:/tmp/1.sock');
?>
--EXPECTF--
Fatal error: EventListener::__construct(): EventBase must be passed by reference in %s on line %d
