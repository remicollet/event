<?php
function readcb($bev, $base) {
	$input = bufferevent_get_input($bev);

	while (($n = evbuffer_remove($input, $buf, 1024)) > 0) {
		echo $buf;
	}
}

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
		exit("exit\n");
	}
}

if ($argc != 3) {
	echo <<<EOS
Trivial HTTP 0.x client
Syntax: php {$argv[0]} [hostname] [resource]
Example: php {$argv[0]} www.google.com /
EOS;
	exit();
}

$base = event_base_new();
$dns_base = evdns_base_new($base, TRUE);
echo "rsrc type of \$dns_base= ", get_resource_type($dns_base), PHP_EOL;
if (!$dns_base) {
	exit("Failed to init DNS Base\n");
}

$bev = bufferevent_socket_new($base, NULL, EVENT_BEV_OPT_CLOSE_ON_FREE /*| EVENT_BEV_OPT_DEFER_CALLBACKS*/);

bufferevent_setcb($bev, "readcb", NULL, "eventcb", $base);
bufferevent_enable($bev, EVENT_READ | EVENT_WRITE);

$output = bufferevent_get_output($bev);

if (!evbuffer_add($output,
	"GET {$argv[2]} HTTP/1.1\r\n".
	"Host: {$argv[1]}\r\n".
	"Connection: Close\r\n\r\n"
)) {
	exit("Failed adding request to output buffer\n");
}

if (!bufferevent_socket_connect_hostname($bev, $dns_base, $argv[1], 80, EVENT_AF_UNSPEC)) {
	exit("Can't connect to host {$argv[1]}\n");
}

event_base_dispatch($base);
