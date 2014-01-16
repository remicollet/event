--TEST--
Check for Event::signal() error behavior
--SKIPIF--
<?php
if (!extension_loaded('pcntl')) die('SKIP pcntl extension required');
?>
--FILE--
<?php
$e = Event::signal(new EventBase(), SIGTERM, function() {});
?>
--EXPECTF--

Fatal error: Event::signal(): EventBase must be passed by reference in %s on line %d
