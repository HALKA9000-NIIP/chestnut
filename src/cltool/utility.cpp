#pragma once

#include "utility.hpp"


ches::Token::Token() {}

ches::Token::Token(Byte type) {
    this->type = type;
}

ches::Token::Token(std::string string) {
    this->string = string;
}

ches::Token::Token(Byte type, std::string string) {
    this->type = type;
    this->string = string;
}

ches::Token::Token(Byte type, std::string string, int index) {
    this->type = type;
    this->string = string;
    this->index = index;
}

// インデックスは比較されません
bool ches::Token::compare(Token token) {
    return this->type == token.type && this->string == token.string;
}

ches::Token ches::Token::getCloseParen() {
    switch(this->type) {
        case TK_LeftParen:
        return Token(TK_RightParen, ")", this->index);

        case TK_LeftBracket:
        return Token(TK_RightBracket, "]", this->index);

        case TK_LeftBrace:
        return Token(TK_RightBrace, "}", this->index);

        case TK_LeftAngleBracket:
        return Token(TK_RightAngleBracket, ">", this->index);
    }

    return Token();
}

ches::Token ches::Token::getOpenParen() {
    switch (this->type) {
        case TK_RightParen:
        return Token(TK_LeftParen, "(", this->index);

        case TK_RightBracket:
        return Token(TK_LeftBracket, "[", this->index);

        case TK_RightBrace:
        return Token(TK_LeftBrace, "{", this->index);

        case TK_RightAngleBracket:
        return Token(TK_LeftAngleBracket, "<", this->index);
    }

    return Token();
}

std::pair<int, int> ches::Token::getPosition(std::string &source) {
    int line = 0;
    int pos = 0;

    for(int i = 0; i < this->index; i++) {
        if(source[i] == '\n') {
            pos = 0;
            line++;
        } else {
            pos++;
        }
    }

    return { line, pos };
}

std::string ches::Token::getPositionText(std::string filePath, std::string &source) {
    std::pair pos = this->getPosition(source);
    return filePath + ":" + std::to_string(pos.first + 1) + ":" + std::to_string(pos.second + 1);
}

bool ches::Token::isParen() {
    return this->match(ByteSeq { TK_LeftParen, TK_LeftBracket, TK_LeftBrace, TK_RightParen, TK_RightBracket, TK_RightBrace });
}

bool ches::Token::isCloseParen() {
    return this->match(ByteSeq { TK_RightParen, TK_RightBracket, TK_RightBrace });
}

bool ches::Token::isOpenParen() {
    return this->match(ByteSeq { TK_LeftParen, TK_LeftBracket, TK_LeftBrace });
}

bool ches::Token::isValueType() {
    return std::regex_match(this->string, std::regex("bol|byt|chr|dbl|flt|int|lon|obj|sht|str|ubyt|usht|uint|ulon|void"));
}

bool ches::Token::match(ches::ByteSeq matches) {
    bool res = false;

    for(int i = 0; i < matches.size(); i++)
        if(matches[i] == this->type)
            res = true;

    return res;
}

bool ches::Token::match(std::string regexp) {
    return std::regex_match(this->string, std::regex(regexp));
}


ches::TokenSeq ches::TokenSeq::copy(int begin) {
    return this->copy(begin, this->size() - 1);
}

ches::TokenSeq ches::TokenSeq::copy(int begin, int end) {
    TokenSeq result;

    auto beginItr = begin >= 0 ? this->begin() + begin : this->begin() + this->size() + begin + 1;
    auto endItr = end >= 0 ? this->end() - this->size() + end + 1 : this->end() + end + 1;

    std::copy(beginItr, endItr, std::back_inserter(result));

    return result;
}

ches::vector_ext<ches::TokenSeq> ches::TokenSeq::divide(ches::TokenType separator) {
    vector_ext<TokenSeq> divided = {{}};

    for(Token value : *this) {
        if(value.type == separator) {
            divided.push_back((TokenSeq){});
        } else {
            divided.at(divided.size() - 1).push_back(value);
        }
    }

    return divided;
}


ches::Node::Node() {}

