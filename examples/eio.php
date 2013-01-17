<?php
/*
 * Dispatch eio_*() via event
 */

/* Callback for eio_nop() */
function my_nop_cb($d, $r) {
	echo "step 6\n";
}

$dir = "/tmp/abc-eio-temp";
if (file_exists($dir)) {
	rmdir($dir);
}

echo "step 1\n";

$base = event_base_new();

echo "step 2\n";

eio_init();

eio_mkdir($dir, 0750, EIO_PRI_DEFAULT, "my_nop_cb");

$event = event_new($base, eio_get_event_stream(),
	EVENT_READ | EVENT_PERSIST, function ($fd, $events, $base) {
	echo "step 5\n";

	while (eio_nreqs()) {
		eio_poll();
	}

	event_base_loopbreak($base);
}, $base);

echo "step 3\n";

event_add($event);

echo "step 4\n";

event_base_dispatch($base);

echo "Done\n";
