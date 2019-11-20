﻿#include "parse_analyser.h"

ParseAnalyser::ParseAnalyser(string fileName, list<struct Lexeme>* lexList, ErrorHanding* errorHanding) {
	this->labelCount = 0;
	this->regCount = 0;
	this->midcodeGenerator->OpenMidcodeFile(fileName);
	this->iter = lexList->begin();
	this->iterEnd = lexList->end();
	this->errorHanding = errorHanding;
}

void ParseAnalyser::CountIterator(int step) {
	if (step > 0) {
		while (step--) {
			iter++;
		}
	} else {
		while (step++) {
			iter--;
		}
	}
}

Symbol* ParseAnalyser::InsertIdentifier(KIND_SYMBOL kind, TYPE_SYMBOL type, int level) {
	return checkTable->AddSymbol(iter->value, kind, type, level);
}

void ParseAnalyser::InsertSymbolTable(string name, int level) {
	symbolTableMap.insert(pair<string, SymbolTable*>(name, checkTable->GetSymbolMap(level)));
}

void ParseAnalyser::CleanLevel(int level) {
	checkTable->ClearLevel(level);
}

Symbol* ParseAnalyser::FindSymbol(int level) {
	return checkTable->FindSymbol(iter->value, level);
}

bool ParseAnalyser::IsThisIdentifier(string identifier) {
	return iter->identifier == identifier;
}

bool ParseAnalyser::IsPlusOrMinu() {
	return IsThisIdentifier(PLUS) || IsThisIdentifier(MINU);
}

bool ParseAnalyser::IsMultOrDiv() {
	return IsThisIdentifier(MULT) || IsThisIdentifier(DIV);
}

bool ParseAnalyser::IsVariableDefine() {
	if (IsThisIdentifier(INTTK) || IsThisIdentifier(CHARTK)) {
		CountIterator(+2);
		if (IsThisIdentifier(LPARENT)) {
			CountIterator(-2);
			return false;
		} else {
			CountIterator(-2);
			return true;
		}
	} else {
		return false;
	}
}

void ParseAnalyser::AddChild(SyntaxNode* node) {
	node->AddChild(new SyntaxNode(iter->identifier, iter->value));
	CountIterator(+1);
}

void ParseAnalyser::AddCommaChild(SyntaxNode* node) {
	if (IsThisIdentifier(COMMA)) {
		AddChild(node);
	}
}

void ParseAnalyser::AddSemicnChild(SyntaxNode* node) {
	if (IsThisIdentifier(SEMICN)) {
		AddChild(node);	// SEMICN
	} else {
		CountIterator(-1);
		errorHanding->AddError(iter->lineNumber, MISSING_SEMICN);
		CountIterator(+1);
	}
}

void ParseAnalyser::AddWhileChild(SyntaxNode* node) {
	if (IsThisIdentifier(WHILETK)) {
		AddChild(node);	// WHILETK
	} else {
		CountIterator(-1);
		errorHanding->AddError(iter->lineNumber, MISSING_WHILE_IN_DO_WHILE);
		CountIterator(+1);
	}
}

void ParseAnalyser::AddRbrackChild(SyntaxNode* node) {
	if (IsThisIdentifier(RBRACK)) {
		AddChild(node);	// RBRACK
	} else {
		CountIterator(-1);
		errorHanding->AddError(iter->lineNumber, MISSING_RBRACK);
		CountIterator(+1);
	}
}

void ParseAnalyser::AddRparentChild(SyntaxNode* node) {
	if (IsThisIdentifier(RPARENT)) {
		AddChild(node);	// RPARENT
	} else {
		CountIterator(-1);
		errorHanding->AddError(iter->lineNumber, MISSING_RPARENT);
		CountIterator(+1);
	}
}

SyntaxNode* ParseAnalyser::AddSyntaxChild(string syntaxName, SyntaxNode* node) {
	SyntaxNode* subRoot = new SyntaxNode(syntaxName);
	node->AddChild(subRoot);
	return subRoot;
}

int ParseAnalyser::AnalyzeInteger(SyntaxNode* node) {
	int op = 1;
	if (IsPlusOrMinu()) {
		if (IsThisIdentifier(MINU)) {
			op = -1;
		}
		AddChild(node);
	}

	int integer = op * stoi(iter->value);
	AddChild(AddSyntaxChild(UNSIGNINT, node));		// INTCON

	return integer;
}