ches::Node::Node(Byte type) {
    this->type = type;
}

ches::Node::Node(Byte type, std::vector<Node> children) {
    this->type = type;
    this->children = children;
}

ches::Node::Node(Byte type, std::vector<Token> tokens) {
    this->type = type;
    this->tokens = tokens;
}

ches::Node::Node(Byte type, std::vector<Node> children, std::vector<Token> tokens) {
    this->type = type;
    this->children = children;
    this->tokens = tokens;
}

void ches::Node::addChild(Node node) {
    this->children.push_back(node);
}

void ches::Node::addToken(Token token) {
    this->tokens.push_back(token);
}

void ches::Node::addToken(TokenSeq tokenSeq) {
    this->tokens.push_back(tokenSeq);
}

ches::Node ches::Node::childAt(int index) {
    return this->children.at((index >= 0 ? index : this->children.size() + index));
}

ches::Token ches::Node::tokenAt(int index) {
    return this->tokens.at((index >= 0 ? index : this->tokens.size() + index));
}

void ches::Node::print() {
    this->print(this->prefix);
}

void ches::Node::print(std::string level) {
    std::cout << "\033[44m" << level << "\033[m" << " " << "\033[94m" << this->typeToString() << "\033[m" << std::endl;

    for(Token token : this->tokens)
        std::cout << "\033[44m" << level << this->prefix << "\033[m" << " " << token.string << std::endl;

    for(Node node : this->children)
        node.print(level + this->prefix);
}

std::string ches::Node::typeToString() {
    if(nodeTypeMap.count(this->type) == 1) {
        return nodeTypeMap[this->type];
    } else {
        return "???";
    }
}


ches::ByteSeq::ByteSeq() {}

ches::ByteSeq::ByteSeq(Byte value) {
    this->push_back(value);
}

ches::ByteSeq::ByteSeq(Byte value, int len) {
    for(int i = 0; i < len; i++)
        this->push_back(value);
}

ches::ByteSeq::ByteSeq(std::initializer_list<Byte> value) {
    for(Byte val : value)
        this->push_back(val);
}

ches::ByteSeq::ByteSeq(int source) {
    std::stringstream ss;
    ss << std::hex << source;
    std::string hex = ss.str();

    if(hex.length() % 2 != 0)
        hex = "0" + hex;

    for(int i = 0; i < hex.length(); i += 2)
        this->push_back((Byte)std::stoi(hex.substr(i, 2), nullptr, 10));
}

ches::ByteSeq::ByteSeq(std::string source) {
    for(Byte src : source)
        this->push_back(src);
}

ches::ByteSeq::ByteSeq(Token token) {
    switch(token.type) {
        case TK_Number:
        this->push_back(std::stoi(token.string));
        break;

        case TK_String:
        this->push_back(token.string);
        break;
    }
}

ches::ByteSeq::ByteSeq(Node node) {
    try {
        if(node.type != ND_Value)
            return;

        if(node.tokens.size() < 2)
            return;

        Token t_type = node.tokenAt(1);
        std::string type = t_type.string;

        Token t_value = node.tokenAt(0);
        std::string value = t_value.string;

        if(t_value.match("bol")) {
            bool isTrue = value == "true";
            this->push_back(ByteSeq(isTrue));
            return;
        }

        if(t_value.match("str")) {
            this->push_back(ByteSeq(value));
            return;
        }

        if(t_value.match("byt|ubyt|sht|usht|int|uint|lng|ulng")) {
            this->push_back(ByteSeq(std::stoi(value)));
            return;
        }

        // 浮動小数点型
        // if(t_value.match("dec")) {
        //     this->push_back(ByteSeq(std::stod(value)));
        //     return;
        // }
    } catch(std::out_of_range ignored) {
        std::cout << "EXCEPTION" << std::endl;
    }
}

ches::ByteSeq::ByteSeq(Node tree, std::string filePath, std::string source) {
    // ヘッダ部分

    HeaderInfo header;
    header.magicNum = ches::MAGIC_NUMBER;
    this->push_back(header.toByteSeq());

    // ボディ部分

    InstConv instConv;
    InstList instList = instConv.toInstList(tree, filePath, source);
    ByteSeq byteSeq = instList.toByteSeq();
    this->push_back(byteSeq);

    std::cout << std::endl;
}

