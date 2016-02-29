--TEST--
Check for EventBufferEvent::createSslFilter() behavior
--SKIPIF--
<?php
if (!class_exists("EventSslContext")) die("skip Event extra functions are disabled");
if (version_compare(PHP_VERSION, '7.0.0') < 0) {
	die('skip target is PHP version >= 7');
}
?>
--FILE--
<?php
$base = new EventBase();
$b = new EventBufferEvent($base);
$ctx = new EventSslContext(EventSslContext::SSLv3_SERVER_METHOD, []);
EventBufferEvent::createSslFilter($b, $ctx, EventBufferEvent::SSL_ACCEPTING);
echo 'ok';
?>
--EXPECT--
ok