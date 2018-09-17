Modules implements Two factor authentication using Twilio SMS servis.

You have to use composer to fetch Twilio PHP 5.x lib

https://www.twilio.com/docs/libraries/php

Expected lib location is
_WEB_ROOT_/vendor/twilio

There is a module settings page where LIB path can be configured if needed.

SETTINGS

Add this line in settings.php
--------------------------------

$conf['drupal_http_request_fails'] = FALSE;
