#include "parser.hpp"
#include "ast.hpp"
#include <alloca.h>
#include <iostream>
#include <llvm-c/Core.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include </usr/include/llvm/IR/TypedPointerType.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>     
#include <llvm/IR/Module.h>        
#include <llvm/IR/Constants.h>      
#include <llvm/Support/raw_ostream.h> 
#include <memory>


llvm::Type* LLVMCodeGenVisitor::getLLVMTypeFromASTType(const std::string& type) {
    if (type == "i8")       return llvm::Type::getInt8Ty(*context);
    else if (type == "i16") return llvm::Type::getInt16Ty(*context);
    else if (type == "i32") return llvm::Type::getInt32Ty(*context);
    else if (type == "i64") return llvm::Type::getInt64Ty(*context);
    else if (type == "u8")  return llvm::Type::getInt8PtrTy(*context, true);
    else if (type == "u16")  return llvm::Type::getInt16PtrTy(*context, true);
    else if (type == "u32") return llvm::Type::getInt32PtrTy(*context, true);
    else if (type == "u64")  return llvm::Type::getInt64PtrTy(*context, true);
    else if (type == "f16") return llvm::Type::getHalfTy(*context);
    else if (type == "f32") return llvm::Type::getFloatTy(*context);
    else if (type == "f64") return llvm::Type::getDoubleTy(*context);
    else if (type == "void") return llvm::Type::getVoidTy(*context);
    else if (type == "string") return llvm::Type::getInt8PtrTy(*context);
    return nullptr;
}

llvm::Type* LLVMCodeGenVisitor::getLLVMType(const std::string &type) {
    if (type == "i8")       return llvm::Type::getInt8Ty(*context);
    else if (type == "i16") return llvm::Type::getInt16Ty(*context);
    else if (type == "i32") return llvm::Type::getInt32Ty(*context);
    else if (type == "i64") return llvm::Type::getInt64Ty(*context);
    else if (type == "u8")  return llvm::Type::getInt8PtrTy(*context, true);
    else if (type == "u16")  return llvm::Type::getInt16PtrTy(*context, true);
    else if (type == "u32") return llvm::Type::getInt32PtrTy(*context, true);
    else if (type == "u64")  return llvm::Type::getInt64PtrTy(*context, true);
    else if (type == "f16") return llvm::Type::getHalfTy(*context);
    else if (type == "f32") return llvm::Type::getFloatTy(*context);
    else if (type == "f64") return llvm::Type::getDoubleTy(*context);

    else if (type == "string") return llvm::Type::getInt8PtrTy(*context);
    return nullptr;
}
llvm::Value* LLVMCodeGenVisitor::getInitValueForType(const std::string &type) {
    if (type == "i32") {
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0);
    } else if (type == "f32") {
        return llvm::ConstantFP::get(llvm::Type::getFloatTy(*context), 0.0);
    } else if (type == "u8") {
        return llvm::ConstantInt::get(llvm::Type::getInt8Ty(*context), 0);
    } else if (type == "u32") {
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0);
    } else if (type == "str") {
        return llvm::ConstantPointerNull::get(llvm::Type::getInt8PtrTy(*context));
    } else if (type == "string") {
        return llvm::ConstantPointerNull::get(llvm::Type::getInt8PtrTy(*context));
    } else if (type.find("array[") == 0) {
        size_t startPos = type.find('[') + 1;
        size_t endPos = type.find(',', startPos);
        std::string elementType = type.substr(startPos, endPos - startPos);
        size_t arraySize = std::stoi(type.substr(endPos + 1, type.find(']') - endPos - 1));

        llvm::Type* llvmElementType = getLLVMTypeFromASTType(elementType);
        std::vector<llvm::Constant*> initialValues;
        for (size_t i = 0; i < arraySize; ++i) {
            initialValues.push_back(static_cast<llvm::Constant*>(getInitValueForType(elementType)));
        }
        return llvm::ConstantArray::get(llvm::ArrayType::get(llvmElementType, arraySize), initialValues);
    } else if (type == "i8*") {
        return llvm::ConstantPointerNull::get(llvm::Type::getInt8PtrTy(*context));
    } else if (type == "i32*") {
        return llvm::ConstantPointerNull::get(llvm::Type::getInt32PtrTy(*context));
    } else if (type == "void") {
        return nullptr;
    }

    return nullptr;
}

void LLVMCodeGenVisitor::addVariable(const std::string &name, llvm::Value *value) {
        variables[name] = value;
    }


llvm::Value* LLVMCodeGenVisitor::getVariable(const std::string &name) {
    auto it = variables.find(name);
    if (it != variables.end()) {
        return it->second;
    }
    return nullptr;  
}