void ParseAnalyser::AnalyzeConstDefine(SyntaxNode* node, int level) {
	Symbol* symbol = new Symbol();
	TYPE_SYMBOL type = iter->identifier == INTTK ? INT : CHAR;
	AddChild(node);	// INTTK or CHARTK
	while (IsThisIdentifier(IDENFR)) {
		if (FindSymbol(level)) {
			errorHanding->AddError(iter->lineNumber, REDEFINITION);
		} else {
			symbol = InsertIdentifier(CONST, type, level);
		}
		AddChild(node);	// IDENFR
		AddChild(node);	// ASDSIGN
		if (IsThisIdentifier(CHARCON)) {
			symbol->SetConstValue("\'" + iter->value + "\'");
			AddChild(node);	// CHARCON
		} else if (IsThisIdentifier(INTCON) || IsPlusOrMinu()) {
			int value = AnalyzeInteger(AddSyntaxChild(INTEGER, node));
			symbol->SetConstValue(to_string(value));
		} else {
			errorHanding->AddError(iter->lineNumber, DEFINE_CONST_OTHERS);
			AddChild(node);
		}
		AddCommaChild(node);	// COMMA
	}
}

void ParseAnalyser::AnalyzeConstDeclare(SyntaxNode* node, int level) {
	while (IsThisIdentifier(CONSTTK)) {
		AddChild(node);	// CONSTTK
		AnalyzeConstDefine(AddSyntaxChild(CONST_DEFINE, node), level);
		AddSemicnChild(node);	// SEMICN
	}
}

void ParseAnalyser::AnalyzeVariableDefine(SyntaxNode* node, int level) {
	TYPE_SYMBOL type = iter->identifier == INTTK ? INT : CHAR;
	AddChild(node);	// INTTK or CHARTK
	while (IsThisIdentifier(IDENFR)) {
		if (FindSymbol(level)) {
			errorHanding->AddError(iter->lineNumber, REDEFINITION);
		}
		InsertIdentifier(VARIABLE, type, level);
		AddChild(node);	// IDENFR

		if (IsThisIdentifier(LBRACK)) {
			AddChild(node);	// LBRACK
			int arrayLength = stoi(iter->value);
			AddChild(AddSyntaxChild(UNSIGNINT, node));	// INTCON
			AddRbrackChild(node);
			Symbol* symbol = InsertIdentifier(ARRAY, type, level);
			symbol->SetArrayLength(arrayLength);
		}
		AddCommaChild(node);	// COMMA
	}
}

void ParseAnalyser::AnalyzeVariableDeclare(SyntaxNode* node, int level) {
	while (IsVariableDefine()) {
		AnalyzeVariableDefine(AddSyntaxChild(VARIABLE_DEFINE, node), level);
		AddSemicnChild(node);	// SEMICN
	}
}

void ParseAnalyser::AnalyzeValuePrameterTable(SyntaxNode* node) {
	if (IsThisIdentifier(RPARENT)) {
		if (tempFunction != NULL && tempFunction->GetParameterCount() != 0) {
			errorHanding->AddError(iter->lineNumber, FUNCTION_PARAMETER_NUMBER_DONT_MATCH);
		}
		return;
	}
	string str;
	TYPE_SYMBOL type;
	while (!IsThisIdentifier(RPARENT)) {
		SyntaxNode* expressionNode = AddSyntaxChild(EXPRESSION, node);
		type = AnalyzeExpression(expressionNode);
		str.push_back(type == INT ? '0' : '1');
		midcodeGenerator->PrintPushParameter(expressionNode->GetNumericalValue());
		while (IsThisIdentifier(COMMA)) {
			AddChild(node);	// COMMA
			type = AnalyzeExpression(AddSyntaxChild(EXPRESSION, node));
			str.push_back(type == INT ? '0' : '1');
		}
	}

	if (tempFunction != NULL && tempFunction->GetParameterCount() != str.length()) {
		errorHanding->AddError(iter->lineNumber, FUNCTION_PARAMETER_NUMBER_DONT_MATCH);
	} else if (tempFunction != NULL && tempFunction->GetParameter() != str) {
		errorHanding->AddError(iter->lineNumber, FUNCTION_PARAMETER_TYPE_DONT_MATCH);
	}
}

void ParseAnalyser::AnalyzeReturnCallSentence(SyntaxNode* node) {
	if (FindSymbol(0) == NULL) {
		errorHanding->AddError(iter->lineNumber, UNDEFINED);
	}
	tempFunction = FindSymbol(0);
	AddChild(node);	// IDENFR
	AddChild(node);	// LPARENT
	AnalyzeValuePrameterTable(AddSyntaxChild(VALUE_PARAMETER_TABLE, node));
	AddRparentChild(node);	// RPARENT
	midcodeGenerator->PrintCallFunction(tempFunction->GetName());
}

