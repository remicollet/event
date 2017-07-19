--TEST--
Check for SEGFAULT with serialization functions
--SKIPIF--
<?php if (!class_exists("EventListener")) print "skip Event extra functions are disabled"; ?>
--FILE--
<?php
$base = new EventBase();
$listener = new EventListener($base, function () { }, null, 0, -1, '0.0.0.0:12345');

$http = new EventHttp($base);
$http_request = new EventHttpRequest(function () {});
$http_connection = new EventHttpConnection($base, null, "0.0.0.0", 9099);
$config  = new EventConfig;

if (class_exists('EventSslContext')) {
    $ssl_context = new EventSslContext(EventSslContext::SSLv3_SERVER_METHOD, []);
    serialize($ssl_context);
}

/////////////////////////////////////////////

if (function_exists('json_encode')) {
	json_encode($listener);
}

function a($a) { debug_backtrace(0, 3); }
a($listener);
echo "1 - ok\n";

/////////////////////////////////////////////

foreach ([ $base, $http, $http_request, $http_connection, $config, $listener ] as &$object) {
    try {
        serialize($object);
    } catch (EventException $e) {
        echo get_class($object), " - ok\n";
    } finally {
        $object = null;
    }
}
?>
--EXPECT--
1 - ok
EventBase - ok
EventHttp - ok
EventHttpRequest - ok
EventHttpConnection - ok
EventConfig - ok
EventListener - ok
