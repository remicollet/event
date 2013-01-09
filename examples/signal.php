<?php
/*
Launch it in a terminal window:

$ php examples/signal.php

In another terminal window find out the pid and send SIGTERM, e.g.:

$ ps aux | grep examp
ruslan    3976  0.2  0.0 139896 11256 pts/1    S+   10:25   0:00 php examples/signal.php
ruslan    3978  0.0  0.0   9572   864 pts/2    S+   10:26   0:00 grep --color=auto examp
$ kill -TERM 3976

At the first terminal window you should catch the following:

Caught signal 15
*/
class MyEventSignal {
	private $base;

	function __construct($base) {
		$this->base = $base;
	}

	function eventSighandler($no, $events, $c) {
		if ($events & EVENT_SIGNAL) {
			echo "Caught signal $no\n"; 
        	event_base_loopexit($c->base);
		} else {
			echo "Unknown error. Stopping\n";
        	event_base_loopexit($c->base);
		}
	}
}

$base = event_base_new();
$c    = new MyEventSignal($base);
$no   = SIGTERM;
$ev   = event_new($base, $no, EVENT_SIGNAL | EVENT_PERSIST, array($c,'eventSighandler'), $c);

event_add($ev);

event_base_loop($base);
?>


