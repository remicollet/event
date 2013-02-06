--TEST--
Check for manupulation with buffer position
--FILE--
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

// 1 12 123 1234 .. 123..9
$i = 1;
$s = "";
$a = "";
while ($i < 10) {
	$s .= $i;
	$a .= $s ." ";
	++$i;
}

$buf = new EventBuffer();
$buf->add($a);

while (--$i > 0) {
	echo $i, " - ", count_instances($buf, $i), PHP_EOL;
}
?>
--EXPECT--
9 - 1
8 - 2
7 - 3
6 - 4
5 - 5
4 - 6
3 - 7
2 - 8
1 - 9

