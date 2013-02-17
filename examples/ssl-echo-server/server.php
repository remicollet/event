<?php
/*
 * SSL echo server
 *
 * To test it:
 * 1) Generate certificates:
 *
 *	$ ./pem-server.sh
 *	# Fill fields ...
 *	$ ./pem-client.sh
 *
 * 2) Run the server:
 *	$ php ./server.php
 *	Optionally provide port:
 *	$ php ./server.php 9999
 *
 * 3) In another terminal window run client:
 *
 *	$ ./client.sh
 *	Optionally provide port:
 *	$ ./client.sh 9999
 */

// This callback is invoked when there is data to read on $bev.
function ssl_read_cb($bev, $ctx) {
	$in = $bev->getInput();

	printf("Received %zu bytes\n", $in->length);
    printf("----- data ----\n");
    printf("%ld:\t%s\n", (int) $in->length, $in->pullup(-1));

	$bev->writeBuffer($in);
}

// This callback is invoked when some even occurs on the event listener,
// e.g. connection closed, or an error occured
function ssl_event_cb($bev, $events, $ctx) {
	if ($events & EventBufferEvent::ERROR) {
		// Fetch errors from the SSL error stack
		while ($err = $bev->sslError()) {
			fprintf(STDERR, "Bufferevent error %s.\n", $err);
		}
	}

	if ($events & (EventBufferEvent::EOF | EventBufferEvent::ERROR)) {
		$bev->free();
	}
}

// This callback is invoked when a client accepts new connection
function ssl_accept_cb($listener, $fd, $address, $ctx) {
	// We got a new connection! Set up a bufferevent for it.
	$base = $listener->getBase();

	$bev = EventBufferEvent::sslSocket($base, $fd, $ctx,
		EventBufferEvent::SSL_ACCEPTING, EventBufferEvent::OPT_CLOSE_ON_FREE);

	if (!$bev) {
		echo "Failed creating ssl buffer\n";
		$base->exit(NULL);
		exit(1);
	}

	$bev->enable(Event::READ);
	$bev->setCallbacks("ssl_read_cb", NULL, "ssl_event_cb", NULL);
}

// This callback is invoked when we failed to setup new connection for a client
function accept_error_cb($listener, $ctx) {
	$base = $listener->getBase();

	fprintf(STDERR, "Got an error %d (%s) on the listener. "
		."Shutting down.\n",
		EventUtil::getLastSocketErrno(),
		EventUtil::getLastSocketError());

	$base->exit(NULL);
}

// Initialize SSL structures, create an EventSslContext
// Optionally create self-signed certificates
function init_ssl($port) {
	// We *must* have entropy. Otherwise there's no point to crypto.
	if (!EventUtil::sslRandPoll()) {
		exit("EventUtil::sslRandPoll failed\n");
	}

	$pem_passphrase = "echo server";
	$pem_file       = __DIR__. "/server.pem";
	$ca_file        = __DIR__. "/client.crt";

	$pem_dn = array (
 		"countryName"            => "RU",
 		"stateOrProvinceName"    => "Moscow",
 		"localityName"           => "Moscow",
 		"organizationName"       => "Fancy Company",
 		"organizationalUnitName" => "Fancy Department",
 		"commonName"             => "devbox",
 		"emailAddress"           => "rrosmanov@gmail.com"
	);
	if (!file_exists($pem_file)) {
		system("./pem-server.sh; ./pem-client.sh");
	}

	$ctx = new EventSslContext(EventSslContext::SSLv3_SERVER_METHOD, array (
 		EventSslContext::OPT_LOCAL_CERT        => $pem_file,
 		EventSslContext::OPT_CA_FILE           => $ca_file,
 		EventSslContext::OPT_PASSPHRASE        => $pem_passphrase,
 		EventSslContext::OPT_ALLOW_SELF_SIGNED => true,
 		EventSslContext::OPT_VERIFY_PEER       => true,
	));
	return $ctx;
}

// Allow to override the port
$port = 9999;
if ($argc > 1) {
	$port = (int) $argv[1];
}
if ($port <= 0 || $port > 65535) {
	exit("Invalid port\n");
}

$host = "127.0.0.1";

$ctx = init_ssl($port);
if (!$ctx) {
	exit("Failed creating SSL context\n");
}

$base = new EventBase();
if (!$base) {
	exit("Couldn't open event base\n");
}

$listener = new EventListener($base, "ssl_accept_cb", $ctx,
	EventListener::OPT_CLOSE_ON_FREE | EventListener::OPT_REUSEABLE,
	-1, "$host:$port");
if (!$listener) {
	exit("Couldn't create listener\n");
}
$listener->setErrorCallback("accept_error_cb");

$base->dispatch();