llvm::Value* LLVMCodeGenVisitor::lookupVariable(const std::string &name) {
    auto it = symbolTable.find(name);
    if (it != symbolTable.end()) {
        return it->second;
    }
    return nullptr;
}


llvm::Value* LLVMCodeGenVisitor::visit(VariableDeclarationNode &node) {
    llvm::Type* llvmType = getLLVMType(node.varType);
    if (!builder->GetInsertBlock()) {
        std::cerr << "Builder não inicializado corretamente" << std::endl;
        return nullptr;
    }

    // Verifique se o nome da variável é válido.
    if (node.varName.empty()) {
        std::cerr << "Nome da variável não pode ser vazio" << std::endl;
        return nullptr;
    }
    llvm::Value* alloca = builder->CreateAlloca(llvmType, nullptr, node.varName);
    
    if (node.initializer) {
        auto initVal = node.initializer->accept(*this);
        builder->CreateStore(initVal, alloca);
    }
    symbolTable[node.varName] = alloca;
    return alloca;
}


llvm::Value* LLVMCodeGenVisitor::visit(ExpressionStatementNode &node) {
    llvm::Value* exprValue = node.expression->accept(*this);
    return nullptr;
}

llvm::Value* LLVMCodeGenVisitor::visit(IntegerLiteralNode &node) {
    return llvm::ConstantInt::get(getLLVMType(node.type), node.value);
}
llvm::Value* LLVMCodeGenVisitor::visit(FunctionNode &node) {
    llvm::Type *returnType = getLLVMTypeFromASTType(node.returnType);
    std::vector<llvm::Type*> paramTypes;

    for (const auto &param : node.parameters) {
        llvm::Type *paramType = getLLVMTypeFromASTType(param->varType);
        paramTypes.push_back(paramType);
    }

    llvm::FunctionType *funcType = llvm::FunctionType::get(returnType, paramTypes, false);
    llvm::Function *function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, node.functionName, module);

    unsigned idx = 0;
    for (auto &arg : function->args()) {
        arg.setName(node.parameters[idx++]->varName);
    }

    llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(*context, "entry", function);
    builder->SetInsertPoint(entryBlock);

    for (const auto &stmt : node.functionBody) {
        stmt->accept(*this);
    }

    if (!entryBlock->getTerminator()) {
        if (returnType->isVoidTy()) {
            builder->CreateRetVoid(); 
        } else {
            builder->CreateRet(llvm::ConstantInt::get(returnType, 0));
        }
    }

    llvm::verifyFunction(*function);
    return function;
}

llvm::Value* LLVMCodeGenVisitor::visit(FunctionCallNode &node) {
    llvm::Function* function = module->getFunction(node.functionName);
    if (!function) {
        throw std::runtime_error("Function not found: " + node.functionName);
    }

    std::vector<llvm::Value*> llvmArguments;
    
    for (auto &arg : node.arguments) {
        llvm::Value* argValue = arg->accept(*this);
        llvmArguments.push_back(argValue);
    }

    return builder->CreateCall(function, llvmArguments);
}


llvm::Value* LLVMCodeGenVisitor::visit(StructDeclarationNode &node) {
    if (structTypes.find(node.structName) != structTypes.end()) {
        return nullptr;
    }

    std::vector<llvm::Type*> memberTypes;
    for (const auto &member : node.members) {
        llvm::Type* llvmType = getLLVMType(member->varType);
        memberTypes.push_back(llvmType);
    }

    llvm::StructType *structType = llvm::StructType::create(*context, node.structName);
    structType->setBody(memberTypes, /*isPacked=*/false);


    structTypes[node.structName] = structType;

    return nullptr; 
}
llvm::Value* LLVMCodeGenVisitor::visit(StructInstantiationNode &node) {
    if (structTypes.find(node.structName) == structTypes.end()) {
        std::cerr << "Struct não declarada: " << node.structName << std::endl;
        return nullptr;
    }

    llvm::StructType *structType = structTypes[node.structName];
    
    llvm::AllocaInst *allocaInst = builder->CreateAlloca(structType, nullptr, node.structName);
    
    return allocaInst;
}




