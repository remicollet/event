--TEST--
Check for EventBufferEvent when exception is thrown from the read callback
--SKIPIF--
<?php
if (PHP_VERSION_ID < 80200) {
	die('skip this test is for PHP version >= 8.2.0');
}
?>
--FILE--
<?php
$eventClass = EVENT_NS . '\\Event';
$eventBaseClass = EVENT_NS . '\\EventBase';
$eventBufferEventClass = EVENT_NS . '\\EventBufferEvent';

$base = new $eventBaseClass();
$pair = $eventBufferEventClass::createPair($base);

[$reader, $writer] = $pair;

$writer->enable($eventClass::WRITE);
$reader->enable($eventClass::READ);

$reader->setCallbacks(function () {
    throw new \RuntimeException('from EventBufferEvent callback');
}, null, null, null);

$writer->write('xyz');

$base->loop();
?>
--EXPECTF--

Warning: EventBase::loop(): Breaking the loop due to exception RuntimeException in %s/33-bevent-read-cb-exception.php on line %d

Fatal error: Uncaught RuntimeException: from EventBufferEvent callback in %s/33-bevent-read-cb-exception.php:%d
Stack trace:
#0 [internal function]: {closure}(Object(EventBufferEvent), NULL)
#1 %s/33-bevent-read-cb-exception.php(%d): %SEventBase->loop()
#2 {main}
%wthrown in %s/33-bevent-read-cb-exception.php on line %d
