--TEST--
Check for event_buffer sockets 
--FILE--
<?php 
$base = event_base_new();
$bev = bufferevent_socket_new($base, NULL, EVENT_BEV_OPT_CLOSE_ON_FREE);

if (! bufferevent_socket_connect($bev, "www.php.net:80", TRUE)) {
	exit("event_buffer_socket_connect failed\n");
}

bufferevent_setcb($bev, NULL, NULL, function ($bev, $events, $data) {
	if ($events & EVENT_BEV_EVENT_CONNECTED) {
		/* We're connected to 127.0.0.1:8080.   Ordinarily we'd do
		something here, like start reading or writing. */
		echo "Connected\n";
	} elseif ($events & EVENT_BEV_EVENT_ERROR) {
		exit("EVENT_BEV_EVENT_ERROR error occured\n");
 	}	
}, "data");

event_base_dispatch($base);
?>
--EXPECT--
Connected

