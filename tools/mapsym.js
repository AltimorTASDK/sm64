/*

Linker map to PJ64 symbol table converter for nodejs

Usage:
    node mapsym.js <input_map> <output_sym>

Eg:
    node mapsym.js "sm64.map" "Project64/Save/SUPER MARIO 64.sym"

*/

const fs = require('fs');

var inputPath = process.argv[2];
var outputPath = process.argv[3];

var map = fs.readFileSync(inputPath).toString();
var matches = map.match(/0x0{8}[0-9A-Fa-f]+[ \t]+[a-zA-Z_][a-zA-Z_\d]+/g);

var output = '';

for(var i = 0; i < matches.length; i++)
{
    var pair = matches[i].split(/\s+/);
    var address = parseInt(pair[0]).toString(16).toUpperCase();
    var symbol = pair[1];

    output += address + ',code,' + symbol + '\n';
}

fs.writeFileSync(outputPath, output);