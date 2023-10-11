--TEST--
Check for event behaviour with exit() in the callback
--FILE--
<?php
$eventBaseClass = EVENT_NS . '\\EventBase';
$eventClass = EVENT_NS . '\\Event';

$base = new $eventBaseClass();

$e = new $eventClass($base, -1, $eventClass::TIMEOUT, function() {
    echo "calling exit\n";
    // This tests that no warning is generated as a result of catching the UnwindExit
    // exception internally.
    // In this case, the event loop should be stopped silently.
    exit();
});
$e->addTimer(0.1);
$base->loop();
?>
--EXPECT--
calling exit
