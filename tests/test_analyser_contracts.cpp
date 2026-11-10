#include "ariadne-reasoning.hpp"
#include "ariadne-testing.hpp"
#include "paving/robdd_paving.hpp"
#include <memory>
#include <type_traits>

using namespace Ariadne;

template<class SYS> struct ContractAnalyser : ReachabilityAnalyser<SYS> {
    using Base=ReachabilityAnalyser<SYS>;
    using Base::Base;
    using Base::_checked_restriction;
    auto storage() const {
        return this->_storage_factory(this->configuration().grid(),this->system());
    }
};

class TestAnalyserContracts : public Testing::ContextFixture {
public:
    using ContextFixture::ContextFixture;

    template<class SYS> Void check(ContractAnalyser<SYS>& analyser) {
        auto manager=std::make_shared<RobddManager>();
        ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument,analyser.set_paving_factory({}));
        analyser.set_paving_factory([manager](Grid const& grid) {
            return Paving(new RobddPaving(manager,grid));
        });
        auto input=analyser.storage();
        auto bounding=analyser.storage();
        auto empty=analyser.storage();
        auto& input_state=input.state_paving();
        auto& bounding_state=bounding.state_paving();
        Grid grid(1u);
        GridCell root(grid,0u,BinaryWord{});
        if constexpr(std::is_same_v<SYS,HybridAutomatonInterface>) {
            DiscreteLocation q;
            input_state.adjoin(HybridGridCell(q,analyser.system().continuous_state_space(q),root));
            bounding_state.adjoin(HybridGridCell(q,analyser.system().continuous_state_space(q),root.split(false)));
        } else {
            input_state.adjoin(root);
            bounding_state.adjoin(root.split(false));
        }
        for(auto policy:{ChainOverspillPolicy::IGNORE,ChainOverspillPolicy::WARNING,
                         ChainOverspillPolicy::ERROR}) {
            auto unchanged=input;
            analyser._checked_restriction(unchanged,empty,policy);
            ARIADNE_TESTING_PROVE(subset(input,unchanged));
            auto restricted=input;
            if(policy==ChainOverspillPolicy::ERROR) {
                ARIADNE_TESTING_EXPECT_EXCEPTION(analyser._checked_restriction(restricted,bounding,policy));
            } else {
                analyser._checked_restriction(restricted,bounding,policy);
            }
            ARIADNE_TESTING_PROVE(subset(restricted,bounding));
            ARIADNE_TESTING_PROVE(subset(bounding,restricted));
            ARIADNE_TESTING_PROVE(not subset(input,restricted));
        }
    }

    Void test_continuous_and_discrete_restriction() {
        RealVariable x("x");
        VectorField flow(List<DottedRealAssignment>{dot(x)=-x});
        GradedTaylorSeriesIntegrator integrator(StepMaximumError(1e-3_pr));
        VectorFieldEvolver flow_evolver(flow,integrator);
        ContractAnalyser<VectorField> continuous(flow_evolver);
        check(continuous);
        IteratedMap map(List<PrimedRealAssignment>{next(x)=x/2});
        IteratedMapEvolver map_evolver(map);
        ContractAnalyser<IteratedMap> discrete(map_evolver);
        check(discrete);
    }

    Void test_hybrid_restriction() {
        RealVariable x("x");
        DiscreteLocation q;
        HybridAutomaton system;
        system.new_mode(q,{dot(x)=-x});
        GeneralHybridEvolver evolver(system);
        ContractAnalyser<HybridAutomatonInterface> analyser(evolver);
        check(analyser);
    }
};

int main(int argc,char** argv) {
    ARIADNE_TESTING_SUITE("test_analyser_contracts");
    ARIADNE_TESTING_ADD_TEST(TestAnalyserContracts,test_continuous_and_discrete_restriction);
    ARIADNE_TESTING_ADD_TEST(TestAnalyserContracts,test_hybrid_restriction);
    return ARIADNE_TESTING_RUN_TESTS_WITH_ARGS(argc,argv);
}
