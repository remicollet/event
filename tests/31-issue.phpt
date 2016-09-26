--TEST--
Check for issue #31
--FILE--
<?php
$b = new EventBase();
$e = new Event($b, 1, Event::READ | Event::WRITE, function ($fd, $what, $e) {
	var_dump($fd);
});
$e->add();
$b->loop();

$e->set($b, 0, Event::READ | Event::WRITE, function ($fd, $what, $e) {
	var_dump($fd);
});
$e->add();
$b->loop();

?>
--EXPECT--
int(1)
int(0)
