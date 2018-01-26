#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"

#include <map>

using namespace llvm;
using namespace std;

namespace {
	struct CountStaticInstructions : public FunctionPass {
		static char ID;
		CountStaticInstructions() : FunctionPass(ID) {}

		bool runOnFunction(Function &F) override {
			map<string, int> counter;
			// errs() << "Hello: ";
			// errs().write_escaped(F.getName()) << '\n';
			
			for (inst_iterator begin = inst_begin(F), end = inst_end(F); begin!=end; begin++) {
				++counter[begin->getOpcodeName()];
			}
			for (map<string,int>::iterator begin = counter.begin(), end = counter.end(); begin!=end; begin++) {
				errs() << begin->first << '\t' << begin->second << '\n';
			}

			return false;
		}
	}; // end of struct CountStaticInstructions
}  // end of anonymous namespace

char CountStaticInstructions::ID = 0;
static RegisterPass<CountStaticInstructions> X("cse231-csi", "Count Static Instructions",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);
