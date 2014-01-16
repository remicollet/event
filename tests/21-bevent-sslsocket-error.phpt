--TEST--
Check for EventBufferEvent::sslSocket() error behavior
--FILE--
<?php
$ctx = new EventSslContext(EventSslContext::SSLv3_SERVER_METHOD, []);
EventBufferEvent::sslSocket(new EventBase(), NULL, $ctx, EventBufferEvent::SSL_ACCEPTING);
?>
--EXPECTF--

Fatal error: EventBufferEvent::sslSocket(): EventBase must be passed by reference in %s on line %d
