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
    "DeviceNetPrioritiesDevice",
    "BusLogging";

//
// The network definition of the Priorities scenario. There are 5 devices connected
// to a DeviceNet network.
//
module DeviceNetPriorities
    submodules:
        device_1: DeviceNetPrioritiesDevice;
            display: "p=55,104;i=block/ifcard";
        device_2: DeviceNetPrioritiesDevice;
            display: "p=110,104;i=block/ifcard";
        device_3: DeviceNetPrioritiesDevice;
            display: "p=165,104;i=block/ifcard";
        device_4: DeviceNetPrioritiesDevice;
            display: "p=220,104;i=block/ifcard";
        device_5: DeviceNetPrioritiesDevice;
            display: "p=275,104;i=block/ifcard";
        bus: Bus;
            gatesizes:
                in[5],
                out[5];
            display: "p=192,34;b=326,10;o=#408060";
        busLogging: BusLogging;
            display: "p=362,104;i=block/table2";
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
    display: "b=402,136";
endmodule

//
// The network definition of the Priorities_One scenario. There are 6 devices connected
// to a DeviceNet network.
//
module DeviceNetPriorities_One
    submodules:
        device_1: DeviceNetPrioritiesDevice;
            display: "p=55,104;i=block/ifcard";
        device_2: DeviceNetPrioritiesDevice;
            display: "p=110,104;i=block/ifcard";
        device_3: DeviceNetPrioritiesDevice;
            display: "p=165,104;i=block/ifcard";
        device_4: DeviceNetPrioritiesDevice;
            display: "p=220,104;i=block/ifcard";
        device_5: DeviceNetPrioritiesDevice;
            display: "p=275,104;i=block/ifcard";
        device_6: DeviceNetPrioritiesDevice;
            display: "p=330,104;i=block/ifcard";
        bus: Bus;
            gatesizes:
                in[6],
                out[6];
            display: "p=192,34;b=326,10;o=#408060";
        busLogging: BusLogging;
            display: "p=417,104;i=block/table2";
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
    display: "b=457,136";
endmodule

network deviceNetPriorities : DeviceNetPriorities
endnetwork

network deviceNetPriorities_One : DeviceNetPriorities_One
endnetwork
