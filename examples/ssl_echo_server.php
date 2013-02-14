<?php
//
// To test: socat - SSL:devbox:9999,verify=0
//

// Creates self-signed certificate in PEM format
function create_cert($pem_file, $pem_passphrase, $pem_dn) {
 	$privkey = openssl_pkey_new();
 	$cert    = openssl_csr_new($pem_dn, $privkey);
 	$cert    = openssl_csr_sign($cert, null, $privkey, 365);

 	$pem = array();
 	openssl_x509_export($cert, $pem[0]);
 	openssl_pkey_export($privkey, $pem[1], $pem_passphrase);
 	$pem = implode($pem);

 	file_put_contents($pem_file, $pem);
 	chmod($pem_file, 0600);
}

// This callback is invoked when there is data to read on $bev.
function ssl_read_cb($bev, $ctx) {
	$in = $bev->getInput();

	printf("Received %zu bytes\n", $in->length);
    printf("----- data ----\n");
    printf("%ld:\t%s\n", (int) $in->length, $in->pullup(-1));

	$bev->writeBuffer($in);
}

function ssl_event_cb($bev, $events, $ctx) {
	if ($events & EventBufferEvent::ERROR) {
		while ($err = $bev->sslError()) {
			fprintf(STDERR, "Bufferevent error %s.\n", $err);
		}
	}

	if ($events & (EventBufferEvent::EOF | EventBufferEvent::ERROR)) {
		$bev->free();
	}
}

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

function accept_error_cb($listener, $ctx) {
	$base = $listener->getBase();

	fprintf(STDERR, "Got an error %d (%s) on the listener. "
		."Shutting down.\n",
		EventUtil::getLastSocketErrno(),
		EventUtil::getLastSocketError());

	$base->exit(NULL);
}

function init_ssl() {
	// We *must* have entropy. Otherwise there's no point to crypto.
	if (!EventUtil::sslRandPoll()) {
		exit("EventUtil::sslRandPoll failed\n");
	}

	$pem_passphrase = "echo server";
	$pem_file       = "cert.pem";

	$pem_dn = array (
 		"countryName"            => "RU",
 		"stateOrProvinceName"    => "Moscow",
 		"localityName"           => "Moscow",
 		"organizationName"       => "Fancy Company",
 		"organizationalUnitName" => "Fancy Department",
 		"commonName"             => "devbox",
 		"emailAddress"           => "rrosmanov@gmail.com"
	);
	create_cert($pem_file, $pem_passphrase, $pem_dn);

	$ctx = new EventSslContext(EventSslContext::SSLv23_SERVER_METHOD, array (
 		EventSslContext::OPT_LOCAL_CERT        => $pem_file,
 		EventSslContext::OPT_PASSPHRASE        => $pem_passphrase,
 		EventSslContext::OPT_ALLOW_SELF_SIGNED => true,
 		EventSslContext::OPT_VERIFY_PEER       => false,
	));
	return $ctx;
}

$port = 9999;
if ($argc > 1) {
	$port = (int) $argv[1];
}
if ($port <= 0 || $port > 65535) {
	exit("Invalid port\n");
}

$host = "127.0.0.1";

$ctx = init_ssl();
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
