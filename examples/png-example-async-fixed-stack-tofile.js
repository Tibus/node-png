var fs  = require('fs');
var sys = require('sys');
var Png = require('../build/Release/png').FixedPngStack;
var Buffer = require('buffer').Buffer;

// the rgba-terminal.dat file is 1152000 bytes long.
var rgba = fs.readFileSync('./rgba-terminal.dat');

console.time("timer");
let numSaved = 0;
let total = 200;
for (var i = 0; i < total; i++) {
    var png = new Png(1000, 1000, "rgba", 0, 0xFF);
    png.push(rgba, 50, 200, 720, 400);
    png.encodeAndSave('./png-async-fixed.png', function (error) {
        if (error) {
            console.log('Error: ' + error.toString());
            process.exit(1);
        }
        numSaved++;
        if (numSaved === total - 1) {
        }
    });
}
console.timeEnd("timer");