<?php
class MyUnixSocketClient {
	private $base, $bev;

	function __construct($base, $sock_path) {
		$this->base = $base;
		$this->bev = new EventBufferEvent($base, NULL, EventBufferEvent::OPT_CLOSE_ON_FREE,
			array ($this, "read_cb"), NULL, array ($this, "event_cb"));

		if (!$this->bev->connectUnix($sock_path)) {
			trigger_error("Failed to connect to socket `$sock_path'", E_USER_ERROR);
		}

		$this->bev->enable(Event::READ);
	}

	function __destruct() {
		if ($this->bev) {
			$this->bev->free();
			$this->bev = NULL;
		}
	}

	function dispatch() {
		$this->base->dispatch();
	}

	function read_cb($bev, $unused) {
		$in = $bev->input;

		printf("Received %ld bytes\n", $in->length);
    	printf("----- data ----\n");
    	printf("%ld:\t%s\n", (int) $in->length, $in->pullup(-1));

		$this->bev->free();
		$this->bev = NULL;
		$this->base->exit(NULL);
	}

	function event_cb($bev, $events, $unused) {
		if ($events & EventBufferEvent::ERROR) {
			echo "Error from bufferevent\n";
		}

		if ($events & (EventBufferEvent::EOF | EventBufferEvent::ERROR)) {
			$bev->free();
			$bev = NULL;
		} elseif ($events & EventBufferEvent::CONNECTED) {
			$bev->output->add("test\n");
		}
	}
}

if ($argc <= 1) {
	exit("Socket path is not provided\n");
}
$sock_path = $argv[1];

$base = new EventBase();
$cl = new MyUnixSocketClient($base, $sock_path);
$cl->dispatch();
?>
