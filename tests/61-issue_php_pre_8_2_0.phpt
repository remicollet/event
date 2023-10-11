--TEST--
Check for issue #61 - segfault after throwing exception from an event handler in PHP version before 8.2.0
--SKIPIF--
<?php
if (PHP_VERSION_ID >= 80200) {
	die("skip only for PHP version before 8.2.0");
}
?>
--FILE--
<?php
$eventBaseClass = EVENT_NS . '\\EventBase';
$eventConfigClass = EVENT_NS . '\\EventConfig';
$eventClass = EVENT_NS . '\\Event';

$base = new $eventBaseClass();
$e = new $eventClass($base, -1, $eventClass::TIMEOUT, function () {
    throw new \Exception('issue61');
});
$e->addTimer(0);
$base->loop();
?>
--EXPECTF--
Fatal error: Uncaught Exception: issue61 in %s/61-issue_php_pre_8_2_0.php:%d
Stack trace:
#0 [internal function]: {closure}(-1, 1, NULL)
#1 %s/61-issue_php_pre_8_2_0.php(%d): %SEventBase->loop()
#2 {main}
  thrown in %s/61-issue_php_pre_8_2_0.php on line %d
