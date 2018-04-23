--TEST--
Check for gc_collect_cycles appled after event free
--SKIPIF--
<?php if (!function_exists("gc_collect_cycles")) print "skip"; ?>
--FILE--
<?php
$eventBaseClass = EVENT_NS . '\\EventBase';
$eventClass = EVENT_NS . '\\Event';

$base = new $eventBaseClass();
$e = new $eventClass($base, 0, $eventClass::READ, function(){});
$e->free();
gc_collect_cycles(); // segfaults if something goes wrong
echo "ok";
?>
--EXPECT--
ok
