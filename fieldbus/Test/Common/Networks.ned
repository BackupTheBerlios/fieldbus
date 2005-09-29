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

//
// Different test networks.
//

import
    "Bus",
    "BusClient",
    "EthernetHost",
    "BusLogging";

module FourOnBus
    submodules:
        host_1: BusClient;
            display: "p=68,104;i=device/pc2";
        host_2: BusClient;
            display: "p=116,104;i=device/pc2";
        host_3: BusClient;
            display: "p=224,104;i=device/pc2";
        host_4: BusClient;
            display: "p=520,104;i=device/pc2";
        bus: Bus;
            parameters:
                positions = "0 200 600 1800",
                propagationSpeed = 200000000,
                bitrate = 10000000,
                protocol = 0;
            gatesizes:
                in[4],
                out[4];
            display: "p=294,34;b=504,10;o=#408060";
    connections:
        bus.in[0] <-- host_1.out;
        bus.out[0] --> host_1.in;
        bus.in[1] <-- host_2.out;
        bus.out[1] --> host_2.in;
        bus.in[2] <-- host_3.out;
        bus.out[2] --> host_3.in;
        bus.in[3] <-- host_4.out;
        bus.out[3] --> host_4.in;
    display: "b=568,136";
endmodule

network fourOnBus : FourOnBus
endnetwork

module FourOnEthernet
    submodules:
        host_1: EthernetHost;
            display: "p=68,104;i=block/ifcard";
        host_2: EthernetHost;
            display: "p=116,104;i=block/ifcard";
        host_3: EthernetHost;
            display: "p=224,104;i=block/ifcard";
        host_4: EthernetHost;
            display: "p=520,104;i=block/ifcard";
        bus: Bus;
            parameters:
                positions = "0 200 600 1800",
                propagationSpeed = 200000000,
                protocol = 0;
            gatesizes:
                in[4],
                out[4];
            display: "p=294,34;b=504,10;o=#408060";
        busLogging: BusLogging;
            display: "p=372,104;i=block/table2";
    connections nocheck:
        bus.in[0] <-- host_1.out;
        bus.out[0] --> host_1.in;
        bus.in[1] <-- host_2.out;
        bus.out[1] --> host_2.in;
        bus.in[2] <-- host_3.out;
        bus.out[2] --> host_3.in;
        bus.in[3] <-- host_4.out;
        bus.out[3] --> host_4.in;
    display: "b=568,136";
endmodule

network fourOnEthernet : FourOnEthernet
endnetwork

module FourOnDeviceNet
    submodules:
        host_1: DeviceNetHost;
            display: "p=68,104;i=block/ifcard";
        host_2: DeviceNetHost;
            display: "p=116,104;i=block/ifcard";
        host_3: DeviceNetHost;
            display: "p=224,104;i=block/ifcard";
        host_4: DeviceNetHost;
            display: "p=520,104;i=block/ifcard";
        bus: Bus;
            parameters:
                positions = "0 200 600 1800",
                propagationSpeed = 200000000,
                protocol = 1;
            gatesizes:
                in[4],
                out[4];
            display: "p=294,34;b=504,10;o=#408060";
        busLogging: BusLogging;
            display: "p=372,104;i=block/table2";
    connections nocheck:
        bus.in[0] <-- host_1.out;
        bus.out[0] --> host_1.in;
        bus.in[1] <-- host_2.out;
        bus.out[1] --> host_2.in;
        bus.in[2] <-- host_3.out;
        bus.out[2] --> host_3.in;
        bus.in[3] <-- host_4.out;
        bus.out[3] --> host_4.in;
    display: "b=568,136";
endmodule

network fourOnDeviceNet : FourOnDeviceNet
endnetwork

module FourOnControlNet
    submodules:
        host_1: ControlNetHost;
            display: "p=68,104;i=block/ifcard";
        host_2: ControlNetHost;
            display: "p=116,104;i=block/ifcard";
        host_3: ControlNetHost;
            display: "p=224,104;i=block/ifcard";
        host_4: ControlNetHost;
            display: "p=520,104;i=block/ifcard";
        bus: Bus;
            parameters:
                positions = "0 200 600 1800",
                propagationSpeed = 200000000,
                protocol = 2;
            gatesizes:
                in[4],
                out[4];
            display: "p=294,34;b=504,10;o=#408060";
        busLogging: BusLogging;
            display: "p=372,104;i=block/table2";
    connections nocheck:
        bus.in[0] <-- host_1.out;
        bus.out[0] --> host_1.in;
        bus.in[1] <-- host_2.out;
        bus.out[1] --> host_2.in;
        bus.in[2] <-- host_3.out;
        bus.out[2] --> host_3.in;
        bus.in[3] <-- host_4.out;
        bus.out[3] --> host_4.in;
    display: "b=568,136";
endmodule

network fourOnControlNet : FourOnControlNet
endnetwork
