<?php

/* Count total occurances of 'str' in 'buf' */
function count_instances($buf, $str) {
    $total = 0;
	$p = new EventBufferPosition();

    $buf->setPosition($p, 0, EventBuffer::PTR_SET);

	$i = 0;
    while (1) {
        $p = $buf->search($str, $p);
        if (!$p) {
            break;
		}
        ++$total;
        $buf->setPosition($p, 1, EventBuffer::PTR_ADD);
    }

    return $total;
}

$buf = new EventBuffer();
$buf->add("Some string within a string inside another string");
var_dump(count_instances($buf, "str"));
?>
