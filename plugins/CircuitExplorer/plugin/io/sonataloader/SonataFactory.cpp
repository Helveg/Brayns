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

#include "SonataFactory.h"

#include <plugin/io/morphology/neuron/builders/PrimitiveNeuronBuilder.h>
#include <plugin/io/morphology/neuron/builders/SampleNeuronBuilder.h>
#include <plugin/io/morphology/neuron/builders/SDFNeuronBuilder.h>

#include <plugin/io/sonataloader/populations/edges/ChemicalSynapsePopulationLoader.h>
#include <plugin/io/sonataloader/populations/edges/ElectricalSynapsePopulationLoader.h>
#include <plugin/io/sonataloader/populations/edges/EndFootPopulationLoader.h>
#include <plugin/io/sonataloader/populations/edges/GlialGlialPopulationLoader.h>
#include <plugin/io/sonataloader/populations/edges/SynapseAstrocytePopulationLoader.h>
#include <plugin/io/sonataloader/populations/nodes/AstrocytePopulationLoader.h>
#include <plugin/io/sonataloader/populations/nodes/BiophysicalPopulationLoader.h>
#include <plugin/io/sonataloader/populations/nodes/VasculaturePopulationLoader.h>

#include <plugin/io/sonataloader/simulations/reports/EdgeCompartmentLoader.h>
#include <plugin/io/sonataloader/simulations/reports/NodeCompartmentLoader.h>
#include <plugin/io/sonataloader/simulations/reports/NodeSpikeLoader.h>
#include <plugin/io/sonataloader/simulations/reports/VasculatureReportLoader.h>

namespace sonataloader
{
SonataFactories::SonataFactories()
{
    _neuronBuilders.registerProduct<PrimitiveNeuronBuilder>(NeuronGeometryType::VANILLA);
    _neuronBuilders.registerProduct<SampleNeuronBuilder>(NeuronGeometryType::SAMPLES);
    _neuronBuilders.registerProduct<SDFNeuronBuilder>(NeuronGeometryType::SMOOTH);

    _edgeLoaders.registerProduct<ChemicalSynapsePopulation>("chemical");
    _edgeLoaders.registerProduct<ElectricalSynapsePopulation>("electrical_synapse");
    _edgeLoaders.registerProduct<EndFootPopulationLoader>("endfoot");
    _edgeLoaders.registerProduct<GlialGlialPopulationLoader>("glialglial");
    _edgeLoaders.registerProduct<SynapseAstrocytePopulationLoader>("synapse_astrocyte");

    _nodeLoaders.registerProduct<AstrocytePopulationLoader>("astrocyte");
    _nodeLoaders.registerProduct<BiophysicalPopulationLoader>("biophysical");
    _nodeLoaders.registerProduct<VasculaturePopulationLoader>("vasculature");

    _simulations.registerProduct<NodeCompartmentLoader>(SimulationType::COMPARTMENT);
    _simulations.registerProduct<NodeSpikeLoader>(SimulationType::SPIKES);
    _simulations.registerProduct<NodeCompartmentLoader>(SimulationType::SUMMATION);
    _simulations.registerProduct<VasculatureReportLoader>(SimulationType::BLOODFLOW_PRESSURE);
    _simulations.registerProduct<VasculatureReportLoader>(SimulationType::BLOODFLOW_SPEED);
    _simulations.registerProduct<VasculatureRadiiReportLoader>(SimulationType::BLOODFLOW_RADII);
}
}
