--TEST--
Check for timer event basic behaviour
--FILE--
<?php
$base = new EventBase();
$e = new Event($base, -1, Event::TIMEOUT, function($fd, $what, $e) {
	echo "0.4 seconds elapsed";
	// By calling free() we prevent segmentation fault with
	// MALLOC_PERTURB_=$(($RANDOM % 255 + 1)), since otherwise the refcount for
	// Event will be bigger than the refcount for EventBase, and EventBase is destroyed earlier.
	$e->free();
});
$e->data = $e;
$e->addTimer(0.4);
$base->loop();
?>
--EXPECT--
0.4 seconds elapsed
