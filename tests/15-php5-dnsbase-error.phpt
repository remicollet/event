--TEST--
Check for EventDnsBase::__construct() error behavior
--SKIPIF--
<?php
if (!class_exists("EventDnsBase")) {
	die("skip Event extra functions are disabled");
}
if (version_compare(PHP_VERSION, '7.0.0') >= 0) {
	die('skip target is PHP version < 7');
}
?>
--FILE--
<?php
$e = new EventDnsBase(new EventBase(), TRUE);
?>
--EXPECTF--

Fatal error: EventDnsBase::__construct(): EventBase must be passed by reference in %s on line %d
