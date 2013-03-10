--TEST--
Check for timer event basic behaviour 
--FILE--
<?php
$base = new EventBase();
$e = new Event($base, -1, Event::TIMEOUT, function($fd, $what, $e) {
	echo "0.4 seconds elapsed";
	//$e->delTimer();
});
$e->data = $e;
$e->addTimer(0.4);
$base->loop();
?>
--EXPECT--
0.4 seconds elapsed
