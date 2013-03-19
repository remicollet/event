<?php

/* Count total occurances of 'str' in 'buf' */
function count_instances($buf, $str) {
    $total = 0;
	$p = 0;

	$i = 0;
    while (1) {
        $p = $buf->search($str, $p);
        if ($p === FALSE) {
            break;
		}
        ++$total;
		++$p;
    }

    return $total;
}

$buf = new EventBuffer();
$buf->add("Some string within a string inside another string");
var_dump(count_instances($buf, "str"));
?>
