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
// Ethernet client module for the 10-Nodes case.
//
// His activity is only controlled by the period range and the beginning time.
// Every instance produces message IDs equal their node numbers.
//
simple EthernetTenNodesClient
    parameters:
        begin: numeric, // First message sending offset
        period: numeric, // Period range
        length: numeric, // Length of data
        writeScalars: bool, // Scalars writing switch
        debug: bool; // Debug flag for detailed tracing output
    gates:
        in: in; // Receiving gate
        out: out; // Sending gate
endsimple
