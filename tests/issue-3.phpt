--TEST--
Check for issue #3: SEGFAULT after calling EventHttpRequest::free()
--SKIPIF--
<?php
if (!class_exists("EventHttpRequest")) die("skip Event extra functions are disabled");
?>
--FILE--
<?php
function _callback_server($req) {
	$req->sendReply(200, "OK");
}

function _callback_client($req) {
	global $inc;
	echo $inc, PHP_EOL;
	//echo $inc++, ' -> ',  memory_get_usage(true), PHP_EOL;
	// Free request and get SegFault or... get memory grows
	$req->free();
}

function _callback_timer(){
	global $timer, $conn, $base, $inc;

	$req = new EventHttpRequest("_callback_client", $base);
	$conn->makeRequest($req, EventHttpRequest::CMD_GET, "/");

	if (++$inc < 30) {
		$timer->addTimer(0.01);
	}
}


$inc  = 0;
$host = "0.0.0.0";
$port = 8181;

$base = new EventBase();

$http = new EventHttp($base);
$http->setAllowedMethods(EventHttpRequest::CMD_GET);
$http->setDefaultCallback("_callback_server");
$http->bind($host, $port);

$timer = Event::timer($base,  '_callback_timer');
$timer->addTimer(1);

$conn = new EventHttpConnection($base, NULL, $host, $port);

$base->loop();
?>
--EXPECT--
1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
16
17
18
19
20
21
22
23
24
25
26
27
28
29
30

