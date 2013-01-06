--TEST--
Check for event configuration features 
--FILE--
<?php 
$cfg = event_config_new();

if (event_config_require_features($cfg, EVENT_FEATURE_FDS)) {
	$base = event_base_new_with_config($cfg);

	if (event_base_get_features($base) & EVENT_FEATURE_FDS) {
		echo "FDS\n";
	}
}

if (event_config_require_features($cfg, EVENT_FEATURE_ET)) {
	$base = event_base_new_with_config($cfg);

	if (event_base_get_features($base) & EVENT_FEATURE_ET) {
		echo "ET\n";
	}
}

if (event_config_require_features($cfg, EVENT_FEATURE_O1)) {
	$base = event_base_new_with_config($cfg);

	if (event_base_get_features($base) & EVENT_FEATURE_O1) {
		echo "O1\n";
	}
}

?>
--EXPECT--
FDS
ET
O1
