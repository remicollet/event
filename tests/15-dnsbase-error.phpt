--TEST--
Check for EventDnsBase::__construct() error behavior
--FILE--
<?php
$e = new EventDnsBase(new EventBase(), TRUE);
?>
--EXPECTF--

Fatal error: EventDnsBase::__construct(): EventBase must be passed by reference in %s on line %d