TYPE_SYMBOL ParseAnalyser::AnalyzeFactor(SyntaxNode* node) {
	TYPE_SYMBOL type;
	if (IsThisIdentifier(IDENFR)) {
		if (FindSymbol(0) != NULL
			&& FindSymbol(0)->GetKind() == FUNCTION
			&& FindSymbol(0)->GetType() != VOID) {
			type = FindSymbol(0)->GetType();
			AnalyzeReturnCallSentence(AddSyntaxChild(RETURN_CALL_SENTENCE, node));
			midcodeGenerator->PrintAssignReturn(regCount++);
		} else {
			if (FindSymbol(0) == NULL) {
				if (FindSymbol(1) == NULL) {
					errorHanding->AddError(iter->lineNumber, UNDEFINED);
					type = INT;
				} else {
					type = FindSymbol(1)->GetType();
				}
			} else {
				type = FindSymbol(0)->GetType();
			}
			string name = iter->value;
			AddChild(node);	// IDENFR
			if (IsThisIdentifier(LBRACK)) {
				AddChild(node);	// LBRACK
				SyntaxNode* expression = AddSyntaxChild(EXPRESSION, node);
				TYPE_SYMBOL expressType = AnalyzeExpression(expression);
				if (expressType != INT) {
					errorHanding->AddError(iter->lineNumber, ILLEGAL_ARRAY_INDEX);
				}
				midcodeGenerator->PrintLoadToTempReg(name,
					expression->GetNumericalValue(), regCount++);
				AddRbrackChild(node);	// RBRACK
			} else {
				node->SetNumericalValue(name);
			}
		}
	} else if (IsThisIdentifier(LPARENT)) {
		AddChild(node);	// LPARENT
		SyntaxNode* expression = AddSyntaxChild(EXPRESSION, node);
		type = AnalyzeExpression(expression);
		type = INT;
		node->SetNumericalValue(expression->GetNumericalValue());
		AddRparentChild(node);	// RPARENT
	} else if (IsThisIdentifier(CHARCON)) {
		type = CHAR;
		midcodeGenerator->PrintChar(iter->value);
		AddChild(node);
	} else {
		type = INT;
		int integer = AnalyzeInteger(AddSyntaxChild(INTEGER, node));
		node->SetNumericalValue(to_string(integer));
	}
	return type;
}

TYPE_SYMBOL ParseAnalyser::AnalyzeItem(SyntaxNode* node) {
	TYPE_SYMBOL type;
	string op;
	int factorCount = 1;
	SyntaxNode* factorRoot = NULL;

	while (true) {
		SyntaxNode* anotherFactorRoot = AddSyntaxChild(FACTOR, node);
		int regNumber = regCount;
		type = AnalyzeFactor(anotherFactorRoot);
		if (regNumber == regCount) {
			if (factorCount == 1) {
				factorRoot = anotherFactorRoot;
			} else if (factorCount == 2) {
				if (factorRoot == NULL) {
					midcodeGenerator->PrintRegOpNumber(regCount++, regNumber,
						anotherFactorRoot->GetNumericalValue(), op);
				} else {
					midcodeGenerator->PrintNumberOpNumber(regCount++,
						factorRoot->GetNumericalValue(),
						anotherFactorRoot->GetNumericalValue(), op);
				}
			} else {
				midcodeGenerator->PrintRegOpNumber(regCount++, regNumber,
					anotherFactorRoot->GetNumericalValue(), op);
			}
		} else {
			if (factorCount == 2) {
				if (factorRoot == NULL) {
					midcodeGenerator->PrintNumberOpReg(regCount,
						factorRoot->GetNumericalValue(), regCount - 1, op);
					regCount++;
				} else {
					midcodeGenerator->PrintRegOpReg(regCount, regNumber, regCount - 1, op);
					regCount++;
				}
			} else if (factorCount != 1) {
				midcodeGenerator->PrintRegOpReg(regCount, regNumber, regCount - 1, op);
				regCount++;
			}

		}

		if (IsMultOrDiv()) {
			op = iter->value;
			factorCount++;
			AddChild(node);
		} else {
			break;
		}
	}

	if (factorCount == 1) {
		return type;
	} else {
		return INT;
	}
}

