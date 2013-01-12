<?php
/*
 * 1. Connect to 127.0.0.1 at port 80
 * by means of bufferevent_socket_connect().
 *
 * 2. Request /index.cphp via HTTP/1.0
 * using the output buffer.
 *
 * 3. Asyncronously read the response and print it to stdout.
 */

/* Read callback */
function readcb($bev, $base) {
	$input = bufferevent_get_input($bev);

	while (($n = evbuffer_remove($input, $buf, 1024)) > 0) {
		echo $buf;
	}
}

/* Event callback */
function eventcb($bev, $events, $base) {
	if ($events & EVENT_BEV_EVENT_CONNECTED) {
		echo "Connected.\n";
	} elseif ($events & (EVENT_BEV_EVENT_ERROR | EVENT_BEV_EVENT_EOF)) {
		if ($events & EVENT_BEV_EVENT_ERROR) {
			echo "DNS error: ", bufferevent_socket_get_dns_error($bev), PHP_EOL;
		}

		echo "Closing\n";
		bufferevent_free($bev);
		event_base_loopexit($base);
		exit("Done\n");
	}
}

$base = event_base_new();

$bev = bufferevent_socket_new($base, /* use internal socket */ NULL,
	EVENT_BEV_OPT_CLOSE_ON_FREE | EVENT_BEV_OPT_DEFER_CALLBACKS);
if (!$bev) {
	exit("Failed creating bufferevent socket\n");
}

bufferevent_setcb($bev, "readcb", /* writecb */ NULL, "eventcb", $base);
bufferevent_enable($bev, EVENT_READ | EVENT_WRITE);

/* Send request */
$output = bufferevent_get_output($bev);
if (!evbuffer_add($output,
	"GET /index.cphp HTTP/1.0\r\n".
	"Connection: Close\r\n\r\n"
)) {
	exit("Failed adding request to output buffer\n");
}

/* Connect to the host syncronously.
 * We know the IP, and don't need to resolve DNS. */
if (!bufferevent_socket_connect($bev, "127.0.0.1:80")) {
	exit("Can't connect to host\n");
}

/* Dispatch pending events */
event_base_dispatch($base);
