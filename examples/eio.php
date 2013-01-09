 <?php
function my_eio_poll($fd, $events, $arg) {
	echo __FUNCTION__, PHP_EOL;
    if (eio_nreqs()) {
        eio_poll();
    }
}

function my_nop_cb($d, $r) {
	echo __FUNCTION__, PHP_EOL;
    var_dump($r, $d);
}

$dir = "/tmp/abc-eio-temp";
if (file_exists($dir)) {
	echo "removing $dir\n";
	rmdir($dir);
}

echo "step 1\n";
$base = event_base_new();

$fd = eio_get_event_stream();
echo "eio fd:\n";
var_dump($fd);

echo "step 2\n";
eio_mkdir($dir, 0750, EIO_PRI_DEFAULT, "my_nop_cb");

$event = event_new($base, $fd, EVENT_READ /*| EVENT_PERSIST*/, function ($fd, $events, $arg) {
	echo "event_new cb()\nfd = $fd, events = $events\n";
	if (eio_nreqs()) {
		eio_poll();
	}
});

echo "step 3\n";
event_add($event);

echo "step 4\n";
event_base_loop($base);
echo "Done\n";
?> 
