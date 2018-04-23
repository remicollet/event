--TEST--
Check for issue #31
--FILE--
<?php
$eventBaseClass = EVENT_NS . '\\EventBase';
$eventClass = EVENT_NS . '\\Event';

$b = new $eventBaseClass();
$e = new $eventClass($b, 1, $eventClass::READ | $eventClass::WRITE, function ($fd, $what, $e) {
	var_dump($fd);
});
$e->add();
$b->loop();

$e->set($b, 0, $eventClass::READ | $eventClass::WRITE, function ($fd, $what, $e) {
	var_dump($fd);
});
$e->add();
$b->loop();

?>
--EXPECT--
int(1)
int(0)
