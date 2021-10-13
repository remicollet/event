--TEST--
Check for issue #69 - handling event callback call failure
--SKIPIF--
<?php
if (PHP_VERSION_ID < 80000) {
	die("skip only for PHP > 8");
}
?>
--FILE--
<?php
$eventBaseClass = EVENT_NS . '\\EventBase';
$eventClass = EVENT_NS . '\\Event';

echo "timer/RuntimeException\n";
$base = new $eventBaseClass();
$timer = new $eventClass($base, -1, $eventClass::TIMEOUT, function () use ($base) {
    static $counter = 0;
    if (++$counter > 10) {
        $base->stop();
    }

    throw new RuntimeException("Timer");
});
$timer->addTimer(0);
try {
    $base->loop();
} catch (RuntimeException $e) {
    printf("Caught %s: %s\n", get_class($e), $e->getMessage());
}

echo "event/RuntimeException\n";
$stream = fopen('php://stdout', 'w');
stream_set_blocking($stream, false);
$event = new $eventClass($base, $stream, Event::WRITE|Event::PERSIST, function () use ($base) {
    static $counter = 0;
    if (++$counter > 10) {
        $base->stop();
    }
    throw new RuntimeException('Stream watcher');
});
$event->add();
try {
    $base->loop();
} catch (RuntimeException $e) {
    printf("Caught %s: %s\n", get_class($e), $e->getMessage());
}

echo "event/exit\n";
$stream = fopen('php://stdout', 'w');
stream_set_blocking($stream, false);
$event = new $eventClass($base, $stream, Event::WRITE|Event::PERSIST, function () use ($base) {
    static $counter = 0;
    if (++$counter > 10) {
        $base->stop();
    }
    exit("exit()\n");
});
$event->add();
try {
    $base->loop();
} catch (RuntimeException $e) {
    printf("Caught %s: %s\n", get_class($e), $e->getMessage());
}

?>
--EXPECTF--
timer/RuntimeException
Caught RuntimeException: Timer
event/RuntimeException

Warning: EventBase::loop(): Failed to invoke event callback, breaking the loop%a
Caught RuntimeException: Stream watcher
event/exit
exit()

Warning: EventBase::loop(): Failed to invoke event callback, breaking the loop%a
