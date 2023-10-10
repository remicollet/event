--TEST--
Check for EventBuffer::copyout method behavior
--FILE--
<?php
$eventBufferClass = EVENT_NS . '\\EventBuffer';

$a = [
	"",
	"test",
	"\0\0\0",
];

foreach ($a as $s) {
	$b = new $eventBufferClass();
	printf("add(%s): %s\n", var_export($s, true), var_export($b->add($s), true));

    $data = null;
    $bytes = $b->copyout($data, strlen($s));
    printf("bytes: %s\n", var_export($bytes, true));
    printf("data: %s\n", var_export($data, true));
    echo PHP_EOL;
}
?>
--EXPECT--
add(''): true
bytes: 0
data: NULL

add('test'): true
bytes: 4
data: 'test'

add('' . "\0" . '' . "\0" . '' . "\0" . ''): true
bytes: 3
data: '' . "\0" . '' . "\0" . '' . "\0" . ''

