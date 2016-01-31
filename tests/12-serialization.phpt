--TEST--
Check for SEGFAULT with serialization functions
--SKIPIF--
<?php if (!class_exists("EventListener")) print "skip Event extra functions are disabled"; ?>
--FILE--
<?php
$base = new EventBase();
$listener = new EventListener($base, function () { }, null, 0, -1, '0.0.0.0:12345');

// The following caused segmentation faults
serialize($listener);
if (function_exists('json_encode')) {
	json_encode($listener);
}

function a($a) { debug_backtrace(0, 3); }
a($listener);
echo "ok";
?>
--EXPECT--
ok