ches::ByteSeq ches::ByteSeq::copy(int begin) {
    return this->copy(begin, -1);
}

ches::ByteSeq ches::ByteSeq::copy(int begin, int end) {
    ByteSeq result;

    auto beginItr = begin >= 0 ? this->begin() + begin : this->begin() + this->size() + begin + 1;
    auto endItr = end >= 0 ? this->end() - this->size() + end + 1 : this->end() + end + 1;

    std::copy(beginItr, endItr, std::back_inserter(result));

    return result;
}

ches::ByteSeq ches::ByteSeq::escape() {
    ByteSeq result;

    for(int i = 0; i < this->size(); i++) {
        result.push_back(this->at(i));

        if(this->at(i) == IT_LineDiv || this->at(i) == IT_TokenDiv) {
            result.push_back(this->at(i));
            i++;
        }
    }

    return result;
}

std::string ches::ByteSeq::toHexString(std::string sep) {
    std::string result;

    for(int src : *this) {
        std::stringstream ss;
        ss << std::hex << src;
        result += (src < 16 ? "0" : "") + ss.str() + sep;
    }

    if(sep.length() && sep.length() >= result.length())
        this->pop_back(sep.length());

    return result;
}

int ches::ByteSeq::toInt() {
    return std::stoi(this->toHexString(), nullptr, 16);
}

ches::LineSeq ches::ByteSeq::toLineSeq() {
    LineSeq lines = {{}};

    try {
        for(int i = 0; i < this->size(); i++) {
            if(this->at(i) == IT_LineDiv || this->at(i) == IT_TokenDiv) {
                if(i + 1 < this->size() && this->at(i + 1) == this->at(i)) {
                    lines.back().push_back(this->at(i));
                    i++;
                } else {
                    lines.push_back();
                }
            } else {
                lines.back().push_back(this->at(i));
            }
        }
    } catch(std::out_of_range ignored) {
        std::cout << "EXCEPTION" << std::endl;
        return lines;
    }

    return lines;
}

std::string ches::ByteSeq::toString() {
    std::string res;

    for(Byte srcChar : *this)
        res += (char)srcChar;

    return res;
}


ches::LineSeq::LineSeq() {}

ches::LineSeq::LineSeq(ByteSeq value) {
    this->push_back(value);
}

ches::LineSeq::LineSeq(std::initializer_list<ByteSeq> value) {
    for(ByteSeq val : value)
        this->push_back(val);
}

ches::LineSeq::LineSeq(std::vector<ByteSeq> value) {
    this->push_back(value);
}


ches::Instruction::Instruction() {}

ches::Instruction::Instruction(ByteSeq bytes) {
    Instruction inst = InstConv::toInst(bytes);
    this->opcode = inst.opcode;
    this->operand = inst.operand;
}

ches::Instruction::Instruction(int opcode) {
    this->opcode = opcode;
}

ches::Instruction::Instruction(int opcode, std::vector<ches::ByteSeq> operand) {
    this->opcode = opcode;
    this->operand = operand;
}

ches::InstList::InstList() {}

ches::InstList::InstList(Instruction value) {
    this->push_back(value);
}

ches::InstList::InstList(std::initializer_list<Instruction> value) {
    for(Instruction val : value)
        this->push_back(val);
}

ches::InstList::InstList(std::vector<Instruction> value) {
    this->push_back(value);
}

ches::ByteSeq ches::InstList::toByteSeq() {
    ByteSeq result;

    for(Instruction inst : *this) {
        ByteSeq byteSeq = InstConv::toByteSeq(inst);

        if(byteSeq.size() >= 1) {
            result.push_back(byteSeq);
            result.push_back((Byte)IT_LineDiv);
        }
    }

    if(this->size() > 0)
        result.pop_back();

    return result;
}


ches::InstConv::InstConv() {}

