/*-------------------------------------------------------------------------
This source file is a part of m8rscript

For the latest info, see http://www.marrin.org/

Copyright (c) 2016, Chris Marrin
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

    - Redistributions of source code must retain the above copyright notice, 
	  this list of conditions and the following disclaimer.
	  
    - Redistributions in binary form must reproduce the above copyright 
	  notice, this list of conditions and the following disclaimer in the 
	  documentation and/or other materials provided with the distribution.
	  
    - Neither the name of the <ORGANIZATION> nor the names of its 
	  contributors may be used to endorse or promote products derived from 
	  this software without specific prior written permission.
	  
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.
-------------------------------------------------------------------------*/

#include "ParseEngine.h"

using namespace m8r;

Map<Token, ParseEngine::OpInfo, ParseEngine::CompareTokens> ParseEngine::_opInfo;

ParseEngine::ParseEngine(Parser* parser)
    : _parser(parser)
{
    if (!_opInfo.size()) {
        _opInfo.emplace(Token::STO,         { 1, OpInfo::RightAssoc, false, Op::MOVE });
        _opInfo.emplace(Token::ADDSTO,      { 2, OpInfo::RightAssoc, true, Op::ADD });
        _opInfo.emplace(Token::SUBSTO,      { 2, OpInfo::RightAssoc, true, Op::SUB });
        _opInfo.emplace(Token::MULSTO,      { 3, OpInfo::RightAssoc, true, Op::MUL });
        _opInfo.emplace(Token::DIVSTO,      { 3, OpInfo::RightAssoc, true, Op::DIV });
        _opInfo.emplace(Token::MODSTO,      { 3, OpInfo::RightAssoc, true, Op::MOD });
        _opInfo.emplace(Token::SHLSTO,      { 4, OpInfo::RightAssoc, true, Op::SHL });
        _opInfo.emplace(Token::SHRSTO,      { 4, OpInfo::RightAssoc, true, Op::SHR });
        _opInfo.emplace(Token::SARSTO,      { 4, OpInfo::RightAssoc, true, Op::SAR });
        _opInfo.emplace(Token::ANDSTO,      { 5, OpInfo::RightAssoc, true, Op::AND });
        _opInfo.emplace(Token::ORSTO,       { 5, OpInfo::RightAssoc, true, Op::OR });
        _opInfo.emplace(Token::XORSTO,      { 5, OpInfo::RightAssoc, true, Op::XOR });
        _opInfo.emplace(Token::LOR,         { 6, OpInfo::LeftAssoc, false, Op::LOR });
        _opInfo.emplace(Token::LAND,        { 7, OpInfo::LeftAssoc, false, Op::LAND });
        _opInfo.emplace(Token::OR,          { 8, OpInfo::LeftAssoc, false, Op::OR });
        _opInfo.emplace(Token::XOR,         { 9, OpInfo::LeftAssoc, false, Op::XOR });
        _opInfo.emplace(Token::Ampersand,   { 10, OpInfo::LeftAssoc, false, Op::AND });
        _opInfo.emplace(Token::EQ,          { 11, OpInfo::LeftAssoc, false, Op::EQ });
        _opInfo.emplace(Token::NE,          { 11, OpInfo::LeftAssoc, false, Op::NE });
        _opInfo.emplace(Token::LT,          { 12, OpInfo::LeftAssoc, false, Op::LT });
        _opInfo.emplace(Token::GT,          { 12, OpInfo::LeftAssoc, false, Op::GT });
        _opInfo.emplace(Token::GE,          { 12, OpInfo::LeftAssoc, false, Op::GE });
        _opInfo.emplace(Token::LE,          { 12, OpInfo::LeftAssoc, false, Op::LE });
        _opInfo.emplace(Token::SHL,         { 13, OpInfo::LeftAssoc, false, Op::SHL });
        _opInfo.emplace(Token::SHR,         { 13, OpInfo::LeftAssoc, false, Op::SHR });
        _opInfo.emplace(Token::SAR,         { 13, OpInfo::LeftAssoc, false, Op::SAR });
        _opInfo.emplace(Token::Plus,        { 14, OpInfo::LeftAssoc, false, Op::ADD });
        _opInfo.emplace(Token::Minus,       { 14, OpInfo::LeftAssoc, false, Op::SUB });
        _opInfo.emplace(Token::Star,        { 15, OpInfo::LeftAssoc, false, Op::MUL });
        _opInfo.emplace(Token::Slash,       { 15, OpInfo::LeftAssoc, false, Op::DIV });
        _opInfo.emplace(Token::Percent,     { 15, OpInfo::LeftAssoc, false, Op::MOD });
    }
}

