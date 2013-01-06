<?php
/* {{{ Config & supported stuff */
echo "Supported methods:\n";
foreach (event_get_supported_methods() as $m) {
	echo $m, PHP_EOL;
}

// Avoiding "select" method
$cfg = event_config_new();
if (event_config_avoid_method($cfg, "select")) {
	echo "`select' method avoided\n";
}

// Create event_base associated with the config
$base = event_base_new_with_config($cfg);
echo "Event method used: ", event_base_get_method($base), PHP_EOL;

echo "Features:\n";
$features = event_base_get_features($base);
($features & EVENT_FEATURE_ET) and print("ET - edge-triggered IO\n");
($features & EVENT_FEATURE_O1) and print("O1 - O(1) operation for adding/deletting events\n");
($features & EVENT_FEATURE_FDS) and print("FDS - arbitrary file descriptor types, and not just sockets\n");

// Require FDS feature
if (event_config_require_features($cfg, EVENT_FEATURE_FDS)) {
	echo "FDS feature is now requried\n";

	$base = event_base_new_with_config($cfg);
	(event_base_get_features($base) & EVENT_FEATURE_FDS)
		and print("FDS - arbitrary file descriptor types, and not just sockets\n");
}
/* }}} */

/* {{{ Base */
$base = event_base_new();
$event = event_new($base, STDIN, EVENT_READ | EVENT_PERSIST, function ($fd, $events, $arg) {
	static $max_iterations = 0;

    if (++$max_iterations >= 5) {
		/* exit after 5 iterations with timeout of 2.33 seconds */
		echo "Stopping...\n";
        event_base_loopexit($arg[0], 2.33);
    }

    echo fgets($fd);
}, array (&$base));

event_add($event);
event_base_loop($base);
/* Base }}} */
?>

