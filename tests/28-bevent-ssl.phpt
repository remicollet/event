--TEST--
Check for EventBufferEvent SSL features
--SKIPIF--
<?php
if (!class_exists('EventBufferEvent')) {
	die('skip Event is built without EventBufferEvent support');
}
$class = 'EventBufferEvent';
$prop = 'allow_ssl_dirty_shutdown';
if (!(property_exists($class, "\"$prop\"") || property_exists($class, $prop))) {
	die('skip Event SSL support is disabled');
}
if (version_compare(PHP_VERSION, '7.0.0') < 0) {
	die('skip target is PHP version >= 7');
}
?>
--FILE--
<?php
$base = new EventBase();

foreach ([
	EventSslContext::SSLv3_SERVER_METHOD,
	EventSslContext::SSLv2_SERVER_METHOD,
	EventSslContext::TLSv11_SERVER_METHOD,
	EventSslContext::TLSv12_SERVER_METHOD,
] as $method)
{
	@$ctx = new EventSslContext($method, []);
	$bev = EventBufferEvent::sslSocket($base, null, $ctx, EventBufferEvent::SSL_ACCEPTING);
	var_dump($bev->sslGetProtocol());

	var_dump($bev->allow_ssl_dirty_shutdown);

	$bev->allow_ssl_dirty_shutdown = false;
	var_dump($bev->allow_ssl_dirty_shutdown);

	$bev->allow_ssl_dirty_shutdown = true;
	var_dump($bev->allow_ssl_dirty_shutdown);
}
?>
--EXPECT--
string(5) "SSLv3"
bool(false)
bool(false)
bool(true)
string(5) "SSLv2"
bool(false)
bool(false)
bool(true)
string(7) "TLSv1.1"
bool(false)
bool(false)
bool(true)
string(7) "TLSv1.2"
bool(false)
bool(false)
bool(true)
