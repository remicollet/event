<?php
/*
 * Sample OpenSSL client.
 *
 * Usage:
 * 1) Launch a server, e.g.:
 * $ php examples/https.php 9999
 *
 * 2) Launch the client in another terminal:
 * $ php examples/ssl-connection.php 9999
 */

function _request_handler($req, $base) {
	echo __FUNCTION__, PHP_EOL;

	if (is_null($req)) {
		echo "Timed out\n";
	} else {
		$response_code = $req->getResponseCode();

		if ($response_code == 0) {
			echo "Connection refused\n";
		} elseif ($response_code != 200) {
			echo "Unexpected response: $response_code\n";
		} else {
			echo "Success: $response_code\n";
			$buf = $req->getInputBuffer();
			echo "Body:\n";
			while ($s = $buf->readLine(EventBuffer::EOL_ANY)) {
				echo $s, PHP_EOL;
			}
		}
	}

	$base->exit(NULL);
}

function _init_ssl() {
	$local_cert = __DIR__."/ssl-echo-server/cert.pem";
	$local_pk   = __DIR__."/ssl-echo-server/privkey.pem";

	$ctx = new EventSslContext(EventSslContext::SSLv3_CLIENT_METHOD, array (
		EventSslContext::OPT_LOCAL_CERT  => $local_cert,
		EventSslContext::OPT_LOCAL_PK    => $local_pk,
		//EventSslContext::OPT_PASSPHRASE  => "test",
		EventSslContext::OPT_ALLOW_SELF_SIGNED => true,
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
$host = 'localhost';

$ctx = _init_ssl();
if (!$ctx) {
	trigger_error("Failed creating SSL context", E_USER_ERROR);
}

$base = new EventBase();
if (!$base) {
	trigger_error("Failed to initialize event base", E_USER_ERROR);
}

echo "new EventHttpConnection(base, null, $host, $port, ctx)\n";
$conn = new EventHttpConnection($base, NULL, $host, $port, $ctx);
$conn->setTimeout(50);

$req = new EventHttpRequest("_request_handler", $base);
$req->addHeader("Host", $host, EventHttpRequest::OUTPUT_HEADER);
$conn->makeRequest($req, EventHttpRequest::CMD_GET, "/about");

$base->dispatch();
echo "END\n";
?>
