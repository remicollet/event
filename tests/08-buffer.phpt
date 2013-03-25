--TEST--
Check for EventBuffer methods' behaviour
--FILE--
<?php
$s     = "abcdefghijklmnopqrstuvwxyz";
$s_len = strlen($s);
$b1    = new EventBuffer();
$b2    = new EventBuffer();

$b1->add($s);

$tmp = "";
for ($i = 0, $j = 1; $i < $s_len; $i += 4, ++$j) {
	if ($b2->removeBuffer($b1, 4)) {
		$b2->remove($tmp, 32);
		echo $j, ' ', $tmp == substr($s, $i, 4) ? "ok" : "failed", PHP_EOL;
	}
}
?>
--EXPECT--
1 ok
2 ok
3 ok
4 ok
5 ok
6 ok
7 ok

