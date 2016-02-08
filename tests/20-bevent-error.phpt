--TEST--
Check for EventBufferEvent::__construct() error behavior in PHP5
--SKIPIF--
<?php
if (version_compare(PHP_VERSION, '7.0.0') >= 0) {
	die('skip PHP version < 7');
}
?>
--FILE--
<?php
$b = new EventBufferEvent(new EventBase());
?>
--EXPECTF--

Fatal error: EventBufferEvent::__construct(): EventBase must be passed by reference in %s on line %d
