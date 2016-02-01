--TEST--
Check for Event::timer() error behavior in PHP5
--SKIPIF--
<?php
if (version_compare(PHP_VERSION, '7.0.0') >= 0) {
	die('skip target is PHP version < 7');
}
?>
--FILE--
<?php
$e = Event::timer(new EventBase(), function() {});
?>
--EXPECTF--

Fatal error: Event::timer(): EventBase must be passed by reference in %s on line %d
