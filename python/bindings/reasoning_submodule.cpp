/***************************************************************************
 *            reasoning_submodule.cpp
 *
 *  Copyright  2026  Luca Geretti
 *
 ****************************************************************************/

/*
 *  This file is part of Ariadne Reasoning.
 *
 *  Ariadne Reasoning is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Ariadne Reasoning is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Ariadne Reasoning. If not, see <https://www.gnu.org/licenses/>.
 */

/** @file python/bindings/reasoning_submodule.cpp
 *  @brief Expose reachability analysers and paving orbits to Python.
 */

#include "evolution/hybrid_evolver.hpp"
#include "evolution/vector_field_evolver.hpp"
#include "hybrid/hybrid_augmented_paving.hpp"
#include "hybrid/hybrid_bounded_constraint_set.hpp"
#include "labelled/labelled_augmented_paving.hpp"
#include "paving/augmented_paving.hpp"
#include "pybind11.hpp"
#include "reasoning/hybrid_reachability_analyser.hpp"
#include "reasoning/neural_symbolic.hpp"
#include "reasoning/reachability_analyser.hpp"
#include "system/hybrid_automaton.hpp"

#include "function-utilities.hpp"

using namespace Ariadne;

Void export_augmented_paving(pybind11::module& module) {
    pybind11::class_<AugmentedPaving> storage_class(module, "AugmentedPaving");
    pybind11::class_<LabelledAugmentedPaving, pybind11::bases<AugmentedPaving>> labelled_storage_class(module, "LabelledAugmentedPaving");
}

template<class RA>
Void export_safety_certificate(pybind11::module& module, const char* name) {
    typedef typename RA::StorageType StorageType;
    typedef typename RA::StateSpaceType StateSpaceType;
    typedef SafetyCertificate<StateSpaceType> SafetyCertificateType;

    pybind11::class_<SafetyCertificateType> safety_certificate_class(module, name);
    safety_certificate_class.def(pybind11::init<ValidatedSierpinskian, StorageType, StorageType>());
    safety_certificate_class.def_readonly("is_safe", &SafetyCertificateType::is_safe);
    // Sierpinskian safety is three-valued: an indeterminate result is not
    // a proof.  Provide an explicit Python bool for certified safety.
    safety_certificate_class.def("proved_safe", [](const SafetyCertificateType& certificate) {
        return definitely(certificate.is_safe);
    });
    safety_certificate_class.def("chain_reach_size", [](const SafetyCertificateType& certificate) {
        return certificate.chain_reach_set.size();
    });
    safety_certificate_class.def_readonly("chain_reach_set", &SafetyCertificateType::chain_reach_set);
    safety_certificate_class.def_readonly("safe_set", &SafetyCertificateType::safe_set);
}

template<class RA, class... Params>
Void export_reachability_analyser(pybind11::module& module, const char* name) {
    typedef typename RA::ConfigurationType Configuration;
    typedef typename RA::StorageType StorageType;
    typedef typename RA::OvertSetInterfaceType OvertSetType;
    typedef typename RA::OpenSetInterfaceType OpenSetType;
    typedef typename RA::CompactSetInterfaceType CompactSetType;
    typedef typename RA::SafetyCertificateType SafetyCertificateType;
    typedef typename RA::TimeType TimeType;

    const auto& reference_internal = pybind11::return_value_policy::reference_internal;

    pybind11::class_<RA> reachability_analyser_class(module, name);
    reachability_analyser_class.def(pybind11::init<Params...>());
    reachability_analyser_class.def("configuration", (Configuration & (RA::*)()) & RA::configuration, reference_internal);
    reachability_analyser_class.def("evolver", &RA::evolver, reference_internal);
    reachability_analyser_class.def("lower_reach", (StorageType (RA::*)(const OvertSetType&, const TimeType&) const) & RA::lower_reach);
    reachability_analyser_class.def("upper_reach", (StorageType (RA::*)(const CompactSetType&, const TimeType&) const) & RA::upper_reach);
    reachability_analyser_class.def("outer_chain_reach", (StorageType (RA::*)(const CompactSetType&) const) & RA::outer_chain_reach);
    reachability_analyser_class.def("verify_safety", (SafetyCertificateType (RA::*)(const CompactSetType&, const OpenSetType&) const) & RA::verify_safety);

    pybind11::class_<Configuration> reachability_analyser_configuration_class(module, "ReachabilityAnalyserConfiguration");
    reachability_analyser_configuration_class.def("set_transient_time", &Configuration::set_transient_time);
    reachability_analyser_configuration_class.def("set_maximum_grid_fineness", &Configuration::set_maximum_grid_fineness);
    reachability_analyser_configuration_class.def("set_lock_to_grid_time", &Configuration::set_lock_to_grid_time);
    reachability_analyser_configuration_class.def("set_maximum_grid_extent", &Configuration::set_maximum_grid_extent);
    reachability_analyser_configuration_class.def("set_bounding_domain", &Configuration::set_bounding_domain);
    reachability_analyser_configuration_class.def("__repr__", &__cstr__<Configuration>);
}

