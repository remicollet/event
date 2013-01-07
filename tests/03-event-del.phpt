--TEST--
Check for event_add and event_del
--FILE--
<?php 
$base = event_base_new();

$e1 = event_timer_new($base, function () { echo "not ok 3\n"; });
event_add($e1, 0.1);

$e2 = event_timer_new($base, function () { echo "ok 3\n"; });
event_add($e2, 0.2);

event_timer_pending($e1) and print("ok 1\n");
event_timer_pending($e2) and print("ok 2\n");

event_del($e1);
event_timer_pending($e1) and print("not ok 4\n");

event_base_loop($base, EVENT_LOOP_ONCE);

event_timer_set($e1, $base, function() { echo "ok 4\n"; });
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
