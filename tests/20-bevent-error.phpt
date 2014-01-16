--TEST--
Check for EventBufferEvent::__construct() error behavior
--FILE--
<?php
$b = new EventBufferEvent(new EventBase());
?>
--EXPECTF--

Fatal error: EventBufferEvent::__construct(): EventBase must be passed by reference in %s on line %d