TYPE_SYMBOL ParseAnalyser::AnalyzeExpression(SyntaxNode* node) {
	TYPE_SYMBOL type;
	if (IsPlusOrMinu()) {
		AddChild(node);	// PLUS or MINU
	}
	type = AnalyzeItem(AddSyntaxChild(ITEM, node));
	while (IsPlusOrMinu()) {
		AddChild(node);
		AnalyzeItem(AddSyntaxChild(ITEM, node));
		type = INT;
	}
	return type;
}

void ParseAnalyser::AnalyzeCondition(SyntaxNode* node, bool isFalseBranch) {
	SyntaxNode* expression1 = AddSyntaxChild(EXPRESSION, node);
	if (AnalyzeExpression(expression1) != INT) {
		CountIterator(-1);
		errorHanding->AddError(iter->lineNumber, ILLEGAL_TYPE_IN_IF);
		CountIterator(+1);
	}
	if (IsThisIdentifier(LSS)
		|| IsThisIdentifier(LEQ)
		|| IsThisIdentifier(GRE)
		|| IsThisIdentifier(GEQ)
		|| IsThisIdentifier(EQL)
		|| IsThisIdentifier(NEQ)) {
		string op = iter->identifier;
		AddChild(node);
		SyntaxNode* expression2 = AddSyntaxChild(EXPRESSION, node);
		if (AnalyzeExpression(expression2) != INT) {
			CountIterator(-1);
			errorHanding->AddError(iter->lineNumber, ILLEGAL_TYPE_IN_IF);
			CountIterator(+1);
		}
		if (op == "==") {
			midcodeGenerator->PrintBeqOrBne(labelCount, expression1->GetNumericalValue(),
				expression2->GetNumericalValue(), BEQ, isFalseBranch);
		} else if (op == "!=") {
			midcodeGenerator->PrintBeqOrBne(labelCount, expression1->GetNumericalValue(),
				expression2->GetNumericalValue(), BNE, isFalseBranch);
		} else if (op == "<") {
			midcodeGenerator->PrintBgeOrBlt(labelCount, expression1->GetNumericalValue(),
				expression2->GetNumericalValue(), BLT, isFalseBranch);
		} else if (op == ">=") {
			midcodeGenerator->PrintBgeOrBlt(labelCount, expression1->GetNumericalValue(),
				expression2->GetNumericalValue(), BGE, isFalseBranch);
		} else if (op == "<=") {
			midcodeGenerator->PrintBgtOrBle(labelCount, expression1->GetNumericalValue(),
				expression2->GetNumericalValue(), BLE, isFalseBranch);
		} else if (op == ">") {
			midcodeGenerator->PrintBgtOrBle(labelCount, expression1->GetNumericalValue(),
				expression2->GetNumericalValue(), BGT, isFalseBranch);
		}
	} else {
		midcodeGenerator->PrintBezOrBnz(labelCount,
			expression1->GetNumericalValue(), isFalseBranch);
	}
}

bool ParseAnalyser::AnalyzeIfSentence(SyntaxNode* node, TYPE_SYMBOL returnType) {
	bool noReturn = true;

	AddChild(node);	// IFTK
	int elseLabel = labelCount++;

	AddChild(node);	// LPARENT
	AnalyzeCondition(AddSyntaxChild(CONDITION, node), true);
	AddRparentChild(node);	// RPARENT
	noReturn = AnalyzeSentence(AddSyntaxChild(SENTENCE, node), returnType);

	int endifLabel = labelCount++;
	midcodeGenerator->PrintGotoLabel(endifLabel);
	midcodeGenerator->PrintLabel(elseLabel);

	if (IsThisIdentifier(ELSETK)) {
		AddChild(node);	// ELSETK
		noReturn = AnalyzeSentence(AddSyntaxChild(SENTENCE, node), returnType) && noReturn;
	}
	midcodeGenerator->PrintLabel(endifLabel);

	return noReturn;
}

int ParseAnalyser::AnalyzeStep(SyntaxNode* node) {
	int step = stoi(iter->value);
	AddChild(AddSyntaxChild(UNSIGNINT, node));	// INTCON
	return step;
}

void ParseAnalyser::AnalyseWhile(SyntaxNode* node, TYPE_SYMBOL returnType) {
	AddChild(node);	// WHILETK
	int whileLabel = labelCount++;
	midcodeGenerator->PrintLabel(whileLabel);

	AddChild(node);	// LPARENT
	int endWhileLabel = labelCount++;
	AnalyzeCondition(AddSyntaxChild(CONDITION, node), true);
	AddRparentChild(node);	// RPARENT

	AnalyzeSentence(AddSyntaxChild(SENTENCE, node), returnType);

	midcodeGenerator->PrintGotoLabel(whileLabel);
	midcodeGenerator->PrintLabel(endWhileLabel);
}

