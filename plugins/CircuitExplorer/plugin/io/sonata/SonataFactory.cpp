/* Copyright (c) 2015-2021, EPFL/Blue Brain Project
 * All rights reserved. Do not distribute without permission.
 * Responsible Author: Nadir Roman <nadir.romanguerrero@epfl.ch>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <plugin/io/sonata/SonataFactory.h>

#include <plugin/io/sonata/morphology/neuron/builders/PrimitiveNeuronBuilder.h>
#include <plugin/io/sonata/morphology/neuron/builders/SampleNeuronBuilder.h>
#include <plugin/io/sonata/morphology/neuron/builders/SDFNeuronBuilder.h>

#include <plugin/io/sonata/populations/edges/ChemicalSynapsePopulationLoader.h>
#include <plugin/io/sonata/populations/edges/ElectricalSynapsePopulationLoader.h>
#include <plugin/io/sonata/populations/edges/EndFootPopulationLoader.h>
#include <plugin/io/sonata/populations/edges/GlialGlialPopulationLoader.h>
#include <plugin/io/sonata/populations/edges/SynapseAstrocytePopulationLoader.h>
#include <plugin/io/sonata/populations/nodes/AstrocytePopulationLoader.h>
#include <plugin/io/sonata/populations/nodes/BiophysicalPopulationLoader.h>
#include <plugin/io/sonata/populations/nodes/VasculaturePopulationLoader.h>

#include <plugin/io/sonata/simulations/reports/NodeCompartmentLoader.h>
#include <plugin/io/sonata/simulations/reports/NodeSpikeLoader.h>

SonataFactories::SonataFactories()
{
    _neuronBuilders.registerProduct<PrimitiveNeuronBuilder>("vanilla");
    _neuronBuilders.registerProduct<SampleNeuronBuilder>("samples");
    _neuronBuilders.registerProduct<SDFNeuronBuilder>("smooth");

    _edgeLoaders.registerProduct<ChemicalSynapsePopulation>("chemical");
    _edgeLoaders.registerProduct<ElectricalSynapsePopulation>("electrical_synapse");
    _edgeLoaders.registerProduct<EndFootPopulationLoader>("endfoot");
    _edgeLoaders.registerProduct<GlialGlialPopulationLoader>("glialglial");
    _edgeLoaders.registerProduct<SynapseAstrocytePopulationLoader>("synapse_astrocyte");

    _nodeLoaders.registerProduct<AstrocytePopulationLoader>("astrocyte");
    _nodeLoaders.registerProduct<BiophysicalPopualtionLoader>("biophysical");
    _nodeLoaders.registerProduct<VasculaturePopulationLoader>("vasculature");

    _simulations.registerProduct<NodeCompartmentLoader>(SimulationType::REPORT);
    _simulations.registerProduct<NodeSpikeLoader>(SimulationType::SPIKES);
}
