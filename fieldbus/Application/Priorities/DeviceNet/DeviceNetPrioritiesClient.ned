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
// DeviceNet client module for the Priorities case.
//
// The arriving time of a message, the priority and the length of data
// are choosed from distributions given in the configuration. The priority
// indicator defines a message as a prior or a non-prior one. The
// messages are only DATA messages.
//
simple DeviceNetPrioritiesClient
    parameters:
        arriving: numeric, // Arriving time of a message
        priority: numeric, // Priority indicator
        length: numeric, // Length of data
        onePrior: numeric, // Prior sending node
        accuracyFP: numeric, // Floating point accuracy
        writeScalars: bool, // Scalars writing switch
        debug: bool, // Debug flag for detailed tracing output
        fileName: string; // Name of statstics file
    gates:
        in: in; // Receiving gate
        out: out; // Sending gate
endsimple
