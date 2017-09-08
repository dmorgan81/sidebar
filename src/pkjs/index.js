var liveconfig = require('./liveconfig');

var Clay = require('pebble-clay');
var config = require('./config.json');
var customClay = require('./custom-clay');
var clay = new Clay(config, customClay, { autoHandleEvents: false });

var GenericWeather = require('pebble-generic-weather');
var genericWeather = new GenericWeather();

var GeocodeMapquest = require('pebble-geocode-mapquest');
var geocodeMapquest = new GeocodeMapquest();

Pebble.addEventListener('appmessage', function(e) {
    genericWeather.appMessageHandler(e);
    geocodeMapquest.appMessageHandler(e);
});

Pebble.addEventListener('showConfiguration', function(e) {
    var settings = JSON.parse(localStorage.getItem('clay-settings')) || {};
    liveconfig.connect(Pebble.getAccountToken(), function(msg) {
        settings[msg.id] = msg.value;
        Pebble.sendAppMessage(Clay.prepareSettingsForAppMessage(settings));
    });

    Pebble.openURL(clay.generateUrl());
});

Pebble.addEventListener('webviewclosed', function(e) {
    if (e && !e.response) return;

    Pebble.sendAppMessage(clay.getSettings(e.response), function(e) {
        liveconfig.close();
    }, function() {
        liveconfig.close();
    });
});

Pebble.addEventListener('ready', function() {
    Pebble.sendAppMessage({ 'APP_READY' : 1 });
});
