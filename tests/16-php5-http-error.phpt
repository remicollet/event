--TEST--
Check for EventHttp::__construct() error behavior
--SKIPIF--
<?php
if (!class_exists("EventHttp")) {
	die("skip Event extra functions are disabled");
}
if (version_compare(PHP_VERSION, '7.0.0') >= 0) {
	die('skip target is PHP version < 7');
}
?>
--FILE--
<?php
$e = new EventHttp(new EventBase());
?>
--EXPECTF--

Fatal error: EventHttp::__construct(): EventBase must be passed by reference in %s on line %d