void ParseAnalyser::AnalyseDoWhile(SyntaxNode* node, bool& noReturn, TYPE_SYMBOL returnType) {
	AddChild(node);	// DOTK

	int doLabel = labelCount++;
	midcodeGenerator->PrintLabel(doLabel);

	noReturn = AnalyzeSentence(AddSyntaxChild(SENTENCE, node), returnType);

	AddWhileChild(node);	// WHILETK
	AddChild(node);	// LPARENT
	AnalyzeCondition(AddSyntaxChild(CONDITION, node), false);
	AddRparentChild(node);	// RPARENT
}

void ParseAnalyser::AnalyseFor(SyntaxNode* node, TYPE_SYMBOL returnType) {
	AddChild(node);	// FORTK
	AddChild(node);	// LPARENT
	if (FindSymbol(0) == NULL && FindSymbol(1) == NULL) {
		errorHanding->AddError(iter->lineNumber, UNDEFINED);
	}

	string name = iter->value;
	AddChild(node);	// IDENFR
	AddChild(node);	// ASSIGN
	SyntaxNode* expressionNode = AddSyntaxChild(EXPRESSION, node);
	AnalyzeExpression(expressionNode);
	midcodeGenerator->PrintAssignValue(name, "", expressionNode->GetNumericalValue());

	int forLabel = labelCount++;
	midcodeGenerator->PrintLabel(forLabel);

	AddSemicnChild(node);	// SEMICN

	int endForLabel = labelCount++;
	AnalyzeCondition(AddSyntaxChild(CONDITION, node), true);

	AddSemicnChild(node);	// SEMICN

	if (FindSymbol(0) == NULL && FindSymbol(1) == NULL) {
		errorHanding->AddError(iter->lineNumber, UNDEFINED);
	}
	string name1 = iter->value;
	AddChild(node);	// IDENFR
	AddChild(node);	// ASSIGN
	if (FindSymbol(0) == NULL && FindSymbol(1) == NULL) {
		errorHanding->AddError(iter->lineNumber, UNDEFINED);
	}
	string name2 = iter->value;
	AddChild(node);	// IDENFR
	string op = iter->value;
	AddChild(node);	// PLUS or MINU
	int step = AnalyzeStep(AddSyntaxChild(STEP, node));
	AddRparentChild(node);	// RPARENT
	AnalyzeSentence(AddSyntaxChild(SENTENCE, node), returnType);
	midcodeGenerator->PrintStep(name1, name2, op, step);

	midcodeGenerator->PrintGotoLabel(forLabel);
	midcodeGenerator->PrintLabel(endForLabel);
}

bool ParseAnalyser::AnalyzeLoopSentence(SyntaxNode* node, TYPE_SYMBOL returnType) {
	bool noReturn = true;
	if (IsThisIdentifier(WHILETK)) {
		AnalyseWhile(node, returnType);
	} else if (IsThisIdentifier(DOTK)) {
		AnalyseDoWhile(node, noReturn, returnType);
	} else if (IsThisIdentifier(FORTK)) {
		AnalyseFor(node, returnType);
	}
	return noReturn;
}

void ParseAnalyser::AnalyzeAssignSentence(SyntaxNode* node) {
	KIND_SYMBOL kind;
	if (FindSymbol(0) == NULL) {
		if (FindSymbol(1) == NULL) {
			errorHanding->AddError(iter->lineNumber, UNDEFINED);
		} else {
			kind = FindSymbol(1)->GetKind();
			if (kind == CONST) {
				errorHanding->AddError(iter->lineNumber, ASSIGN_TO_CONST);
			}
		}
	} else {
		kind = FindSymbol(0)->GetKind();
		if (kind == CONST) {
			errorHanding->AddError(iter->lineNumber, ASSIGN_TO_CONST);
		}
	}
	string name = iter->value;
	string arrayIndex = "";
	AddChild(node);	// IDENFR

	SyntaxNode* expressionNode;
	if (IsThisIdentifier(LBRACK)) {
		AddChild(node);	// LBRACK
		expressionNode = AddSyntaxChild(EXPRESSION, node);
		if (AnalyzeExpression(expressionNode) == CHAR) {
			errorHanding->AddError(iter->lineNumber, ILLEGAL_ARRAY_INDEX);
		}
		arrayIndex = expressionNode->GetNumericalValue();
		AddRbrackChild(node);	// RBRACK
	}
	AddChild(node);	// ASSIGN
	expressionNode = AddSyntaxChild(EXPRESSION, node);
	AnalyzeExpression(expressionNode);
	midcodeGenerator->PrintAssignValue(name, arrayIndex, expressionNode->GetNumericalValue());
}