void ParseEngine::syntaxError(Error error, Token token)
{
    String s;
    uint8_t c = static_cast<uint8_t>(token);
    
    switch(error) {
        case Error::Unknown:
            s = "unknown token: (";
            s += Value::toString(c);
            s += ")";
            break;
        case Error::Expected:
            s = "syntax error: expected ";
            if (c < 0x80) {
                s += '\'';
                s += c;
                s += '\'';
            } else {
                const char* errorString;
                switch(token) {
                    case Token::Expr: errorString = "expression"; break;
                    case Token::PropertyAssignment: errorString = "property assignment"; break;
                    case Token::Identifier: errorString = "identifier"; break;
                    default: errorString = "*** UNKNOWN TOKEN ***"; break;
                }
                s += errorString;
            }
            break;
    }
    
    _parser->printError(s.c_str());
}

bool ParseEngine::expect(Token token)
{
    if (getToken() != token) {
        syntaxError(Error::Expected, token);
        return false;
    }
    retireToken();
    return true;
}

bool ParseEngine::expect(Token token, bool expected)
{
    if (!expected) {
        syntaxError(Error::Expected, token);
    }
    return expected;
}

bool ParseEngine::statement()
{
    while (1) {
        if (functionDeclaration()) {
            return true;
        }
        if (getToken() == Token::EndOfFile) {
            return false;
        }
        if (getToken() == Token::Semicolon) {
            retireToken();
            return true;
        }
        if (compoundStatement() || selectionStatement() || 
            switchStatement() || iterationStatement() || jumpStatement()) {
            return true;
        }
        if (getToken() == Token::Var) {
            retireToken();
            variableDeclarationList();
            expect(Token::Semicolon);
            return true;
        } else if (getToken() == Token::Delete) {
            retireToken();
            leftHandSideExpression();
            expect(Token::Semicolon);
            return true;
        } else if (expression()) {
            _parser->discardResult();
            expect(Token::Semicolon);
            return true;
        } else {
            return false;
        }
    }
}

bool ParseEngine::functionDeclaration()
{
    if (getToken() != Token::Function) {
        return false;
    }
    retireToken();
    Atom name = getTokenValue().atom;
    expect(Token::Identifier);
    ObjectId f = function();
    _parser->addNamedFunction(f, name);
    return true;
}

bool ParseEngine::compoundStatement()
{
    if (getToken() != Token::LBrace) {
        return false;
    }
    retireToken();
    while (statement()) ;
    expect(Token::RBrace);
    return true;
}

bool ParseEngine::selectionStatement()
{
    if (getToken() != Token::If) {
        return false;
    }
    retireToken();
    expect(Token::LParen);
    expression();
    
    Label ifLabel = _parser->label();
    Label elseLabel = _parser->label();
    _parser->addMatchedJump(m8r::Op::JF, elseLabel);

    expect(Token::RParen);
    statement();

    if (getToken() == Token::Else) {
        retireToken();
        _parser->addMatchedJump(m8r::Op::JMP, ifLabel);
        _parser->matchJump(elseLabel);
        statement();
        _parser->matchJump(ifLabel);
    } else {
        _parser->matchJump(elseLabel);
    }
    return true;
}

bool ParseEngine::switchStatement()
{
    // FIXME: Implement
    return false;
}

void ParseEngine::forLoopCondAndIt()
{
    // On entry, we are at the semicolon before the cond expr
    expect(Token::Semicolon);
    Label label = _parser->label();
    expression(); // cond expr
    _parser->addMatchedJump(m8r::Op::JF, label);
    _parser->startDeferred();
    expect(Token::Semicolon);
    expression(); // iterator
    _parser->discardResult();
    _parser->endDeferred();
    expect(Token::RParen);
    statement();
    _parser->emitDeferred();
    _parser->jumpToLabel(Op::JMP, label);
    _parser->matchJump(label);
}

