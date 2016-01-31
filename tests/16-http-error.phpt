--TEST--
Check for EventHttp::__construct() error behavior
--SKIPIF--
<?php if (!class_exists("EventHttp")) print "skip Event extra functions are disabled"; ?>
--FILE--
<?php
$e = new EventHttp(new EventBase());
?>
--EXPECTF--

Fatal error: EventHttp::__construct(): EventBase must be passed by reference in %s on line %d
