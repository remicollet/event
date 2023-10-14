--TEST--
Check for EventDnsBase::parseResolvConf error behavior
--SKIPIF--
<?php
if (!class_exists(EVENT_NS . "\\EventDnsBase")) {
	die("skip Event extra functions are disabled");
}
?>
--FILE--
<?php
$eventDnsBaseClass = EVENT_NS . '\\EventDnsBase';
$eventBaseClass = EVENT_NS . '\\EventBase';
$eventClass = EVENT_NS . '\\Event';

$base = new $eventBaseClass();
$dnsBase = new $eventDnsBaseClass($base, false);

$dnsBase->parseResolvConf($eventDnsBaseClass::OPTION_NAMESERVERS, 'file_does_not_exist');

$tmpFilename = tempnam(sys_get_temp_dir(), 'php_event_resolvconf_');
file_put_contents($tmpFilename, "nameserver 127.0.0.53\n");
$dnsBase->parseResolvConf($eventDnsBaseClass::OPTION_NAMESERVERS, $tmpFilename);
?>
--CLEAN--
<?php
if (!empty($tmpFilename)) {
    unlink($tmpFilename);
}
?>
--EXPECTF--

Warning: EventDnsBase::parseResolvConf(): Failed to open file in %s/35-dns-base-parse-resolv-conf-error.php on line %d