bool ParseEngine::iterationStatement()
{
    Token type = getToken();
    if (type != Token::While && type != Token::Do && type != Token::For) {
        return false;
    }
    
    retireToken();
    expect(Token::LParen);
    if (type == Token::While) {
        Label label = _parser->label();
        expression();
        _parser->addMatchedJump(m8r::Op::JF, label);
        expect(Token::RParen);
        statement();
        _parser->jumpToLabel(Op::JMP, label);
        _parser->matchJump(label);
    } else if (type == Token::Do) {
        Label label = _parser->label();
        statement();
        expect(Token::While);
        expect(Token::LParen);
        expression();
        _parser->jumpToLabel(m8r::Op::JT, label);
        expect(Token::RParen);
        expect(Token::Semicolon);
    } else if (type == Token::For) {
        if (getToken() == Token::Var) {
            retireToken();
            variableDeclarationList();
            if (getToken() == Token::Colon) {
                // for-in case with var
                //FIXME: implement
                // FIXME: We need a way to know if the above decl is a legit variable for for-in
                retireToken();
                leftHandSideExpression();
            } else {
                forLoopCondAndIt();
            }
        } else {
            if (expression()) {
                if (getToken() == Token::Colon) {
                    // for-in case with left hand expr
                    // FIXME: We need a way to know if the above expression is a legit left hand expr
                    // FIXME: implement
                    retireToken();
                    leftHandSideExpression();
                } else {
                    forLoopCondAndIt();
                }
            }
        }
    }
    
    // We should be at the closing paren
    return true;
}

bool ParseEngine::jumpStatement()
{
    if (getToken() == Token::Break || getToken() == Token::Continue) {
        retireToken();
        expect(Token::Semicolon);
        return true;
    }
    if (getToken() == Token::Return) {
        retireToken();
        uint32_t count = 0;
        if (expression()) {
            count = 1;
        }
        _parser->emitWithCount(m8r::Op::RET, count);
        expect(Token::Semicolon);
    }
    return false;
}

bool ParseEngine::variableDeclarationList()
{
    bool haveOne = false;
    while (variableDeclaration()) {
        if (getToken() != Token::Comma) {
            return true;
        }
        haveOne = true;
        retireToken();
    }
    return haveOne;
}

bool ParseEngine::variableDeclaration()
{
    if (getToken() != Token::Identifier) {
        return false;
    }
    Atom name = getTokenValue().atom;
    _parser->addVar(name);
    retireToken();
    if (getToken() != Token::STO) {
        return true;
    }
    retireToken();
    _parser->emitId(name, Parser::IdType::MustBeLocal);
    if (!expect(Token::Expr, expression())) {
        return false;
    }
        
    _parser->emitMove();
    _parser->discardResult();
    return true;
}

bool ParseEngine::arithmeticPrimary()
{
    if (getToken() == Token::LParen) {
        retireToken();
        expression();
        expect(Token::RParen);
        return true;
    }
    
    Op op;
    switch(getToken()) {
        case Token::INC: op = Op::PREINC; break;
        case Token::DEC: op = Op::PREDEC; break;
        case Token::Minus: op = Op::UMINUS; break;
        case Token::Twiddle: op = Op::UNEG; break;
        case Token::Bang: op = Op::UNOT; break;
        default: op = Op::UNKNOWN; break;
    }
    
    if (op != Op::UNKNOWN) {
        retireToken();
        arithmeticPrimary();
        _parser->emitUnOp(op);
        return true;
    }
    
    if (!leftHandSideExpression()) {
        return false;
    }
    switch(getToken()) {
        case Token::INC: op = Op::POSTINC; break;
        case Token::DEC: op = Op::POSTDEC; break;
        default: op = Op::UNKNOWN; break;
    }
    
    if (op != Op::UNKNOWN) {
        retireToken();
        _parser->emitUnOp(op);
    }
    return true;
}

bool ParseEngine::expression(uint8_t minPrec)
{
    if (!arithmeticPrimary()) {
        return false;
    }
    
    while(1) {
        OpInfo opInfo;            
        if (!_opInfo.find(getToken(), opInfo) || opInfo.prec < minPrec) {
            break;
        }
        uint8_t nextMinPrec = (opInfo.assoc == OpInfo::LeftAssoc) ? (opInfo.prec + 1) : opInfo.prec;
        retireToken();
        if (opInfo.sto) {
            _parser->emitDup();
        }
    
        expression(nextMinPrec);
        _parser->emitBinOp(opInfo.op);
        if (opInfo.sto) {
            _parser->emitMove();
        }
    }
    return true;
}

