--TEST--
Check for EventUtil::createSocket
--SKIPIF--
<?php
if (!extension_loaded('sockets')) {
	die("skip sockets extension is not available");
}
if (!method_exists('EventUtil', 'createSocket')) {
	die('skip Event is built without sockets support');
}
?>
--FILE--
<?php
if (!$sock = socket_create(AF_INET, SOCK_STREAM, SOL_TCP)) {
	exit('socket_create failed');
}

$ip = '127.0.0.2';
if (!socket_bind($sock, $ip)) {
	exit('socket_bind failed');
}

if (($fd = EventUtil::getSocketFD($sock)) <= 0) {
	exit('EventUtil::getSocketFD failed');
}

$sock2 = EventUtil::createSocket($fd);

var_dump($sock2);
var_dump($fd);
var_dump(EventUtil::getSocketFD($sock2) === $fd);
?>
--EXPECTF--
resource(%d) of type (Socket)
int(%d)
bool(true)