ches::InstList ches::InstConv::toInstList(Node tree) {
    InstList result;
    this->tree = tree;

    for(Node node : tree.children)
        if(node.type == ND_DefFunc)
            this->labelList.push_back(Function(ByteSeq::generateUUID(), ByteSeq(node.tokenAt(0).string)));

    for(int i = 0; i < tree.children.size(); i++)
        result.push_back(this->toInstList(tree, i));

    Console::writeln();

    for(Instruction inst : result)
        Console::writeln(std::to_string(inst.opcode));

    return result;
}

ches::InstList ches::InstConv::toInstList(Node tree, std::string filePath, std::string source) {
    this->filePath = filePath;
    this->source = source;
    return this->toInstList(tree);
}

ches::InstList ches::InstConv::toInstList(Node parent, int &index) {
    Node node = parent.childAt(index);
    InstList result;

    try {
        Console::writeln();
        Console::writeln(std::to_string((int)node.type) + ": " + node.typeToString());
        Console::writeln("    " + std::to_string((int)index) + " > " + std::to_string(parent.children.size()));

        switch(node.type) {
            case ND_Unknown: {
                result.push_back(Instruction(IT_Unknown));
            } break;

            case ND_Root: {
                for(int i = 0; i < node.children.size(); i++) {
                    InstList resLines = this->toInstList(node, i);
                    result.push_back(resLines);
                }
            } break;

            case ND_DefVar: {
                //result.push_back(Instruction(IT_LLPush, {  }));
                int i = 1;
                this->toInstList(node, i);
                this->localStackLen++;
            } break;

            case ND_InitVar: {
                ByteSeq value;
                Node n_value = node.childAt(0);
                Token t_type = n_value.tokenAt(0);
                Token t_value = n_value.tokenAt(1);

                if(t_type.string == "int")
                    value = ByteSeq(std::stoi(t_value.string));
                else if(t_type.string == "str")
                    value = ByteSeq(t_value.string);

                result.push_back(Instruction(IT_LSPush, { value.escape() }));
                this->localStackLen++;
            } break;

            case ND_DefFunc: {
                ByteSeq funcName = ByteSeq(node.tokenAt(0).string);
                ByteSeq funcID = ByteSeq(this->labelList.findByName(funcName).id).escape();
                result.push_back(Instruction(IT_Label, { funcID, funcName }));
                this->localListLen += node.childAt(0).children.size();

                int i = 1;
                InstList insts = this->toInstList(node, i);
                result.push_back(insts);
            } break;

            case ND_CallFunc: {
                Token funcNameToken = node.tokenAt(0);
                std::string funcName = funcNameToken.string;

                ByteSeq funcID = this->labelList.findByName(ByteSeq(funcName)).id;

                if(funcID.size() == 0)
                    Console::log(LogType_Error, 1822, { { "At", funcNameToken.getPositionText(this->filePath, this->source ) }, { "Id", funcName } }, false);

                result.push_back(Instruction(IT_Jump, { funcID }));
            } break;

            case ND_If: {
                int nextNodeType = index + 1 < parent.children.size() ? parent.childAt(index + 1).type : ND_Unknown;

                if(nextNodeType == ND_Else || nextNodeType == ND_ElseIf) {
                } else {
                    // ifelse / else が後に続かない場合

                    // 処理部分を取得
                    int i = 1;
                    InstList procLines = this->toInstList(node, i);

                    // IFスコープに処理がない場合は弾く
                    if(procLines.size() == 1)
                        break;

                    // 条件部分をスタックにプッシュ
                    i = 0;
                    InstList insts = this->toInstList(node, i);

                    // 処理部分の行数をもとにIFJUMP命令を追加
                    ByteSeq lineIndex((Byte)IT_VarPref);
                    lineIndex.push_back(ByteSeq((int)procLines.size()));
                    result.push_back(Instruction(IT_IFJump, { lineIndex }));
                    result.push_back(procLines);

                    // プッシュした条件部分をポップ
                    result.push_back(Instruction(IT_LSPop));
                }
            } break;

            case ND_Else: {
                int i = 0;
                InstList insts = this->toInstList(node, i);
                result.push_back(insts);
            } break;

            case ND_Loop: {
                int index = 0;
                InstList process = this->toInstList(node.childAt(1), index);
                Node exprs = node.childAt(0);

                switch(exprs.children.size()) {
                    case 1: {
                        //ByteSeq(exprs.childAt(0).tokenAt(0))

                        // インデクサの値を追加
                        result.push_back(Instruction(IT_LSPush, { ByteSeq((int)0).escape() }));

                        int startIndex = result.size();

                        result.push_back(Instruction(IT_LSPush, { ByteSeq(startIndex).escape() }));
                        result.push_back(Instruction(IT_IFJump));

                        // インデックスを加算
                        result.push_back(Instruction(IT_LSPush, { ByteSeq((int)1).escape() }));
                        result.push_back(Instruction(IT_Add));

                        // インデックスの条件を判定
                        int endIndex = result.size();
                        result.push_back(Instruction(IT_Compare));
                        result.push_back(Instruction(IT_LSPush, { ByteSeq(endIndex - startIndex).escape() }));
                        result.push_back(Instruction(IT_IFJump));
                    } break;

                    case 3: {
                        result.push_back(this->toInstList(exprs.childAt(0)));
                        result.push_back(this->toInstList(exprs.childAt(1)));
                        result.push_back(this->toInstList(exprs.childAt(2)));
                    } break;
                }
            } break;

            case ND_Compare: {
                ByteSeq item;

                if(node.childAt(0).type == ND_Value) {
                    item = ByteSeq(node.childAt(0).tokens.at(0));
                    result.push_back(Instruction((Byte)IT_LSPush, { item.escape() }));
                } else {
                    int i = 0;
                    result.push_back(this->toInstList(node, i));
                }

                if(node.childAt(0).type == ND_Value) {
                    item = ByteSeq(node.childAt(0).tokens.at(0));
                    result.push_back(Instruction((Byte)IT_LSPush, { item.escape() }));
                } else {
                    int i = 1;
                    result.push_back(this->toInstList(node, i));
                }

                result.push_back(Instruction((Byte)IT_Compare));
            }
        }

        return result;
    } catch(std::out_of_range ignored) {
        std::cout << "EXCEPTION" << std::endl;
        return result;
    }
}


