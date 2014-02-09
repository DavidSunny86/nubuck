#include <Nubuck\nubuck.h>

struct Algorithm : IAlgorithm {
	IPhase* Init(const Nubuck& nb, const leda::GRAPH<leda::d3_rat_point, int>& G) {
        return NULL;
	}

	bool Run() { return false; }
};

int main(int argc, char* argv[]) {
    return RunNubuck(argc, argv, CreateAlgorithm<Algorithm>);
}