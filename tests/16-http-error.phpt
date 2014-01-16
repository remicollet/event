--TEST--
Check for EventHttp::__construct() error behavior
--FILE--
<?php
$e = new EventHttp(new EventBase());
?>
--EXPECTF--

Fatal error: EventHttp::__construct(): EventBase must be passed by reference in %s on line %d
