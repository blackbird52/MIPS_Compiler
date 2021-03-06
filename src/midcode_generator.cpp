﻿#include "midcode_generator.h"

using namespace std;

MidcodeGenerator::MidcodeGenerator() = default;

void MidcodeGenerator::OpenMidcodeFile(const string &file_name) {
    this->midcode_.open(file_name);
}

list<Midcode *> MidcodeGenerator::midcode_list() {
    return this->midcode_list_;
}

void MidcodeGenerator::AddMidcode(Midcode *midcode) {
    this->midcode_list_.push_back(midcode);
}

void MidcodeGenerator::FileClose() {
    this->midcode_.close();
}

void MidcodeGenerator::PrintParameter(TypeSymbol type, const string &name) {
    string parameter_type = type == TypeSymbol::INT ? kIntType : kCharType;
    this->midcode_ << "parameter " + parameter_type + " " + name << endl;

    MidcodeInstr midcode_instr = type == TypeSymbol::INT
                                 ? MidcodeInstr::PARA_INT : MidcodeInstr::PARA_CHAR;
    this->AddMidcode(new Midcode(midcode_instr, name));
}

void MidcodeGenerator::PrintVariable(TypeSymbol type, const string &name) {
    string variable_type = type == TypeSymbol::INT ? kIntType : kCharType;
    this->midcode_ << "variable " + variable_type + " " + name << endl;

    MidcodeInstr midcode_instr = type == TypeSymbol::INT
                                 ? MidcodeInstr::VAR_INT : MidcodeInstr::VAR_CHAR;
    this->AddMidcode(new Midcode(midcode_instr, name));
}

void MidcodeGenerator::PrintFuncDeclare(Symbol *function) {
    string type = function->type() == TypeSymbol::INT ? kIntType : kCharType;
    this->midcode_ << endl << type + " " + function->name() + "()" << endl;

    MidcodeInstr midcode_instr = function->type() == TypeSymbol::INT
                                 ? MidcodeInstr::INT_FUNC_DECLARE : MidcodeInstr::CHAR_FUNC_DECLARE;
    this->AddMidcode(new Midcode(midcode_instr, function->name()));
}

void MidcodeGenerator::PrintVoidFuncDeclare(Symbol *function) {
    this->midcode_ << endl << "void " + function->name() + "()" << endl;

    this->AddMidcode(new Midcode(MidcodeInstr::VOID_FUNC_DECLARE, function->name()));
}

void MidcodeGenerator::PrintReturn(bool isVoid, const string &value) {
    if (isVoid) {
        this->midcode_ << "return" << endl;

        this->AddMidcode(new Midcode(MidcodeInstr::RETURN_NON));
    } else {
        this->midcode_ << "return " + value << endl;

        this->AddMidcode(new Midcode(MidcodeInstr::RETURN, value));
    }
}

void MidcodeGenerator::PrintLabel(int label) {
    this->midcode_ << "Label_" << label << ":" << endl;

    this->AddMidcode(new Midcode(MidcodeInstr::LABEL, label));
}

void MidcodeGenerator::PrintJump(int label) {
    this->midcode_ << "jump Label_" << label << ":" << endl;

    this->AddMidcode(new Midcode(MidcodeInstr::JUMP, label));
}

void MidcodeGenerator::PrintLoop() {
    this->AddMidcode(new Midcode(MidcodeInstr::LOOP));
}

void MidcodeGenerator::PrintStep(const string &name1, const string &name2, const string &op, int step) {
    midcode_ << name1 << " = " << name2 << " " + op + " " << step << endl;

    this->AddMidcode(new Midcode(MidcodeInstr::STEP));
    this->AddMidcode(new Midcode(midcodeinstr::GetOperatorInstr(op),
                                 OperaMember::NUMBER_OP_NUMBER, name1, name2, to_string(step)));
}

void MidcodeGenerator::PrintBez(int label, const string &expression) {
    this->midcode_ << "bez " << expression << " Label_" << ":" << label << endl;

    this->AddMidcode(new Midcode(MidcodeInstr::BEZ, expression, label));
}

void MidcodeGenerator::PrintBnz(int label, const string &expression) {
    this->midcode_ << "bnz " << expression << " Label_" << ":" << label << endl;

    this->AddMidcode(new Midcode(MidcodeInstr::BNZ, expression, label));
}

