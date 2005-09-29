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
// DeviceNet frame class. SOF is included into the frame, ACK and EOF are excluded
// but anyway handled in the module.
//
message DeviceNetFrame extends Frame
{
    fields:
        int SOF; // The start of frame value
        int arbitration; // Arbitration value
        int RTR; // Remote transmission request bit
        int control; // Control field
        int data; // Data bits
        int CRC; // The check value
}
