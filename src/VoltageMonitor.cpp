/* 
* Copyright 2014 Friedemann Zenke
*
* This file is part of Auryn, a simulation package for plastic
* spiking neural networks.
* 
* Auryn is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* Auryn is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with Auryn.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "VoltageMonitor.h"

VoltageMonitor::VoltageMonitor(NeuronGroup * source, NeuronID id, string filename, AurynDouble stepsize) : Monitor(filename)
{
	init(source,id,filename,(AurynTime)(stepsize/dt));
}

VoltageMonitor::~VoltageMonitor()
{
}

void VoltageMonitor::init(NeuronGroup * source, NeuronID id, string filename, AurynTime stepsize)
{
	// only register if the neuron exists on this rank
	src = source;
	ssize = stepsize;
	nid = id;
	gid = src->rank2global(nid);
	paste_spikes = true;

	if ( nid < src->get_post_size() ) {
		sys->register_monitor(this);
		outfile << setiosflags(ios::fixed) << setprecision(6);
		outfile << "# Recording from neuron " << gid << "\n";
	}
}

void VoltageMonitor::propagate()
{
	if (active && (sys->get_clock())%ssize==0 && nid < src->get_size() ) {
		if ( paste_spikes && 
				std::find(src->get_spikes_immediate()->begin(), 
					      src->get_spikes_immediate()->end(), gid)!=src->get_spikes_immediate()->end() ) 
			outfile << (sys->get_time()) << " " << VOLTAGEMONITOR_PASTED_SPIKE_HEIGHT << "\n";
		else
			outfile << (sys->get_time()) << " " << src->get_mem(nid) << "\n";
	}
}