void ParseAnalyser::AnalyseScanfIdentifier(SyntaxNode* node) {
	if (FindSymbol(0) == NULL && FindSymbol(1) == NULL) {
		errorHanding->AddError(iter->lineNumber, UNDEFINED);
	} else {
		Symbol* symbol = FindSymbol(0) != NULL ? FindSymbol(0) : FindSymbol(1);
		string type = symbol->GetType() == INT ? INT_TYPE : CHAR_TYPE;
		midcodeGenerator->PrintScanf(type, symbol->GetName());
	}
	AddChild(node);	// IDENFR
}

void ParseAnalyser::AnalyzeScanfSentence(SyntaxNode* node) {
	AddChild(node);	// SCANFTK
	AddChild(node);	// LPARENT

	AnalyseScanfIdentifier(node);
	while (IsThisIdentifier(COMMA)) {
		AddChild(node);	// COMMA
		AnalyseScanfIdentifier(node);
	}

	AddRparentChild(node);	// RPARENT
}

void ParseAnalyser::AnalyzePrintfSentence(SyntaxNode* node) {
	AddChild(node);	// PRINTFTK
	AddChild(node);	// LPARENT

	if (IsThisIdentifier(STRCON)) {
		int stringNumber = stringTable->AddString(iter->value);
		midcodeGenerator->PrintString(stringNumber);
		AddChild(AddSyntaxChild(STRING, node));
		if (IsThisIdentifier(COMMA)) {
			AddChild(node);	// COMMA
			SyntaxNode* expression = AddSyntaxChild(EXPRESSION, node);
			TYPE_SYMBOL type = AnalyzeExpression(expression);
			if (type == INT) {
				midcodeGenerator->PrintInteger(expression->GetNumericalValue());
			} else {
				midcodeGenerator->PrintChar(expression->GetNumericalValue());
			}
			midcodeGenerator->PrintNewline();
		}
	} else {
		SyntaxNode* expression = AddSyntaxChild(EXPRESSION, node);
		TYPE_SYMBOL type = AnalyzeExpression(expression);
		if (type == INT) {
			midcodeGenerator->PrintInteger(expression->GetNumericalValue());
		} else {
			midcodeGenerator->PrintChar(expression->GetNumericalValue());
		}
		midcodeGenerator->PrintNewline();
	}

	AddRparentChild(node);	// RPARENT
}

TYPE_SYMBOL ParseAnalyser::AnalyzeReturnSentence(SyntaxNode* node) {
	TYPE_SYMBOL type = VOID;
	AddChild(node);	// RETURNTK
	if (IsThisIdentifier(LPARENT)) {
		AddChild(node);	// LPARENT
		type = AnalyzeExpression(AddSyntaxChild(EXPRESSION, node));
		midcodeGenerator->PrintReturn(false, node->GetNumericalValue());
		AddRparentChild(node);	// RPARENT
	}
	return type;
}

