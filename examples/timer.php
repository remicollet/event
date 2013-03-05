<?php
$base = new EventBase();
$e = new Event($base, -1, Event::TIMEOUT, function($fd, $what, $e) {
	echo "2 seconds elapsed\n";
	$e->delTimer();
});
$e->data = $e;
$e->addTimer(2);
$base->loop();
?>
