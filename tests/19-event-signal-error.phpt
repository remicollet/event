--TEST--
Check for Event::signal() error behavior in PHP5
--SKIPIF--
<?php
if (!extension_loaded('pcntl')) {
	die('SKIP pcntl extension required');
}
if (version_compare(PHP_VERSION, '7.0.0') >= 0) {
	die('skip target is PHP version < 7');
}
?>
--FILE--
<?php
$e = Event::signal(new EventBase(), SIGTERM, function() {});
?>
--EXPECTF--

Fatal error: Event::signal(): EventBase must be passed by reference in %s on line %d