Void export_hybrid_reasoning(pybind11::module& module) {
    using Analyser = HybridReachabilityAnalyser;
    using Configuration = HybridReachabilityAnalyserConfiguration;
    using EvolverConfiguration = HybridEvolverBaseConfiguration;

    const auto reference_internal = pybind11::return_value_policy::reference_internal;

    // The concrete hybrid sets implement both the compact initial-set and
    // open safe-set interfaces required by the analyser's C++ API.
    pybind11::class_<HybridBoundedConstraintSet> hybrid_set_class(module, "HybridBoundedConstraintSet");
    hybrid_set_class.def(pybind11::init<const DiscreteLocation&, const RealExpressionBoundedConstraintSet&>());
    hybrid_set_class.def("adjoin", &HybridBoundedConstraintSet::adjoin, reference_internal);
    hybrid_set_class.def("location", &HybridBoundedConstraintSet::location);
    hybrid_set_class.def("__str__", &__cstr__<HybridBoundedConstraintSet>);

    // Hybrid evolution was not previously constructible from the composed
    // Python module.  Register an actual evolver, not a type-only placeholder.
    pybind11::class_<GeneralHybridEvolver> evolver_class(module, "GeneralHybridEvolver");
    evolver_class.def(pybind11::init([](const HybridAutomaton& system) {
        return std::make_unique<GeneralHybridEvolver>(static_cast<const HybridAutomatonInterface&>(system));
    }));
    evolver_class.def("configuration", [](GeneralHybridEvolver& evolver) -> EvolverConfiguration& { return evolver.configuration(); }, reference_internal);
    evolver_class.def("set_integrator", [](GeneralHybridEvolver& evolver, const IntegratorInterface& integrator) {
        evolver.set_integrator(integrator);
    });
    evolver_class.def("__str__", &__cstr__<GeneralHybridEvolver>);

    pybind11::class_<EvolverConfiguration> evolver_configuration_class(module, "HybridEvolverConfiguration");
    evolver_configuration_class.def("set_flow_accuracy", &EvolverConfiguration::set_flow_accuracy);
    evolver_configuration_class.def("set_maximum_step_size", &EvolverConfiguration::set_maximum_step_size);
    evolver_configuration_class.def("set_maximum_enclosure_radius", &EvolverConfiguration::set_maximum_enclosure_radius);
    evolver_configuration_class.def("set_maximum_spacial_error", &EvolverConfiguration::set_maximum_spacial_error);
    evolver_configuration_class.def("set_enable_reconditioning", &EvolverConfiguration::set_enable_reconditioning);
    evolver_configuration_class.def("set_enable_subdivisions", &EvolverConfiguration::set_enable_subdivisions);
    evolver_configuration_class.def("__repr__", &__cstr__<EvolverConfiguration>);

    // The returned storage and certificate must be inspectable from Python.
    auto hybrid_augmented_paving_class = pybind11::reinterpret_borrow<pybind11::class_<HybridAugmentedPaving>>(
        module.attr("HybridAugmentedPaving"));
    hybrid_augmented_paving_class.def("size", &HybridAugmentedPaving::size);
    hybrid_augmented_paving_class.def("__len__", &HybridAugmentedPaving::size);
    hybrid_augmented_paving_class.def("is_empty", &HybridAugmentedPaving::is_empty);
    hybrid_augmented_paving_class.def("__str__", &__cstr__<HybridAugmentedPaving>);

    export_safety_certificate<Analyser>(module, "HybridSafetyCertificate");

    pybind11::class_<Analyser> analyser_class(module, "HybridReachabilityAnalyser");
    analyser_class.def(pybind11::init([](const GeneralHybridEvolver& evolver) {
        return std::make_unique<Analyser>(static_cast<const HybridEvolverInterface&>(evolver));
    }));
    analyser_class.def("configuration", [](Analyser& analyser) -> Configuration& { return analyser.configuration(); }, reference_internal);

    // Accept concrete hybrid sets rather than unregistered abstract interface
    // types.  Each call delegates directly to the inherited C++ algorithm.
    analyser_class.def("upper_evolve", [](const Analyser& a, const HybridBoundedConstraintSet& s, const HybridTime& t) {
        return a.upper_evolve(s, t);
    });
    analyser_class.def("upper_reach_evolve", [](const Analyser& a, const HybridBoundedConstraintSet& s, const HybridTime& t) {
        return a.upper_reach_evolve(s, t);
    });
    analyser_class.def("upper_reach", [](const Analyser& a, const HybridBoundedConstraintSet& s, const HybridTime& t) {
        return a.upper_reach(s, t);
    });
    analyser_class.def("lower_evolve", [](const Analyser& a, const HybridBoundedConstraintSet& s, const HybridTime& t) {
        return a.lower_evolve(s, t);
    });
    analyser_class.def("lower_reach_evolve", [](const Analyser& a, const HybridBoundedConstraintSet& s, const HybridTime& t) {
        return a.lower_reach_evolve(s, t);
    });
    analyser_class.def("lower_reach", [](const Analyser& a, const HybridBoundedConstraintSet& s, const HybridTime& t) {
        return a.lower_reach(s, t);
    });
    analyser_class.def("lower_reach", [](const Analyser& a, const HybridBoundedConstraintSet& s) {
        return a.lower_reach(s);
    });
    analyser_class.def("outer_chain_reach", [](const Analyser& a, const HybridBoundedConstraintSet& s) {
        return a.outer_chain_reach(s);
    });
    analyser_class.def("verify_safety", [](const Analyser& a, const HybridBoundedConstraintSet& initial,
                                           const HybridBoundedConstraintSet& safe) {
        return a.verify_safety(initial, safe);
    });

    pybind11::class_<Configuration> configuration_class(module, "HybridReachabilityAnalyserConfiguration");
    configuration_class.def("set_transient_time", &Configuration::set_transient_time);
    configuration_class.def("set_transient_steps", &Configuration::set_transient_steps);
    configuration_class.def("set_lock_to_grid_time", &Configuration::set_lock_to_grid_time);
    configuration_class.def("set_lock_to_grid_steps", &Configuration::set_lock_to_grid_steps);
    configuration_class.def("set_maximum_grid_fineness", &Configuration::set_maximum_grid_fineness);
    configuration_class.def("set_maximum_grid_extent", &Configuration::set_maximum_grid_extent);
    configuration_class.def("__repr__", &__cstr__<Configuration>);
}

void reasoning_submodule(pybind11::module& module) {
    module.def("make_expression", [](const NeuralParameterSnapshot& snapshot, const Space<Real>& space) { return make_expression(snapshot, space); }, pybind11::arg("snapshot"), pybind11::arg("space"));
    export_augmented_paving(module);
    pybind11::class_<HybridAugmentedPaving>(module, "HybridAugmentedPaving");
    export_hybrid_reasoning(module);
    export_safety_certificate<ContinuousReachabilityAnalyser>(module, "SafetyCertificate");
    export_reachability_analyser<ContinuousReachabilityAnalyser, VectorFieldEvolver>(module, "ContinuousReachabilityAnalyser");
}