ches::Function::Function() {}

ches::Function::Function(ches::ByteSeq id, ches::ByteSeq name) {
    this->id = id;
    this->name = name;
}

ches::Function::Function(ches::ByteSeq id, ches::ByteSeq name, int begin, int end) {
    this->id = id;
    this->name = name;
    this->begin = begin;
    this->end = end;
}


ches::FuncList::FuncList() {}

ches::FuncList::FuncList(ches::Function value) {
    this->push_back(value);
}

ches::FuncList::FuncList(std::initializer_list<Function> value) {
    for(Function val : value)
        this->push_back(val);
}

ches::FuncList::FuncList(std::vector<Function> value) {
    this->push_back(value);
}

ches::Function ches::FuncList::findById(ches::ByteSeq id) {
    for(Function func : *this)
        if(func.id == id)
            return func;

    return Function();
}

ches::Function ches::FuncList::findByName(ches::ByteSeq name) {
    for(Function func : *this)
        if(func.name == name)
            return func;

    return Function();
}


ches::Stack::Stack() {}


ches::StackList::StackList() {}


ches::HeaderInfo::HeaderInfo() {}

ches::HeaderInfo::HeaderInfo(ches::ByteSeq bytes) {
    if(bytes.size() != HEADER_LEN)
        return;

    // 各データのバイト数変更に対応するためのインデックス管理
    int index = 0;

    int magicNumSize = ches::MAGIC_NUMBER.size();
    this->magicNum = bytes.copy(index, index + magicNumSize - 1);
    index += magicNumSize;
}

ches::ByteSeq ches::HeaderInfo::toByteSeq() {
    ByteSeq result;

    result.push_back(ches::MAGIC_NUMBER);
    result.push_back(ByteSeq(0, (int)(HEADER_LEN - result.size())));

    return result;
}
