--TEST--
Check for issue #49
--FILE--
<?php
$eventBaseClass = EVENT_NS . '\\EventBase';
$eventClass = EVENT_NS . '\\Event';

$base = new $eventBaseClass;
$stream = STDOUT;
$event = new $eventClass($base, $stream, $eventClass::READ | $eventClass::PERSIST, function () {
    echo "ok\n";
});
$event->add(.1);
$base->loop($eventBaseClass::LOOP_ONCE);

?>
--EXPECT--
ok
