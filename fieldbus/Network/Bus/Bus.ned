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
// A generic bus model.
//
// in[i], out[i] gate pairs represent taps. Messages arriving on a tap
// travel on the bus on both directions, and copies of it are sent out
// on every other tap after delays proportional to their distances.
//
// Tap connections should have zero delays and zero data rates!
//
// Messages are not interpreted by the bus model in any way. Messages
// may represent anything, from the beginning of a frame transmission to
// end (or abortion) of transmission. Please note parameter dependencies.
//
simple Bus
    parameters:
        accuracyFP : numeric,
        unsetPosition : bool, // No position value are considered only the
                                     // the distance that is the same between every tap
        distance : numeric, // The distance
        positions : string,  // Positions of taps on the cable in meters, as a sequence
                             // of numbers separated by spaces. If there are too
                             // few values, the distance between the last two positions
                             // is repeated, or 5 meters is used.
        propagationSpeed : numeric,
                             // Signal propagation speed on the bus, in m/s
                             // (typically 200,000 m/s)
        bitrate : numeric, // The bitrate on the bus, in bit/s
        protocol : numeric, // The mac protocol running on the bus
        biterrors : bool, // Enable/disable biterrors on the bus
        biterrorRate : numeric, // The biterror rate for one bit
        writeScalars : bool,  // Enable/disable recording statistics in omnetpp.sca
        debug : bool, // Debug option for detailed output in the trace<br><br>note: the following values are for logging/measurement with valid values: -1 = off, 0 = only vector/statistics, >0 = vector/statistics with period >0<br>
        messagesVector: numeric, // Messages vector settings
        messagesStatistics: numeric; // Messages statistics settings
    gates:
        in: in[];    // To stations; each in[k], out[k] pair represents a tap
        out: out[];  // To stations; each in[k], out[k] pair represents a tap
endsimple
