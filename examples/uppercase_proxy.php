<?php
/* TODO: Maybe use bufferevent pairs to complete example? */

function read_callback_uppercase($bev, $unused) {
	/* This callback removes the data from $bev's input buffer 128
		bytes at a time, uppercases it, and starts sending it
		back.
	 */

	$tmp = NULL;

	while (1) {
		$n = bufferevent_read($bev, $tmp, 128);
		($n > 0) or break;
		bufferevent_write($bev, strtoupper($tmp), $n);
	}
}
?>
