--TEST--
Check for Event::timer() error behavior
--FILE--
<?php
$e = Event::timer(new EventBase(), function() {});
?>
--EXPECTF--

Fatal error: Event::timer(): EventBase must be passed by reference in %s on line %d
