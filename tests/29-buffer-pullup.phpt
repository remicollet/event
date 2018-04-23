--TEST--
Check for EventBuffer::pullup method behavior
--FILE--
<?php
$eventBuffereClass = EVENT_NS . '\\EventBuffer';

$a = [
	"",
	"test",
	"\0\0\0",
];

foreach ($a as $s) {
	$b = new $eventBuffereClass();
	$b->add($s);
	$s_pullup = $b->pullup(-1);
	var_dump(strlen($s_pullup) == strlen($s) && $s_pullup == $s);
}
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
