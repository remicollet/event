--TEST--
Check for event_buffer sockets 
--FILE--
<?php 
$base = new EventBase();
$bev = new EventBufferEvent($base, NULL, EventBufferEvent::OPT_CLOSE_ON_FREE);

if (!$bev->connect("www.php.net:80", TRUE)) {
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

