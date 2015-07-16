/* 
* Copyright 2014-2015 Friedemann Zenke
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
*
* If you are using Auryn or parts of it for your work please cite:
* Zenke, F. and Gerstner, W., 2014. Limits to high-speed simulations 
* of spiking neural networks using general-purpose computers. 
* Front Neuroinform 8, 76. doi: 10.3389/fninf.2014.00076
*/

#ifndef SPIKEMONITOR_H_
#define SPIKEMONITOR_H_

#include "auryn_definitions.h"
#include "SpikingGroup.h"
#include "Monitor.h"
#include "System.h"
#include <fstream>

using namespace std;

/*! \brief The standard Monitor object to record spikes from a 
 * SpikingGroup and write them to a text file
 *
 * SpikeMonitor is specified with a source group of type SpikingGroup
 * and writes all or a specified range of the neurons spikes to a
 * text (ras) file that has to be given at construction time.
 * 
 */
class SpikeMonitor : Monitor
{
private:
    NeuronID n_from;
    NeuronID n_to;
    NeuronID n_every;
	SpikeContainer::const_iterator it;
	SpikingGroup * src;
	NeuronID offset;
	void init(SpikingGroup * source, string filename, NeuronID from, NeuronID to);
	void free();
	
public:
	/*! Switch variable to enable/disable recording. */
	bool active;

	/*! Default constructor
	 *
	 * \param source Specifies the source SpikingGroup to record from 
	 * \param filename Specifies the filename to write to. 
	 * This filename needs to be rank specific to avoid problems in parallelm mode
	 * */
	SpikeMonitor(SpikingGroup * source, string filename);

	/*! Default constructor which records from limited number of neurons
	 *
	 * \param source Specifies the source SpikingGroup to record from 
	 * \param filename Specifies the filename to write to. 
	 * This filename needs to be rank specific to avoid problems in parallelm mode
	 * \param to The last NeuronID to record from starting from 0.
	 * */
	SpikeMonitor(SpikingGroup * source, string filename, NeuronID to);

	/*! Default constructor which records from a range of neurons
	 *
	 * \param source Specifies the source SpikingGroup to record from 
	 * \param filename Specifies the filename to write to. 
	 * This filename needs to be rank specific to avoid problems in parallelm mode
	 * \param from The first NeuronID to record from.
	 * \param to The last NeuronID to record from.
	 * */
	SpikeMonitor(SpikingGroup * source, string filename, NeuronID from, NeuronID to);

	/*! Sets the offset added to each NeuronID. Can be used to disambiguate between differen NeuronGroups.
	 * \param of The (positive) offset */
	void set_offset(NeuronID of);

	/*! Sets every parameter that ellow to record only from every X neuron.
	 *
	 * \param the number X as described above. */
	void set_every(NeuronID every);

	/*! Default destructor. */
	virtual ~SpikeMonitor();

	/*! Propagate function for internal use. */
	void propagate();
};

#endif /*SPIKEMONITOR_H_*/
