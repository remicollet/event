--TEST--
Check for EventBufferEvent SSL features
--SKIPIF--
<?php
if (!class_exists('EventBufferEvent')) {
	die('skip Event is built without EventBufferEvent support');
}
$class = 'EventBufferEvent';
$prop = 'allow_ssl_dirty_shutdown';

if (!property_exists($class, $prop)) {
	die('skip Event SSL allow_ssl_dirty_shutdown property is not supported');
}
if (version_compare(PHP_VERSION, '7.0.0') < 0) {
	die('skip target is PHP version >= 7');
}
?>
--FILE--
<?php
$base = new EventBase();

foreach ([
	'SSLv3'   => 'EventSslContext::SSLv3_SERVER_METHOD',
	'SSLv2'   => 'EventSslContext::SSLv2_SERVER_METHOD',
	'TLSv1.1' => 'EventSslContext::TLSv11_SERVER_METHOD',
	'TLSv1.2' => 'EventSslContext::TLSv12_SERVER_METHOD'
] as $k => $method)
{
    if (!defined($method)) {
        var_dump($k);
        var_dump(false);
        var_dump(false);
        var_dump(true);
        continue;
    }
    $method = constant($method);

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
