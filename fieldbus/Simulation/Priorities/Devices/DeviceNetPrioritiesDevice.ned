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
    "DeviceNetPrioritiesClient",
    "DeviceNetMAC";

//
// The DeviceNet device definition for the Priorities scenario.
//
module DeviceNetPrioritiesDevice
    gates:
        in: in;
        out: out;
    submodules:
        client: DeviceNetPrioritiesClient;
            display: "p=100,60,col";
        mac: DeviceNetMAC;
            display: "p=100,150;i=block/queue;q=queue";
        devicenetMACLogging: DeviceNetMACLogging;
            display: "p=228,104;i=block/table2";
    connections nocheck:
        mac.UpperLayerIn <-- client.out;
        mac.UpperLayerOut --> client.in;
        mac.LowerLayerIn <-- in display "m=s";
        mac.LowerLayerOut --> out display "m=s";
    display: "b=300,216";
endmodule