void MidcodeGenerator::PrintBezOrBnz(int label, const string &expression, bool is_false_branch) {
    if (is_false_branch) {
        this->PrintBez(label, expression);
    } else {
        this->PrintBnz(label, expression);
    }
}

void MidcodeGenerator::PrintBeq(int label, const string &expression1, const string &expression2) {
    this->midcode_ << "beq " + expression1
                   << " " + expression2 << " Label_" << label << endl;

    this->AddMidcode(new Midcode(MidcodeInstr::BEQ, expression1, expression2, label));
}

void MidcodeGenerator::PrintBne(int label, const string &expression1, const string &expression2) {
    this->midcode_ << "bne " + expression1
                   << " " + expression2 << " Label_" << label << endl;

    this->AddMidcode(new Midcode(MidcodeInstr::BNE, expression1, expression2, label));
}

void
MidcodeGenerator::PrintBeqOrBne(int label, const string &expression1, const string &expression2, Judge judge,
                                bool is_false_branch) {
    if (judge == Judge::BEQ) {
        if (is_false_branch) {
            this->PrintBne(label, expression1, expression2);
        } else {
            this->PrintBeq(label, expression1, expression2);
        }
    } else {
        if (is_false_branch) {
            this->PrintBeq(label, expression1, expression2);
        } else {
            this->PrintBne(label, expression1, expression2);
        }
    }
}

void MidcodeGenerator::PrintBge(int label, const string &expression1, const string &expression2) {
    this->midcode_ << "bge " + expression1
                   << " " + expression2 << " Label_" << label << endl;

    this->AddMidcode(new Midcode(MidcodeInstr::BGE, expression1, expression2, label));
}

void MidcodeGenerator::PrintBlt(int label, const string &expression1, const string &expression2) {
    this->midcode_ << "blt " + expression1
                   << " " + expression2 << " Label_" << label << endl;

    this->AddMidcode(new Midcode(MidcodeInstr::BLT, expression1, expression2, label));
}

void
MidcodeGenerator::PrintBgeOrBlt(int label, const string &expression1, const string &expression2, Judge judge,
                                bool is_false_branch) {
    if (judge == Judge::BGE) {
        if (is_false_branch) {
            this->PrintBlt(label, expression1, expression2);
        } else {
            this->PrintBge(label, expression1, expression2);
        }
    } else {
        if (is_false_branch) {
            this->PrintBge(label, expression1, expression2);
        } else {
            this->PrintBlt(label, expression1, expression2);
        }
    }
}

void MidcodeGenerator::PrintBgt(int label, const string &expression1, const string &expression2) {
    this->midcode_ << "bgt " + expression1
                   << " " + expression2 << " Label_" << label << endl;

    this->AddMidcode(new Midcode(MidcodeInstr::BGT, expression1, expression2, label));
}

void MidcodeGenerator::PrintBle(int label, const string &expression1, const string &expression2) {
    this->midcode_ << "ble " + expression1
                   << " " + expression2 << " Label_" << label << endl;

    this->AddMidcode(new Midcode(MidcodeInstr::BLE, expression1, expression2, label));
}

void
MidcodeGenerator::PrintBgtOrBle(int label, const string &expression1, const string &expression2, Judge judge,
                                bool is_false_branch) {
    if (judge == Judge::BGT) {
        if (is_false_branch) {
            this->PrintBle(label, expression1, expression2);
        } else {
            this->PrintBgt(label, expression1, expression2);
        }
    } else {
        if (is_false_branch) {
            this->PrintBgt(label, expression1, expression2);
        } else {
            this->PrintBle(label, expression1, expression2);
        }
    }
}

void MidcodeGenerator::PrintString(int string_number) {
    midcode_ << "printf str_" << string_number << endl;

    this->AddMidcode(new Midcode(MidcodeInstr::PRINTF_STRING, string_number));
}

void MidcodeGenerator::PrintInteger(const string &number) {
    midcode_ << "printf int " + number << endl;

    this->AddMidcode(new Midcode(MidcodeInstr::PRINTF_INT, number));
}

void MidcodeGenerator::PrintChar(const string &c) {
    midcode_ << "printf char " + c << endl;

    this->AddMidcode(new Midcode(MidcodeInstr::PRINTF_CHAR, c));
}

void MidcodeGenerator::PrintEnd() {
    midcode_ << "printf_end" << endl;

    this->AddMidcode(new Midcode(MidcodeInstr::PRINTF_END));
}

