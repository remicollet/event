--TEST--
Check for EventBufferEvent::sslFilter() error behavior
--FILE--
<?php
$base = new EventBase();
$b = new EventBufferEvent($base);
$ctx = new EventSslContext(EventSslContext::SSLv3_SERVER_METHOD, []);
EventBufferEvent::sslFilter(new EventBase(), $b, $ctx, EventBufferEvent::SSL_ACCEPTING);
?>
--EXPECTF--

Fatal error: EventBufferEvent::sslFilter(): EventBase must be passed by reference in %s on line %d
