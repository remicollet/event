<?php
function echo_read_cb($bev, $ctx) {
    /* This callback is invoked when there is data to read on $bev. */
    $input  = bufferevent_get_input($bev);
    $output = bufferevent_get_output($bev);

    /* Copy all the data from the input buffer to the output buffer. */
    evbuffer_add_buffer($output, $input);
}

function echo_event_cb($bev, $events, $ctx) {
    if ($events & EVENT_BEV_EVENT_ERROR)
        echo "Error from bufferevent";
    if ($events & (EVENT_BEV_EVENT_EOF | EVENT_BEV_EVENT_ERROR)) {
        bufferevent_free($bev);
    }
}

function accept_conn_cb($listener, $fd, $address, $ctx) {
	echo __FUNCTION__, " args:\n";
	var_dump($listener, $fd, $address, $ctx);
    /* We got a new connection! Set up a bufferevent for it. */
    $base = evconnlistener_get_base($listener);
    $bev = bufferevent_socket_new($base, $fd, EVENT_BEV_OPT_CLOSE_ON_FREE);

    bufferevent_setcb($bev, "echo_read_cb", NULL, "echo_event_cb", NULL);

    bufferevent_enable($bev, EVENT_READ | EVENT_WRITE);
}

function accept_error_cb($listener, $ctx) {
    $base = evconnlistener_get_base($listener);

    fprintf(STDERR, "Got an error %d (%s) on the listener. "
        ."Shutting down.\n",
		event_socket_get_last_errno(),
		event_socket_get_last_error());

    event_base_loopexit($base, NULL);
}


$port = 9876;

if ($argc > 1) {
    $port = (int) $argv[1];
}
if ($port <= 0 || $port > 65535) {
    puts("Invalid port");
    return 1;
}

$base = event_base_new();
if (!$base) {
    echo "Couldn't open event base";
	exit(1);
}

/*
$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
if (!socket_bind($socket, '0.0.0.0', $port)) {
	echo "Unable to bind socket\n";
	exit(1);
}
 */

$listener = evconnlistener_new_bind($base, "accept_conn_cb", /* data for callbacks */ NULL,
    EVENT_LEV_OPT_CLOSE_ON_FREE | EVENT_LEV_OPT_REUSEABLE, /* backlog */ -1, "0.0.0.0:$port");
if (!$listener) {
    echo "Couldn't create listener";
	exit(1);
}
evconnlistener_set_error_cb($listener, "accept_error_cb");

event_base_dispatch($base);
