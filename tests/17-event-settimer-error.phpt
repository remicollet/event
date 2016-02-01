--TEST--
Check for Event::set() error behavior in PHP 5
--SKIPIF--
<?php
if (version_compare(PHP_VERSION, '7.0.0') >= 0) {
	die('skip target is PHP version < 7');
}
?>
--FILE--
<?php
$b = new EventBase();
$e = new Event($b, 0, Event::READ, function() {});
$e->set(new EventBase(), 1);
?>
--EXPECTF--

Fatal error: Event::set(): EventBase must be passed by reference in %s on line %d
