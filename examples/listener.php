<?php
/*
 * Simple echo server based on libevent's connection listener.
 *
 * Usage:
 * 1) In one terminal window run:
 *
 * $ php listener.php 9881
 *
 * 2) In another terminal window open up connection, e.g.:
 *
 * $ nc 127.0.0.1 9881
 *
 * 3) start typing. The server should repeat the input.
 */

class MyListener {
	public $base,
		$listener,
		$bev,
		$socket;

	public function __construct($port) {
		$this->base = new EventBase();
		if (!$this->base) {
			echo "Couldn't open event base";
			exit(1);
		}

		// Variant #1
		/*
		$this->socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
		if (!socket_bind($this->socket, '0.0.0.0', $port)) {
			echo "Unable to bind socket\n";
			exit(1);
		}
		$this->listener = new EventListener($this->base,
			array($this, "accept_conn_cb"), $this->base,
			EventListener::OPT_CLOSE_ON_FREE | EventListener::OPT_REUSEABLE,
			-1, $this->socket);
		 */

//		Variant #2
 		$this->listener = new EventListener($this->base,
 			array($this, "accept_conn_cb"), $this->base,
 			EventListener::OPT_CLOSE_ON_FREE | EventListener::OPT_REUSEABLE, -1,
 			"0.0.0.0:$port");

		if (!$this->listener) {
			echo "Couldn't create listener";
			exit(1);
		}

		$this->listener->setErrorCallback(array($this, "accept_error_cb"));
	}

	public function dispatch() {
		$this->base->dispatch();
	}

	// This callback is invoked when there is data to read on $bev
	public function echo_read_cb($bev, $ctx) {
		// Copy all the data from the input buffer to the output buffer
		
		// Variant #1
		$bev->output->addBuffer($bev->input);

		/* Variant #2 */
		/*
		$input	= $bev->getInput();
		$output = $bev->getOutput();
		$output->addBuffer($input);
		*/
	}

	public function echo_event_cb($bev, $events, $ctx) {
		if ($events & EventBufferEvent::ERROR)
			echo "Error from bufferevent\n";

		if ($events & (EventBufferEvent::EOF | EventBufferEvent::ERROR)) {
			$bev->free();
		}
	}

	public function accept_conn_cb($listener, $fd, $address, $ctx) {
		/* We got a new connection! Set up a bufferevent for it. */
		//$base = $ctx;
		//$base = $listener->getBase();
		$base = $this->base;

		$this->bev = new EventBufferEvent($base, $fd, EventBufferEvent::OPT_CLOSE_ON_FREE);

		$this->bev->setCallbacks(array($this, "echo_read_cb"), NULL, array($this, "echo_event_cb"), NULL);

		if (!$this->bev->enable(Event::READ | Event::WRITE)) {
			echo "Failed to enable READ | WRITE\n";
			exit();
		}
	}

	public function accept_error_cb($listener, $ctx) {
		//$base = $listener->getBase();
		$base = $this->base;

		fprintf(STDERR, "Got an error %d (%s) on the listener. "
			."Shutting down.\n",
			EventUtil::getLastSocketErrno(),
			EventUtil::getLastSocketError());

		$base->exit(NULL);
	}
}

$port = 9808;

if ($argc > 1) {
	$port = (int) $argv[1];
}
if ($port <= 0 || $port > 65535) {
	puts("Invalid port");
	return 1;
}

$l = new MyListener($port);
$l->dispatch();
