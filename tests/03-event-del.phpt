--TEST--
Check for event_add and event_del
--FILE--
<?php 
$base = event_base_new();

$e1 = evtimer_new($base, function () { echo "not ok 3\n"; });
event_add($e1, 0.1);

$e2 = evtimer_new($base, function () { echo "ok 3\n"; });
event_add($e2, 0.2);

evtimer_pending($e1) and print("ok 1\n");
evtimer_pending($e2) and print("ok 2\n");

event_del($e1);
evtimer_pending($e1) and print("not ok 4\n");

event_base_loop($base, EVENT_LOOP_ONCE);

evtimer_set($e1, $base, function() { echo "ok 4\n"; });
event_add($e1, 0.3);
event_base_loop($base, EVENT_LOOP_ONCE);

event_del($e1);
event_del($e2);
event_base_loop($base);

?>
--EXPECT--
ok 1
ok 2
ok 3
ok 4
