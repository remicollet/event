--TEST--
Check for event_buffer sockets 
--SKIPIF--
<?php
    if (!extension_loaded('event')) {
        die('skip - event extension not available.');
    }
    if (getenv("SKIP_ONLINE_TESTS")) {
        die("skip test requiring internet connection");
    }
?>
--FILE--
<?php 
$base = new EventBase();
$bev = new EventBufferEvent($base, NULL, EventBufferEvent::OPT_CLOSE_ON_FREE);

if (!$bev->connectHost(NULL, "www.php.net", 80, EventUtil::AF_INET)) {
	exit("Connection failed\n");
}

$bev->setCallbacks(NULL, NULL, function ($bev, $events, $data) {
	if ($events & EventBufferEvent::CONNECTED) {
		/* We're connected to 127.0.0.1:8080.   Ordinarily we'd do
		something here, like start reading or writing. */
		echo "Connected\n";
	} elseif ($events & EventBufferEvent::ERROR) {
		exit("EVENT_BEV_EVENT_ERROR error occured\n");
 	}	
}, "data");

$base->dispatch();

$bev->free();
?>
--EXPECT--
Connected

