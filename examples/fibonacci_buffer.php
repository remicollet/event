<?php
/* TODO: Maybe use bufferevent pairs to complete example? */

function write_callback_fibonacci($bev, $c) {
	/* Here's a callback that adds some Fibonacci numbers to the
	   	output buffer of $bev.  It stops once we have added 1k of
	   	data; once this data is drained, we'll add more. */

	echo __FUNCTION__, PHP_EOL;

	$tmp = evbuffer_new();
	while (evbuffer_get_length($tmp) < 1024) {
		$next = $c[0] + $c[1];
		$c[0] = $c[1];
		$c[1] = $next;

		evbuffer_add($tmp, $next);
	}

	/* Now we add the whole contents of tmp to bev. */
	bufferevent_write_buffer($bev, $tmp);

	/* We don't need tmp any longer. */
	evbuffer_free($tmp);
}
?>
