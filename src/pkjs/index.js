var Clay = require('pebble-clay');
var config = require('./config.json');
var customClay = require('./custom-clay');
var clay = new Clay(config, customClay);

Pebble.addEventListener('ready', function() {
    Pebble.sendAppMessage({ 'APP_READY' : 1 });
});