bool ParseEngine::leftHandSideExpression()
{
    if (getToken() == Token::New) {
        retireToken();
        leftHandSideExpression();
        return true;
    }
    
    if (getToken() == Token::Function) {
        retireToken();
        ObjectId f = function();
        _parser->pushK(f);
        return true;
    }
    
    if (!primaryExpression()) {
        return false;
    }
    while(1) {
        if (getToken() == Token::LParen) {
            retireToken();
            uint32_t argCount = argumentList();
            expect(Token::RParen);
            _parser->emitWithCount(m8r::Op::CALL, argCount);
        } else if (getToken() == Token::LBracket) {
            retireToken();
            expression();
            expect(Token::RBracket);
            _parser->emitDeref(false);
        } else if (getToken() == Token::Period) {
            retireToken();
            Atom name = getTokenValue().atom;
            if (expect(Token::Identifier)) {
                _parser->emitId(name, Parser::IdType::NotLocal);
                _parser->emitDeref(true);
            }
        } else {
            return true;
        }
    }
}

uint32_t ParseEngine::argumentList()
{
    uint32_t i = 0;
    
    if (!expression()) {
        return i;
    }
    ++i;
    while (getToken() == Token::Comma) {
        retireToken();
        expression();
        ++i;
    }
    return i;
}

bool ParseEngine::primaryExpression()
{
    switch(getToken()) {
        case Token::Identifier: _parser->emitId(getTokenValue().atom, Parser::IdType::MightBeLocal); retireToken(); break;
        case Token::Float: _parser->pushK(getTokenValue().number); retireToken(); break;
        case Token::Integer: _parser->pushK(getTokenValue().integer); retireToken(); break;
        case Token::String: _parser->pushK(getTokenValue().string); retireToken(); break;
        case Token::True: _parser->pushK(true); retireToken(); break;
        case Token::False: _parser->pushK(false); retireToken(); break;
        case Token::Null: _parser->pushK(); retireToken(); break;
        case Token::LBracket:
            retireToken();
            _parser->emitLoadLit(true);
            if (expression()) {
                _parser->emitMove();
                while (getToken() == Token::Comma) {
                    retireToken();
                    if (!expect(Token::Expr, expression())) {
                        break;
                    }
                    _parser->emitMove();
                }
            }
            expect(Token::RBracket);
            break;
        case Token::LBrace:
            retireToken();
            _parser->emitLoadLit(false);
            if (propertyAssignment()) {
                _parser->emitStoProp();
                while (getToken() == Token::Comma) {
                    retireToken();
                    if (!expect(Token::PropertyAssignment, propertyAssignment())) {
                        break;
                    }
                    _parser->emitStoProp();
                }
            }
            expect(Token::RBrace);
            break;
            
            break;
        
        default: return false;
    }
    return true;
}

bool ParseEngine::propertyAssignment()
{
    if (!propertyName()) {
        return false;
    }
    return expect(Token::Colon) && expect(Token::Expr, expression());
}

bool ParseEngine::propertyName()
{
    switch(getToken()) {
        case Token::Identifier: _parser->emitId(getTokenValue().atom, Parser::IdType::NotLocal); retireToken(); return true;
        case Token::String: _parser->pushK(getTokenValue().string); retireToken(); return true;
        case Token::Float: _parser->pushK(getTokenValue().number); retireToken(); return true;
        case Token::Integer: _parser->pushK(getTokenValue().integer); retireToken(); return true;
        default: return false;
    }
}

ObjectId ParseEngine::function()
{
    expect(Token::LParen);
    _parser->functionStart();
    formalParameterList();
    _parser->functionParamsEnd();
    expect(Token::RParen);
    expect(Token::LBrace);
    while(statement()) { }
    expect(Token::RBrace);
    _parser->emitEnd();
    return _parser->functionEnd();
}

void ParseEngine::formalParameterList()
{
    if (getToken() != Token::Identifier) {
        return;
    }
    while (1) {
        _parser->functionAddParam(getTokenValue().atom);
        retireToken();
        if (getToken() != Token::Comma) {
            return;
        }
        retireToken();
        if (getToken() != Token::Identifier) {
            syntaxError(Error::Expected, Token::Identifier);
            return;
        }
    }
}