bool ParseAnalyser::AnalyzeSentence(SyntaxNode* node, TYPE_SYMBOL returnType) {
	bool noReturn = true;
	if (IsThisIdentifier(IFTK)) {
		noReturn = AnalyzeIfSentence(AddSyntaxChild(IF_SENTENCE, node), returnType);
	} else if (IsThisIdentifier(WHILETK)
		|| IsThisIdentifier(DOTK)
		|| IsThisIdentifier(FORTK)) {
		noReturn = AnalyzeLoopSentence(AddSyntaxChild(LOOP_SENTENCE, node), returnType);
	} else if (IsThisIdentifier(LBRACE)) {
		AddChild(node);	// LBRACE
		noReturn = AnalyzeSentenceCollection(AddSyntaxChild(SENTENCE_COLLECTION, node), returnType);
		AddChild(node);	// RBRACE
	} else if (IsThisIdentifier(IDENFR)) {
		noReturn = true;
		if (FindSymbol(0) != NULL
			&& FindSymbol(0)->GetKind() == FUNCTION
			&& FindSymbol(0)->GetType() != VOID) {
			AnalyzeReturnCallSentence(AddSyntaxChild(RETURN_CALL_SENTENCE, node));
			AddSemicnChild(node);	// SEMICN
		} else if (FindSymbol(0) != NULL
			&& FindSymbol(0)->GetKind() == FUNCTION
			&& FindSymbol(0)->GetType() == VOID) {
			AnalyzeReturnCallSentence(AddSyntaxChild(NO_RETURN_CALL_SENTENCE, node));
			AddSemicnChild(node);	// SEMICN
		} else {
			if (FindSymbol(0) != NULL || FindSymbol(1) != NULL) {
				AnalyzeAssignSentence(AddSyntaxChild(ASSIGN_SENTENCE, node));
				AddSemicnChild(node);	// SEMICN
			} else if (FindSymbol(0) == NULL) {
				if (FindSymbol(1) == NULL) {
					errorHanding->AddError(iter->lineNumber, UNDEFINED);
				}
				CountIterator(+1);
				if (IsThisIdentifier(LPARENT)) {
					while (!IsThisIdentifier(RPARENT)) {
						CountIterator(+1);
					}
					CountIterator(+1);
				} else if (IsThisIdentifier(ASSIGN)) {
					while (!IsThisIdentifier(SEMICN)) {
						CountIterator(+1);
					}
					CountIterator(+1);
				}
			}
		}
	} else if (IsThisIdentifier(SCANFTK)) {
		noReturn = true;
		AnalyzeScanfSentence(AddSyntaxChild(SCANF_SENTENCE, node));
		AddSemicnChild(node);	// SEMICN
	} else if (IsThisIdentifier(PRINTFTK)) {
		noReturn = true;
		AnalyzePrintfSentence(AddSyntaxChild(PRINTF_SENTENCE, node));
		AddSemicnChild(node);	// SEMICN
	} else if (IsThisIdentifier(SEMICN)) {
		noReturn = true;
		AddSemicnChild(node);	// SEMICN
	} else if (IsThisIdentifier(RETURNTK)) {
		TYPE_SYMBOL type = AnalyzeReturnSentence(AddSyntaxChild(RETURN_SENTENCE, node));
		noReturn = type == VOID;
		CountIterator(-1);
		if (returnType == VOID && type != VOID) {
			errorHanding->AddError(iter->lineNumber, RETURN_IN_NO_RETURN_FUNCTION);
		} else if (returnType != type) {
			errorHanding->AddError(iter->lineNumber, NO_RETURN_OR_WRONG_RETURN_IN_RETURN_FUNCTION);
		}
		CountIterator(+1);
		AddSemicnChild(node);	// SEMICN
	}
	return noReturn;
}

bool ParseAnalyser::AnalyzeSentenceCollection(SyntaxNode* node, TYPE_SYMBOL returnType) {
	bool noReturn = true;
	while (!IsThisIdentifier(RBRACE)) {
		if (!AnalyzeSentence(AddSyntaxChild(SENTENCE, node), returnType)) {
			noReturn = false;
		}
	}

	return noReturn;
}

void ParseAnalyser::AnalyzeCompositeSentence(SyntaxNode* node, TYPE_SYMBOL returnType) {
	if (IsThisIdentifier(CONSTTK)) {
		AnalyzeConstDeclare(AddSyntaxChild(CONST_DECLARE, node), 1);
	}
	if (IsThisIdentifier(INTTK) || IsThisIdentifier(CHARTK)) {
		AnalyzeVariableDeclare(AddSyntaxChild(VARIABLE_DECLARE, node), 1);
	}
	bool noReturn = AnalyzeSentenceCollection(AddSyntaxChild(SENTENCE_COLLECTION, node), returnType);
	if (returnType != VOID && noReturn) {
		CountIterator(-1);
		errorHanding->AddError(iter->lineNumber, NO_RETURN_OR_WRONG_RETURN_IN_RETURN_FUNCTION);
		CountIterator(+1);
	}
}

void ParseAnalyser::AnalyzeMain(SyntaxNode* node) {
	AddChild(node);	// VOIDTK
	InsertIdentifier(FUNCTION, VOID, 0);
	tempFunction = FindSymbol(0);
	CleanLevel(1);
	AddChild(node);	// MAINTK
	AddChild(node);	// LPARENT
	AddRparentChild(node);	// RPARENT
	midcodeGenerator->PrintVoidFuncDeclare(tempFunction);
	AddChild(node);	// LBRACE
	AnalyzeCompositeSentence(AddSyntaxChild(COMPOSITE_SENTENCE, node), VOID);
	midcodeGenerator->PrintReturn(true, NULL);
	AddChild(node);	// RBRACE
	InsertSymbolTable(tempFunction->GetName(), 1);
}

