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

cplusplus {{
#include "Frame_m.h"
}};

class Frame;

//
// Ethernet frame class. The preamble and the SOF are included into the frame.
//
message EthernetFrame extends Frame
{
    fields:
        int preamble; // The preamble
        int SOF; // Start of frame value
        int srcAddress; // The source address
        int dstAddress; // The destination address
        int type; // Type of the frame
        int pad; // Number of pad bits
        int CRC; // The check value
}
