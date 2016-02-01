--TEST--
Check for EventBufferEvent::createPair() error behavior in PHP5
--SKIPIF--
<?php
if (version_compare(PHP_VERSION, '7.0.0') >= 0) {
	die('skip target is PHP version < 7');
}
?>
--FILE--
<?php
EventBufferEvent::createPair(new EventBase());
?>
--EXPECTF--
Fatal error: EventBufferEvent::createPair(): EventBase must be passed by reference in %s on line %d