void ParseAnalyser::AnalyzeParameterTable(SyntaxNode* node) {
	while (IsThisIdentifier(INTTK) || IsThisIdentifier(CHARTK)) {

		TYPE_SYMBOL type = iter->identifier == INTTK ? INT : CHAR;
		AddChild(node);	// INTTK or CHARTK

		if (FindSymbol(1) != NULL) {
			errorHanding->AddError(iter->lineNumber, REDEFINITION);
		}
		tempFunction->AddParameter(type == INT ? '0' : '1');
		InsertIdentifier(PARAMETER, type, 1);
		AddChild(node);	// IDENTFR
		AddCommaChild(node);
	}
}

void ParseAnalyser::AnalyzeVoidFunc(SyntaxNode* node) {
	AddChild(node);	// VOIDTK
	if (FindSymbol(0) != NULL) {
		errorHanding->AddError(iter->lineNumber, REDEFINITION);
	}
	InsertIdentifier(FUNCTION, VOID, 0);
	tempFunction = FindSymbol(0);
	CleanLevel(1);
	AddChild(node);	// IDENFR
	AddChild(node);	// LPARENT
	AnalyzeParameterTable(AddSyntaxChild(PARAMETER_TABLE, node));
	AddRparentChild(node);	// RPARENT
	midcodeGenerator->PrintVoidFuncDeclare(tempFunction);
	AddChild(node);	// LBRACE
	AnalyzeCompositeSentence(AddSyntaxChild(COMPOSITE_SENTENCE, node), VOID);
	midcodeGenerator->PrintReturn(true, NULL);
	AddChild(node);	// RBRACE
	InsertSymbolTable(tempFunction->GetName(), 1);
}

void ParseAnalyser::AnalyzeHeadState(SyntaxNode* node) {
	TYPE_SYMBOL type = iter->identifier == INTTK ? INT : CHAR;
	AddChild(node);	// INTTK or CHARTK
	if (FindSymbol(0) != NULL) {
		errorHanding->AddError(iter->lineNumber, REDEFINITION);
	}
	InsertIdentifier(FUNCTION, type, 0);
	tempFunction = FindSymbol(0);
	CleanLevel(1);
	AddChild(node);	// IDENFR
}

void ParseAnalyser::AnalyzeFunc(SyntaxNode* node) {
	AnalyzeHeadState(AddSyntaxChild(HEAD_STATE, node));
	AddChild(node);	// LPARENT
	AnalyzeParameterTable(AddSyntaxChild(PARAMETER_TABLE, node));
	AddRparentChild(node);	// RPARENT
	midcodeGenerator->PrintFuncDeclare(tempFunction);
	AddChild(node);	// LBRACE
	AnalyzeCompositeSentence(AddSyntaxChild(COMPOSITE_SENTENCE, node),
		tempFunction->GetType());
	AddChild(node);	// RBRACE
	InsertSymbolTable(tempFunction->GetName(), 1);
}

void ParseAnalyser::BuildSyntaxTree(SyntaxNode* root) {
	string flag = CONST_DECLARE;

	while (iter != iterEnd) {
		if (IsThisIdentifier(CONSTTK)) {
			AnalyzeConstDeclare(AddSyntaxChild(CONST_DECLARE, root), 0);
		} else if (IsThisIdentifier(INTTK) || IsThisIdentifier(CHARTK)) {
			if (flag == CONST_DECLARE) {
				CountIterator(+2);
				if (IsThisIdentifier(LPARENT)) {
					flag = RETURN_FUNCTION;
				} else {
					flag = VARIABLE_DECLARE;
				}
				CountIterator(-2);
			}
			if (flag == RETURN_FUNCTION) {
				AnalyzeFunc(AddSyntaxChild(RETURN_FUNCTION, root));
			} else {
				AnalyzeVariableDeclare(AddSyntaxChild(VARIABLE_DECLARE, root), 0);
				flag = RETURN_FUNCTION;
			}
		} else {
			if (IsThisIdentifier(VOIDTK)) {
				CountIterator(+1);
			}
			if (IsThisIdentifier(MAINTK)) {
				CountIterator(-1);
				AnalyzeMain(AddSyntaxChild(MAIN_FUNCTION, root));
			} else {
				CountIterator(-1);
				AnalyzeVoidFunc(AddSyntaxChild(NO_RETURN_FUNCTION, root));
			}
		}
	}
	InsertSymbolTable("global", 0);
}

void ParseAnalyser::AnalyzeParse() {
	SyntaxNode* root = new SyntaxNode(PROGRAM);
	BuildSyntaxTree(root);
	// root->print();
}

map<string, SymbolTable*> ParseAnalyser::GetSymbolTableMap() {
	return this->symbolTableMap;
}

void ParseAnalyser::FileClose() {
	midcodeGenerator->FileClose();
}