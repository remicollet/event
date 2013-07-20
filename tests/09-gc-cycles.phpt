--TEST--
Check for gc_collect_cycles appled after event free
--SKIPIF--
<?php if (!function_exists("gc_collect_cycles")) print "skip"; ?>
--FILE--
<?php
$base = new EventBase();
$e = new Event($base, 0, Event::READ, function(){});
$e->free();
gc_collect_cycles(); // segfaults if something goes wrong
echo "ok";
?>
--EXPECT--
ok
