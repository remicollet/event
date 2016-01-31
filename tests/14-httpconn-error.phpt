--TEST--
Check for EventHttpConnection::__construct() error behavior
--SKIPIF--
<?php if (!class_exists("EventHttpConnection")) print "skip Event extra functions are disabled"; ?>
--FILE--
<?php
$e = new EventHttpConnection(new EventBase(), NULL, '10.10.0.1', 9899);
?>
--EXPECTF--

Fatal error: EventHttpConnection::__construct(): EventBase must be passed by reference in %s on line %d