llvm::Value* LLVMCodeGenVisitor::visit(ConstructorNode &node) {
    llvm::StructType* structType = structTypes[node.associatedStruct];

    std::vector<llvm::Type*> paramTypes;
    for (const auto &param : node.parameters) {
        llvm::Type* llvmParamType = getLLVMType(param->varType);
        paramTypes.push_back(llvmParamType);
    }

    llvm::FunctionType *funcType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(*context), paramTypes, /*isVarArg=*/false);

    llvm::Function *llvmFunc = llvm::Function::Create(
        funcType, llvm::Function::ExternalLinkage, node.functionName, module);

    llvmFunc->setName(node.associatedStruct + ".constructor");

    llvm::BasicBlock *entry = llvm::BasicBlock::Create(*context, "entry", llvmFunc);
    builder->SetInsertPoint(entry);

    llvm::Value* structInstance = allocatedStructs[node.associatedStruct];
    builder->CreateAlloca(structType, nullptr);

    for (size_t i = 0; i < node.parameters.size(); ++i) {
        llvm::Value* paramValue = node.parameters[i]->accept(*this);
        llvm::Value* memberPtr = builder->CreateStructGEP(structType, structInstance, i);
        builder->CreateStore(paramValue, memberPtr);
    }

    builder->CreateRetVoid();
    return llvmFunc;
}
llvm::Value* LLVMCodeGenVisitor::visit(MethodNode &node){
    return nullptr;
}

llvm::Value* LLVMCodeGenVisitor::visit(BinaryExpressionNode &node) {
    llvm::Value* left = node.left->accept(*this);
    llvm::Value* right = node.right->accept(*this);

    if (left->getType()->isPointerTy()) {
        auto varDecl = dynamic_cast<VariableReferenceNode*>(node.left.get());
        left = builder->CreateLoad(getLLVMTypeFromASTType(varDecl->getType()), left, "load_left");
    }

    if (right->getType()->isPointerTy()) {
        auto varDecl = dynamic_cast<VariableReferenceNode*>(node.right.get());
        right = builder->CreateLoad(getLLVMTypeFromASTType(varDecl->getType()), right, "load_right");
    }


    if (left->getType()->isHalfTy()) {
        llvm::Type* floatType = llvm::Type::getFloatTy(*context);
        left = builder->CreateFPExt(left, floatType, "promote_left");
    }

    if (right->getType()->isHalfTy()) {
        llvm::Type* floatType = llvm::Type::getFloatTy(*context);
        right = builder->CreateFPExt(right, floatType, "promote_right");
    }

    llvm::Type* leftType = left->getType();
    llvm::Type* rightType = right->getType();

    if (node.op == "+") {
        if (leftType->isFloatTy() && rightType->isFloatTy()) {
            return builder->CreateFAdd(left, right, "addtmp");
        } else if (leftType->isIntegerTy() && rightType->isIntegerTy()) {
            return builder->CreateAdd(left, right, "addtmp");
        }
    } else if (node.op == "-") {
        if (leftType->isFloatTy() && rightType->isFloatTy()) {
            return builder->CreateFSub(left, right, "subtmp");
        } else if (leftType->isIntegerTy() && rightType->isIntegerTy()) {
            return builder->CreateSub(left, right, "subtmp");
        }
    } else if (node.op == "*") {
        if (leftType->isFloatTy() && rightType->isFloatTy()) {
            return builder->CreateFMul(left, right, "multmp");
        } else if (leftType->isIntegerTy() && rightType->isIntegerTy()) {
            return builder->CreateMul(left, right, "multmp");
        }
    } else if (node.op == "/") {
        if (leftType->isFloatTy() && rightType->isFloatTy()) {
            return builder->CreateFDiv(left, right, "divtmp");
        } else if (leftType->isIntegerTy() && rightType->isIntegerTy()) {
            return builder->CreateSDiv(left, right, "divtmp");
        }
    } else if (node.op == "<") {
        if (leftType->isIntegerTy() && rightType->isIntegerTy()) {
            return builder->CreateICmpSLT(left, right, "cmp_lt");
        } else if (leftType->isFloatTy() && rightType->isFloatTy()) {
            return builder->CreateFCmpULT(left, right, "cmp_lt");
        }
    }  else if (node.op == ">") {
        if (leftType->isIntegerTy() && rightType->isIntegerTy()) {
            return builder->CreateICmpSGT(left, right, "cmp_gt");
        } else if (leftType->isFloatTy() && rightType->isFloatTy()) {
            return builder->CreateFCmpUGT(left, right, "cmp_gt");
        }
    } else if (node.op == "==") {
        if (leftType->isIntegerTy() && rightType->isIntegerTy()) {
            return builder->CreateICmpEQ(left, right, "cmp_eq");
        } else if (leftType->isFloatTy() && rightType->isFloatTy()) {
            return builder->CreateFCmpOEQ(left, right, "cmp_eq");
        }
    } else if (node.op == "!=") {
        if (leftType->isIntegerTy() && rightType->isIntegerTy()) {
            return builder->CreateICmpNE(left, right, "cmp_neq");
        } else if (leftType->isFloatTy() && rightType->isFloatTy()) {
            return builder->CreateFCmpONE(left, right, "cmp_neq");
        }
    } else if (node.op == "<=") {
        if (leftType->isIntegerTy() && rightType->isIntegerTy()) {
            return builder->CreateICmpSLE(left, right, "cmp_le");
        } else if (leftType->isFloatTy() && rightType->isFloatTy()) {
            return builder->CreateFCmpOLE(left, right, "cmp_le");
        }
    } else if (node.op == ">=") {
        if (leftType->isIntegerTy() && rightType->isIntegerTy()) {
            return builder->CreateICmpSGE(left, right, "cmp_ge");
        } else if (leftType->isFloatTy() && rightType->isFloatTy()) {
            return builder->CreateFCmpOGE(left, right, "cmp_ge");
        }
    } else if (node.op == "+=") {
        auto varDecl = dynamic_cast<VariableReferenceNode*>(node.left.get());
        if (varDecl) {
            llvm::Value* ptr = lookupVariable(varDecl->getName()); 
            if (ptr) {
                llvm::Value* currentValue = builder->CreateLoad(getLLVMTypeFromASTType(varDecl->getType()), ptr, "current_value");
                llvm::Value* updatedValue;
                if (currentValue->getType()->isIntegerTy() && right->getType()->isIntegerTy()) {
                    updatedValue = builder->CreateAdd(currentValue, right, "updated_value");
                } else if (currentValue->getType()->isFloatTy() && right->getType()->isFloatTy()) {
                    updatedValue = builder->CreateFAdd(currentValue, right, "updated_value"); 
                } else {
                    throw std::runtime_error("Incompatible types for '+=' operation");
                }
                builder->CreateStore(updatedValue, ptr);

                return updatedValue;
            } else {
                throw std::runtime_error("Variable not found: " + varDecl->getName());
            }
        }
    }

    throw std::runtime_error("Invalid operand types for operation: " + node.op);
}





