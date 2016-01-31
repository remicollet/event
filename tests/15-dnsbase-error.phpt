--TEST--
Check for EventDnsBase::__construct() error behavior
--SKIPIF--
<?php if (!class_exists("EventDnsBase")) print "skip Event extra functions are disabled"; ?>
--FILE--
<?php
$e = new EventDnsBase(new EventBase(), TRUE);
?>
--EXPECTF--

Fatal error: EventDnsBase::__construct(): EventBase must be passed by reference in %s on line %d
