--TEST--
Check for EventBufferEvent::sslSocket() error behavior
--SKIPIF--
<?php
if (!class_exists("EventSslContext")) die("skip Event extra functions are disabled");
?>
--FILE--
<?php
$ctx = new EventSslContext(EventSslContext::SSLv3_SERVER_METHOD, []);
EventBufferEvent::sslSocket(new EventBase(), NULL, $ctx, EventBufferEvent::SSL_ACCEPTING);
?>
--EXPECTF--

Fatal error: EventBufferEvent::sslSocket(): EventBase must be passed by reference in %s on line %d
