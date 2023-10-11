--TEST--
Check for EventBufferEvent when exception is thrown from the read callback
--SKIPIF--
<?php
    if (PHP_VERSION_ID < 80200) {
        die('skip this test is for PHP version >= 8.2.0');
    }
    if (getenv("SKIP_ONLINE_TESTS")) {
        die("skip test requiring internet connection");
    }
?>
--FILE--
<?php
$eventBaseClass = EVENT_NS . '\\EventBase';
$eventBufferEventClass = EVENT_NS . '\\EventBufferEvent';
$eventUtilClass = EVENT_NS . '\\EventUtil';

$base = new $eventBaseClass();
$bev = new $eventBufferEventClass($base, NULL, $eventBufferEventClass::OPT_CLOSE_ON_FREE);

if (!$bev->connectHost(NULL, "www.php.net", 80, $eventUtilClass::AF_INET)) {
	exit("Connection failed\n");
}

$bev->setCallbacks(null, null, function ($bev, $events, $data) {
    throw new RuntimeException("from event callback");
});

$base->dispatch();
$bev->free();
?>
--EXPECTF--

Warning: EventBase::dispatch(): Breaking the loop due to exception RuntimeException in %s/33-bevent-event-cb-exception.php on line %d

Fatal error: Uncaught RuntimeException: from event callback in %s/33-bevent-event-cb-exception.php:%d
Stack trace:
#0 [internal function]: {closure}(Object(EventBufferEvent), %d, NULL)
#1 %s/33-bevent-event-cb-exception.php(%d): EventBase->dispatch()
#2 {main}
%wthrown in %s/33-bevent-event-cb-exception.php on line %d
