//
// Copyright (C) 2005 Lehrstuhl fuer praktische Informatik, Universitaet Dortmund
// Copyright (C) 2005 Dragan Isakovic
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

import
    "Bus",
    "DeviceNetTenNodesDevice",
    "BusLogging";

//
// The network definition of the 10-Node scenario. There are 10 devices connected
// to a DeviceNet network.
//
module DeviceNetTenNodes
    submodules:
        device_1: DeviceNetTenNodesDevice;
            display: "p=55,104;i=block/ifcard";
        device_2: DeviceNetTenNodesDevice;
            display: "p=110,104;i=block/ifcard";
        device_3: DeviceNetTenNodesDevice;
            display: "p=165,104;i=block/ifcard";
        device_4: DeviceNetTenNodesDevice;
            display: "p=220,104;i=block/ifcard";
        device_5: DeviceNetTenNodesDevice;
            display: "p=275,104;i=block/ifcard";
        device_6: DeviceNetTenNodesDevice;
            display: "p=330,104;i=block/ifcard";
        device_7: DeviceNetTenNodesDevice;
            display: "p=385,104;i=block/ifcard";
        device_8: DeviceNetTenNodesDevice;
            display: "p=440,104;i=block/ifcard";
        device_9: DeviceNetTenNodesDevice;
            display: "p=495,104;i=block/ifcard";
        device_10: DeviceNetTenNodesDevice;
            display: "p=550,104;i=block/ifcard";
        bus: Bus;
            gatesizes:
                in[10],
                out[10];
            display: "p=302,34;b=556,10;o=#408060";
        busLogging: BusLogging;
            display: "p=640,104;i=block/table2";
    connections nocheck:
        bus.in[0] <-- device_1.out;
        bus.out[0] --> device_1.in;
        bus.in[1] <-- device_2.out;
        bus.out[1] --> device_2.in;
        bus.in[2] <-- device_3.out;
        bus.out[2] --> device_3.in;
        bus.in[3] <-- device_4.out;
        bus.out[3] --> device_4.in;
        bus.in[4] <-- device_5.out;
        bus.out[4] --> device_5.in;
        bus.in[5] <-- device_6.out;
        bus.out[5] --> device_6.in;
        bus.in[6] <-- device_7.out;
        bus.out[6] --> device_7.in;
        bus.in[7] <-- device_8.out;
        bus.out[7] --> device_8.in;
        bus.in[8] <-- device_9.out;
        bus.out[8] --> device_9.in;
        bus.in[9] <-- device_10.out;
        bus.out[9] --> device_10.in;
    display: "b=680,136";
endmodule

network deviceNetTenNodes : DeviceNetTenNodes
endnetwork