llvm::Value* LLVMCodeGenVisitor::visit(VariableReferenceNode &node) {
    llvm::Value* variableValue = lookupVariable(node.variableName);
    if (!variableValue) {
        throw std::runtime_error("Undefined variable: " + node.variableName);
    }
    return variableValue;
}

llvm::Value* LLVMCodeGenVisitor::visit(ReturnNode &node){
    return nullptr;
}
llvm::Value* LLVMCodeGenVisitor::visit(IfNode &node){
    return nullptr;
}
llvm::Value* LLVMCodeGenVisitor::visit(WhileNode &node){
    return nullptr;
}

llvm::Value* LLVMCodeGenVisitor::visit(ForNode &node) {
    llvm::Function *function = builder->GetInsertBlock()->getParent();
    
    // Create basic blocks for the loop structure
    llvm::BasicBlock *entryBB = builder->GetInsertBlock();
    llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(*context, "loop", function);
    llvm::BasicBlock *bodyBB = llvm::BasicBlock::Create(*context, "body", function);
    llvm::BasicBlock *afterBB = llvm::BasicBlock::Create(*context, "after_loop", function);

    // Initialize the loop variable
    node.initializer->accept(*this);
    
    // Jump to the loop condition
    builder->CreateBr(loopBB);
    builder->SetInsertPoint(loopBB);

    // Evaluate the loop condition
    llvm::Value *condValue = node.condition->accept(*this);
    builder->CreateCondBr(condValue, bodyBB, afterBB);

    // Execute the loop body
    builder->SetInsertPoint(bodyBB);
    node.body->accept(*this);

    // Process the increment expression (ensure this updates the variable)
    node.increment->accept(*this);

    // Jump back to the loop condition
    builder->CreateBr(loopBB);
    
    // Set the insert point after the loop
    builder->SetInsertPoint(afterBB);

    return nullptr;
}








llvm::Value* LLVMCodeGenVisitor::visit(AssignmentNode &node) {
    llvm::Value* exprValue = node.expression->accept(*this);

    llvm::Value* varValue = getVariable(node.variableName);

    if (!varValue) {
        throw std::runtime_error("Variable not found: " + node.variableName);
    }

    return builder->CreateStore(exprValue, varValue);
}

llvm::Value* LLVMCodeGenVisitor::visit(BlockNode &node) {
    llvm::Value* lastValue = nullptr;
    for (auto &stmt : node.statements) {
        lastValue = stmt->accept(*this);  
    }
    return lastValue;
}

