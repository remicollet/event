--TEST--
Check for EventBufferEvent::createPair()
--FILE--
<?php
$base = new EventBase();

$pair = EventBufferEvent::createPair($base);

$pair[0]->enable(Event::WRITE);
$pair[1]->enable(Event::READ);
$pair[0]->write("xyz");
echo $pair[1]->read(10) == "xyz" ? "ok" : 'failed', PHP_EOL;
$base->loop();

$pair[0]->disable(Event::WRITE);
$pair[0]->write("xyz");
echo $pair[1]->read(10) == "" ? "ok" : 'failed', PHP_EOL;
$base->loop();
?>
--EXPECTF--
ok
ok