void MidcodeGenerator::PrintScanf(const string &type, const string &identifier) {
    midcode_ << "scanf " + type << " " + identifier << endl;

    MidcodeInstr midcode_instr = type == "int" ? MidcodeInstr::SCANF_INT : MidcodeInstr::SCANF_CHAR;

    this->AddMidcode(new Midcode(midcode_instr, identifier));
}

void MidcodeGenerator::PrintAssignValue(const string &name, const string &array_index, const string &value) {
    if (array_index.empty()) {
        midcode_ << name + " = " + value << endl;

        this->AddMidcode(new Midcode(MidcodeInstr::ASSIGN, name, value));
    } else {
        midcode_ << name + "[" + array_index + "] = " + value << endl;

        this->AddMidcode(new Midcode(MidcodeInstr::ASSIGN_ARRAY, name, array_index, value));
    }
}

void MidcodeGenerator::PrintLoadToTempReg(const string &name, const string &array_index, int temp_reg_count) {
    if (array_index.empty()) {
        midcode_ << "#" << temp_reg_count << " = " << name << endl;

        this->AddMidcode(new Midcode(MidcodeInstr::LOAD, temp_reg_count, name));
    } else {
        midcode_ << "#" << temp_reg_count << " = " << name + "[" + array_index + "]" << endl;

        this->AddMidcode(new Midcode(MidcodeInstr::LOAD_ARRAY, name, array_index, temp_reg_count));
    }
}

void MidcodeGenerator::PrintPushParameter(const string &function, const string &value, int count) {

    midcode_ << "push " + value << endl;

    this->AddMidcode(new Midcode(MidcodeInstr::PUSH, function, value, count));
}

void MidcodeGenerator::PrintCallFunction(const string &name) {
    midcode_ << "call " + name << endl;

    this->AddMidcode(new Midcode(MidcodeInstr::CALL, name));
}

void MidcodeGenerator::PrintFuncEnd() {
    midcode_ << "function end" << endl;

    this->AddMidcode(new Midcode(MidcodeInstr::FUNCTION_END));
}

void MidcodeGenerator::PrintSave(const string &function_name) {
    midcode_ << "save " + function_name << endl;

    this->AddMidcode(new Midcode(MidcodeInstr::SAVE, function_name));
}

void MidcodeGenerator::PrintAssignReturn(int temp_reg_count) {
    midcode_ << "#" << temp_reg_count << " = RET" << endl;

    this->AddMidcode(new Midcode(MidcodeInstr::ASSIGN_RETURN, temp_reg_count));
}

void MidcodeGenerator::PrintRegOpReg(int result_reg, int op_reg1, int op_reg2, const string &op) {
    midcode_ << "#" << result_reg << " = #" << op_reg1 << " " + op + " #" << op_reg2 << endl;

    this->AddMidcode(new Midcode(midcodeinstr::GetOperatorInstr(op),
                                 OperaMember::REG_OP_REG, result_reg, op_reg1, op_reg2));
}

void MidcodeGenerator::PrintRegOpNumber(int result_reg, int op_reg, const string &number, const string &op) {
    midcode_ << "#" << result_reg << " = #" << op_reg << " " + op + " " + number << endl;

    this->AddMidcode(new Midcode(midcodeinstr::GetOperatorInstr(op),
                                 OperaMember::REG_OP_NUMBER, result_reg, op_reg, number));
}

void MidcodeGenerator::PrintNumberOpReg(int result_reg, const string &number, int op_reg, const string &op) {
    midcode_ << "#" << result_reg << " = " + number + " " + op + " #" << op_reg << endl;

    this->AddMidcode(new Midcode(midcodeinstr::GetOperatorInstr(op),
                                 OperaMember::NUMBER_OP_REG, result_reg, op_reg, number));
}

void
MidcodeGenerator::PrintNumberOpNumber(int result_reg, const string &number1, const string &number2, const string &op) {
    midcode_ << "#" << result_reg << " = " + number1 + " " + op + " " + number2 << endl;

    this->AddMidcode(new Midcode(midcodeinstr::GetOperatorInstr(op),
                                 OperaMember::NUMBER_OP_NUMBER, result_reg, number1, number2));
}

void MidcodeGenerator::PrintNeg(int result_reg, const string &number) {
    midcode_ << "#" << result_reg << " = -" + number << endl;

    this->AddMidcode(new Midcode(MidcodeInstr::NEG, result_reg, number));
}