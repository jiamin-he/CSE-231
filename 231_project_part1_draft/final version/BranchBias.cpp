#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"

#include <map>

using namespace llvm;
using namespace std;

namespace {
    struct BranchBias : public FunctionPass {
        static char ID;
        BranchBias() : FunctionPass(ID) {}

        bool runOnFunction(Function &F) override {
            Module *M = F.getParent();
            LLVMContext &context = M->getContext();

            Function *updateBranch = cast<Function>(M->getOrInsertFunction("updateBranchInfo", 
                Type::getVoidTy(context),Type::getInt1Ty(context)));
            Function *printOutBranch = cast<Function>(M->getOrInsertFunction("printOutBranchInfo", Type::getVoidTy(context)));

            // B is a pointer to a basic block.
            for(Function::iterator B = F.begin(), BE = F.end(); B != BE; ++B) {
                // Branch must be at the end of a basic block. So getTerminator to insert it.
                IRBuilder<> Builder(&*B);
                Builder.SetInsertPoint(B->getTerminator());
                // cast to branchInst
                // dynamic cast, if cannot cast, is null
                BranchInst *branch_inst = dyn_cast<BranchInst>(B->getTerminator());
                
                // for some branch instruction, only comparision branch instruction is conditional, valid here
                if(branch_inst != NULL && branch_inst->isConditional()) {
                    vector<Value*> args;
                    args.push_back(branch_inst->getCondition());
                    Builder.CreateCall(updateBranch,args);
                }

                // I is a pointer to an instruction.
                for(BasicBlock::iterator I = B->begin(), IE = B->end(); I != IE; ++I) {
                    //instruction should be inserted to instructions
                    //when =="ret", means it's the end, insert the print function
                    if((string)I->getOpcodeName() == "ret") {
                        Builder.SetInsertPoint(&*I);
                        Builder.CreateCall(printOutBranch);
                    }
                }
            }

            // errs() << "Hello: ";
            // errs().write_escaped(F.getName()) << '\n';
            return false;
        }
    }; // end of struct BranchBias
}  // end of anonymous namespace

char BranchBias::ID = 0;
static RegisterPass<BranchBias> X("cse231-bb", "Count Branch Bias",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);
