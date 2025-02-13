var addon = require('bindings')('readFsdb');

// console.log(addon.hello()); // 'world'
console.log(addon.readFsdb());

export type NetlistId = number;
export type SignalId  = number;

class NetlistItem {
  
    constructor(
      public readonly label:      string,
      public readonly type:       string,
      public readonly encoding:   string,
      public readonly width:      number,
      public readonly signalId:   SignalId, // Signal-specific information
      public readonly netlistId:  NetlistId, // Netlist-specific information
      public readonly name:       string,
      public readonly modulePath: string,
      public readonly msb:        number,
      public readonly lsb:        number,
      public children:         NetlistItem[] = [],
    ) {

    }
}