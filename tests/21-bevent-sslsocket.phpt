--TEST--
Check for EventBufferEvent::sslSocket() error behavior
--SKIPIF--
<?php
if (!class_exists("EventSslContext")) die("skip Event extra functions are disabled");
if (version_compare(PHP_VERSION, '7.0.0') < 0) {
	die('skip target is PHP version >= 7');
}
?>
--FILE--
<?php
foreach (['EventSslContext::SSLv3_SERVER_METHOD',
	'EventSslContext::SSLv2_SERVER_METHOD',
	'EventSslContext::SSLv23_SERVER_METHOD'] as $method)
{
	if (defined($method)) {
		$method = constant($method);
		break;
	}
}

$base = new EventBase();
$ctx = new EventSslContext($method, []);
EventBufferEvent::sslSocket(new EventBase(), null, $ctx, EventBufferEvent::SSL_ACCEPTING);
echo 'ok';
?>
--EXPECT--
ok
