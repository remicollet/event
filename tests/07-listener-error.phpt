--TEST--
Check for EventListener error behaviour 
--FILE--
<?php
$base = new EventBase();

// Create listener based on UNIX domain socket. Pass wrong address family.
// The contructor should return NULL
$listener = new EventListener($base, function() {}, NULL, 0, -1,
	"/tmp/".mt_rand().".sock", EventUtil::AF_UNSPEC);

var_dump($listener);
?>
--EXPECT--
NULL
