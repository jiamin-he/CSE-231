#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instruction.h"

#include <map>

using namespace llvm;
using namespace std;

namespace {
    struct CountDynamicInstructions : public FunctionPass {
        static char ID;
        CountDynamicInstructions() : FunctionPass(ID) {}

        bool runOnFunction(Function &F) override {
            Module *M = F.getParent();
            LLVMContext &context = M->getContext();

            // looking up functions
            Function *update = cast<Function>(M->getOrInsertFunction("updateInstrInfo", 
                Type::getVoidTy(context),Type::getInt32Ty(context),Type::getInt32PtrTy(context),Type::getInt32PtrTy(context)));
            Function *print = cast<Function>(M->getOrInsertFunction("printOutInstrInfo", Type::getVoidTy(context)));
            
            // B is a pointer to the basic block
            for(Function::iterator B = F.begin(), BE = F.end(); B!=BE; ++B) {
                map<int, int> counter;
                
                // I is a pointer to instruction
                for(BasicBlock::iterator I = B->begin(), IE = B->end(); I != IE; ++I) {
                    ++counter[I->getOpcode()];
                }

                IRBuilder<> Builder(&*B);
                Builder.SetInsertPoint(B->getTerminator());

                vector<Value*> args;
                vector<Constant*> keys;
                vector<Constant*> values;

                // prepare arguments for a function call
                Constant *nums = ConstantInt::get(IntegerType::get(context,32),counter.size());
                args.push_back(nums);

                for(map<int,int>::iterator it = counter.begin(), it_e = counter.end(); it!=it_e; ++it) {
                    keys.push_back(ConstantInt::get(IntegerType::get(context,32),it->first));
                    values.push_back(ConstantInt::get(IntegerType::get(context,32),it->second));
                }

                // LLVM representation of the type of an array with counter.size() elements
                ArrayType *arrayTy = ArrayType::get(IntegerType::get(context,32),counter.size());
                GlobalVariable *K = new GlobalVariable(*M, arrayTy, true, GlobalVariable::InternalLinkage,ConstantArray::get(arrayTy,keys), "key_global");
                GlobalVariable *V = new GlobalVariable(*M, arrayTy, true, GlobalVariable::InternalLinkage,ConstantArray::get(arrayTy,values), "value_global");

                // obtain a reference to the above array
                Value* idx_list[2] = {ConstantInt::get(IntegerType::get(context,32),0), ConstantInt::get(IntegerType::get(context,32),0)};
                args.push_back(Builder.CreateInBoundsGEP(K,idx_list));
                args.push_back(Builder.CreateInBoundsGEP(V,idx_list));
                // create a call
                Builder.CreateCall(update,args);  

                // I is the pointer to each instruction
                for(BasicBlock::iterator I = B->begin(), IE = B->end(); I != IE; ++I) {
                    if((string)I->getOpcodeName() == "ret") {
                        Builder.SetInsertPoint(&*I);
                        Builder.CreateCall(print);
                    }
                }

            }
            // errs() << "Hello: ";
            // errs().write_escaped(F.getName()) << '\n';

            return false;
        }
    }; // end of struct CountDynamicInstructions
}  // end of anonymous namespace

char CountDynamicInstructions::ID = 0;
static RegisterPass<CountDynamicInstructions> X("cse231-cdi", "Count Static Instructions",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);
