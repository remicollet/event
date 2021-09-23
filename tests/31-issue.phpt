--TEST--
Check for issue #31
--FILE--
<?php
$eventBaseClass = EVENT_NS . '\\EventBase';
$eventClass = EVENT_NS . '\\Event';
$eventUtilClass = EVENT_NS . '\\EventUtil';

$stdout = fopen("php://stdout", 'w');
$stdin = fopen("php://stdin", 'r');

stream_set_blocking($stdout, false);
stream_set_blocking($stdin, false);

register_shutdown_function(function () use (&$stdin, &$stdout) {
	fclose($stdin);
	fclose($stdout);
});

$b = new $eventBaseClass();
$e = new $eventClass(
	$b,
	$stdout, $eventClass::READ | $eventClass::WRITE,
	function ($fd, $what, $e) use ($eventUtilClass) {
		$numericFd = $eventUtilClass::getSocketFd($fd);
		printf("fd: %s\n", var_export($numericFd, true));
		printf("fd > 2: %s\n", var_export($numericFd > 2, true));
	}
);
$e->add();
$b->loop();

$e->set(
	$b,
	$stdin,
	$eventClass::READ | $eventClass::WRITE,
	function ($fd, $what, $e) use ($eventUtilClass) {
		$numericFd = $eventUtilClass::getSocketFd($fd);
		printf("fd: %s\n", var_export($numericFd, true));
		printf("fd > 2: %s\n", var_export($numericFd > 2, true));
	}
);
$e->add();
$b->loop();
?>
--EXPECTF--
fd: %d
fd > 2: true
fd: %d
fd > 2: true
