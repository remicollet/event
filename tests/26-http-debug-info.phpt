--TEST--
Check for EventHttp* objects debug info
--SKIPIF--
<?php
if (!class_exists('EventHttp')) {
	die("skip Event extra functions are disabled");
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

var_dump(new EventHttpRequest(function() {}, null));
var_dump(new EventHttp($base));
var_dump(new EventHttpConnection($base, null, '127.0.0.1', 0));
?>
--EXPECTF--
object(EventHttpRequest)#%d (0) {
}
object(EventHttp)#%d (0) {
}
object(EventHttpConnection)#%d (0) {
}
