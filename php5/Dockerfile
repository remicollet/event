FROM php:5.6-cli

RUN apt-get update && apt-get install -y libssl1.1 libssl-dev libevent-dev
RUN docker-php-ext-install sockets posix pcntl \
    && docker-php-ext-enable --ini-name 0-sockets.ini sockets \
    && docker-php-ext-enable posix pcntl

RUN mkdir -p /usr/src/pecl-event
WORKDIR /usr/src/pecl-event

COPY . .

RUN ./rebuild.sh

ENV NO_INTERACTION=1
CMD ./run-tests.sh

# vim: ft=dockerfile
