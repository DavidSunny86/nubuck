#include <Nubuck\nubuck.h>

class MyAlgorithm : public IAlgorithm {
public:
    IPhase* Init(const Nubuck& nubuck, const leda::GRAPH<leda::d3_rat_point, int>& G) {
        return NULL;
    }

    bool Run(void) { return false; /* do nothing special */ }
};

int main(int argc, char* argv[]) {
    return RunNubuck(argc, argv, CreateAlgorithm<MyAlgorithm>);
}