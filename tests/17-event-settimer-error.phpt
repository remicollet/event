--TEST--
Check for Event::set() error behavior
--FILE--
<?php
$b = new EventBase();
$e = new Event($b, 0, Event::READ, function() {});
$e->set(new EventBase(), 1);
?>
--EXPECTF--

Fatal error: Event::set(): EventBase must be passed by reference in %s on line %d
