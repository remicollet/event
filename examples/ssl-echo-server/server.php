<?php
/*
 * SSL echo server
 *
 * To test:
 * 1) Run:
 * $ php examples/ssl-echo-server/server.php 9998
 *
 * 2) in another terminal window run:
 * $ socat - SSL:127.0.0.1:9998,verify=1,cafile=examples/ssl-echo-server/cert.pem
 */

class MySslEchoServer {
	public $port,
		$base,
		$bev,
		$listener,
		$ctx;

	function __construct ($port, $host = "127.0.0.1") {
		$this->port = $port;
		$this->ctx = $this->init_ssl();
		if (!$this->ctx) {
			exit("Failed creating SSL context\n");
		}

		$this->base = new EventBase();
		if (!$this->base) {
			exit("Couldn't open event base\n");
		}

		$this->listener = new EventListener($this->base,
			array($this, "ssl_accept_cb"), $this->ctx,
			EventListener::OPT_CLOSE_ON_FREE | EventListener::OPT_REUSEABLE,
			-1, "$host:$port");
		if (!$this->listener) {
			exit("Couldn't create listener\n");
		}

		$this->listener->setErrorCallback(array($this, "accept_error_cb"));
	}
	function dispatch() {
		$this->base->dispatch();
	}

	// This callback is invoked when there is data to read on $bev.
	function ssl_read_cb($bev, $ctx) {
		$in = $bev->input; //$bev->getInput();

		printf("Received %ld bytes\n", $in->length);
    	printf("----- data ----\n");
    	printf("%ld:\t%s\n", (int) $in->length, $in->pullup(-1));

		$bev->writeBuffer($in);
	}

	// This callback is invoked when some even occurs on the event listener,
	// e.g. connection closed, or an error occured
	function ssl_event_cb($bev, $events, $ctx) {
		echo __METHOD__, PHP_EOL;
		if ($events & EventBufferEvent::ERROR) {
			fprintf(STDERR, "Error! Events: 0x%x\n", $events);
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
		$this->bev = EventBufferEvent::sslSocket($this->base, $fd, $this->ctx,
			EventBufferEvent::SSL_ACCEPTING, EventBufferEvent::OPT_CLOSE_ON_FREE);

		if (!$this->bev) {
			echo "Failed creating ssl buffer\n";
			$this->base->exit(NULL);
			exit(1);
		}

		$this->bev->enable(Event::READ);
		$this->bev->setCallbacks(array($this, "ssl_read_cb"), NULL,
			array($this, "ssl_event_cb"), NULL);
	}

	// This callback is invoked when we failed to setup new connection for a client
	function accept_error_cb($listener, $ctx) {
		fprintf(STDERR, "Got an error %d (%s) on the listener. "
			."Shutting down.\n",
			EventUtil::getLastSocketErrno(),
			EventUtil::getLastSocketError());

		$this->base->exit(NULL);
	}

	// Initialize SSL structures, create an EventSslContext
	// Optionally create self-signed certificates
	function init_ssl() {
		// We *must* have entropy. Otherwise there's no point to crypto.
		if (!EventUtil::sslRandPoll()) {
			exit("EventUtil::sslRandPoll failed\n");
		}

		$local_cert = __DIR__."/cert.pem";
		$local_pk   = __DIR__."/privkey.pem";

		if (!file_exists($local_cert) || !file_exists($local_pk)) {
			echo "Couldn't read $local_cert or $local_pk file.  To generate a key\n",
				"and self-signed certificate, run:\n",
				"  openssl genrsa -out $local_pk 2048\n",
				"  openssl req -new -key $local_pk -out cert.req\n",
				"  openssl x509 -req -days 365 -in cert.req -signkey $local_pk -out $local_cert\n";

			return FALSE;
		}

		$ctx = new EventSslContext(EventSslContext::SSLv3_SERVER_METHOD, array (
 			EventSslContext::OPT_LOCAL_CERT  => $local_cert,
 			EventSslContext::OPT_LOCAL_PK    => $local_pk,
 			EventSslContext::OPT_PASSPHRASE  => "test",
 			EventSslContext::OPT_VERIFY_PEER => true,
 			EventSslContext::OPT_ALLOW_SELF_SIGNED => true,
		));

		return $ctx;
	}
}

// Allow to override the port
$port = 9999;
if ($argc > 1) {
	$port = (int) $argv[1];
}
if ($port <= 0 || $port > 65535) {
	exit("Invalid port\n");
}


$l = new MySslEchoServer($port);
$l->dispatch();
