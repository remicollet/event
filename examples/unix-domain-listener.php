<?php
/*
 * Simple echo server based on libevent's connection listener.
 *
 * Usage:
 * 1) In one terminal window run:
 *
 * $ php unix-domain-listener.php [path-to-socket]
 *
 * 2) In another terminal window open up connection
 * to the socket, e.g.:
 *
 * $ socat - GOPEN:/tmp/1.sock
 *
 * 3) Start typing. The server should repeat the input.
 */

class MyListenerConnection {
	private $bev, $base;

	public function __destruct() {
		if ($this->bev) {
			$this->bev->free();
		}
	}

	public function __construct($base, $fd) {
		$this->base = $base;

		$this->bev = new EventBufferEvent($base, $fd, EventBufferEvent::OPT_CLOSE_ON_FREE);

		$this->bev->setCallbacks(array($this, "echoReadCallback"), NULL,
			array($this, "echoEventCallback"), NULL);

		if (!$this->bev->enable(Event::READ)) {
			echo "Failed to enable READ\n";
			return;
		}
	}

	public function echoReadCallback($bev, $ctx) {
		// Copy all the data from the input buffer to the output buffer
		$bev->output->addBuffer($bev->input);
	}

	public function echoEventCallback($bev, $events, $ctx) {
		if ($events & EventBufferEvent::ERROR) {
			echo "Error from bufferevent\n";
		}

		if ($events & (EventBufferEvent::EOF | EventBufferEvent::ERROR)) {
			$bev->free();
			$bev = NULL;
		}
	}
}

class MyListener {
	public $base,
		$listener,
		$socket;
	private $conn = array();

	public function __destruct() {
		foreach ($this->conn as &$c) $c = NULL;
	}

	public function __construct($sock_path) {
		$this->base = new EventBase();
		if (!$this->base) {
			echo "Couldn't open event base";
			exit(1);
		}

		if (file_exists($sock_path)) {
			unlink($sock_path);
		}

 		$this->listener = new EventListener($this->base,
 			array($this, "acceptConnCallback"), $this->base,
 			EventListener::OPT_CLOSE_ON_FREE | EventListener::OPT_REUSEABLE, -1,
 			"unix:$sock_path");

		if (!$this->listener) {
			trigger_error("Couldn't create listener", E_USER_ERROR);
		}

		$this->listener->setErrorCallback(array($this, "accept_error_cb"));
	}

	public function dispatch() {
		$this->base->dispatch();
	}

	// This callback is invoked when there is data to read on $bev
	public function acceptConnCallback($listener, $fd, $address, $ctx) {
		// We got a new connection! Set up a bufferevent for it. */
		$base = $this->base;
		$this->conn[] = new MyListenerConnection($base, $fd);
	}

	public function accept_error_cb($listener, $ctx) {
		$base = $this->base;

		fprintf(STDERR, "Got an error %d (%s) on the listener. "
			."Shutting down.\n",
			EventUtil::getLastSocketErrno(),
			EventUtil::getLastSocketError());

		$base->exit(NULL);
	}
}

if ($argc <= 1) {
	exit("Socket path is not provided\n");
}
$sock_path = $argv[1];

$l = new MyListener($sock_path);
$l->dispatch();
