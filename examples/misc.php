<?php
/* {{{ Config & supported stuff */
echo "Supported methods:\n";
foreach (Event::getSupportedMethods() as $m) {
	echo $m, PHP_EOL;
}

// Avoiding "select" method
$cfg = new EventConfig();
if ($cfg->avoidMethod("select")) {
	echo "`select' method avoided\n";
}

// Create event_base associated with the config
$base = new EventBase($cfg);
echo "Event method used: ", $base->getMethod(), PHP_EOL;

echo "Features:\n";
$features = $base->getFeatures();
($features & EVENT_FEATURE_ET) and print("ET - edge-triggered IO\n");
($features & EVENT_FEATURE_O1) and print("O1 - O(1) operation for adding/deletting events\n");
($features & EVENT_FEATURE_FDS) and print("FDS - arbitrary file descriptor types, and not just sockets\n");

// Require FDS feature
if ($cfg->requireFeatures(EVENT_FEATURE_FDS)) {
	echo "FDS feature is now requried\n";

	$base = new EventBase($cfg);
	($base->getFeatures() & EVENT_FEATURE_FDS)
		and print("FDS - arbitrary file descriptor types, and not just sockets\n");
}
/* }}} */

/* {{{ Base */
$base = new EventBase();
$event = new Event($base, STDIN, EVENT_READ | EVENT_PERSIST, function ($fd, $events, $arg) {
	static $max_iterations = 0;

    if (++$max_iterations >= 5) {
		/* exit after 5 iterations with timeout of 2.33 seconds */
		echo "Stopping...\n";
        $arg[0]->exit(2.33);
    }

    echo fgets($fd);
}, array (&$base));

$event->add();
$base->loop();
/* Base }}} */
?>

