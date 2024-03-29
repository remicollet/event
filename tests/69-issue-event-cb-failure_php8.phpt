--TEST--
Check for issue #69 - handling event callback call failure in PHP 8.2.0 and above
--SKIPIF--
<?php
// We have a test file dedecated to PHP 8.2.0.
if (PHP_VERSION_ID < 802000) {
	die("skip only for PHP version 8.2.0 and above");
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

Warning: EventBase::loop(): Breaking the loop due to exception%a
Caught RuntimeException: Stream watcher
event/exit
exit()

Warning: EventBase::loop(): Breaking the loop due to exception%a
