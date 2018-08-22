var fs  = require('fs');
var sys = require('sys');
var Png = require('../build/Release/png').FixedPngStack;
var Buffer = require('buffer').Buffer;

// the rgba-terminal.dat file is 1152000 bytes long.
var rgba = fs.readFileSync('./rgba-terminal.dat');

var png = new Png(1000, 1000, "rgba", 0, 0xFF);
png.push(rgba, 50, 200, 720, 400);
png.encode(function (data, error) {
    if (error) {
        console.log('Error: ' + error.toString());
        process.exit(1);
    }
    fs.writeFileSync('./png-async-fixed.png', data.toString('binary'), 'binary');
});