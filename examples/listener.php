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

function echo_read_cb($bev, $ctx) {
	/* This callback is invoked when there is data to read on $bev. */
	$input	= $bev->getInput();
	$output = $bev->getOutput();

	/* Copy all the data from the input buffer to the output buffer. */
	$output->addBuffer($input);
}

function echo_event_cb($bev, $events, $ctx) {
	if ($events & EventBufferEvent::ERROR)
		echo "Error from bufferevent\n";

	if ($events & (EventBufferEvent::EOF | EventBufferEvent::ERROR)) {
		$bev->free();
	}
}

function accept_conn_cb($listener, $fd, $address, $ctx) {
	/* We got a new connection! Set up a bufferevent for it. */
	$base = $ctx;
	//$base = $listener->getBase();

	$bev = new EventBufferEvent($base, $fd, EventBufferEvent::OPT_CLOSE_ON_FREE);

	$bev->setCallbacks("echo_read_cb", NULL, "echo_event_cb", NULL);

	$bev->enable(Event::READ | Event::WRITE);

	//$bev->ref();
}

function accept_error_cb($listener, $ctx) {
	$base = $listener->getBase();

	fprintf(STDERR, "Got an error %d (%s) on the listener. "
		."Shutting down.\n",
		EventUtil::getLastSocketErrno(),
		EventUtil::getLastSocketError());

	$base->exit(NULL);
}

$port = 9808;

if ($argc > 1) {
	$port = (int) $argv[1];
}
if ($port <= 0 || $port > 65535) {
	puts("Invalid port");
	return 1;
}

$base = new EventBase();
if (!$base) {
	echo "Couldn't open event base";
	exit(1);
}

/* Variant #1 */
$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
if (!socket_bind($socket, '0.0.0.0', $port)) {
	echo "Unable to bind socket\n";
	exit(1);
}
$listener = new EventListener($base, "accept_conn_cb", $base,
	EventListener::OPT_CLOSE_ON_FREE | EventListener::OPT_REUSEABLE, -1, $socket);

/* Variant #2 */
/*
$listener = new EventListener($base, "accept_conn_cb", $base,
	EventListener::OPT_CLOSE_ON_FREE | EventListener::OPT_REUSEABLE, -1, "0.0.0.0:$port");
 */

if (!$listener) {
	echo "Couldn't create listener";
	exit(1);
}
$listener->setErrorCallback("accept_error_cb");

$base->dispatch();
