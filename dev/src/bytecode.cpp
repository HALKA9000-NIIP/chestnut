#pragma once

#include "bytecode.hpp"



Instruction::Instruction(ByteSeq bytes) {
    try {
        if(bytes.size() == 0)
            bytes = { IT_Unknown };

        this->bytecode = bytes;
        this->opcode = bytes[0];

        switch(this->opcode) {
            case IT_Label: {
                this->operand["id"] = this->bytecode.copy(1, 16);
                this->operand["name"] = this->bytecode.copy(17 , -1);
            } break;

            case IT_Jump: {
                this->operand["index"] = this->bytecode.copy(1, -1);
            } break;

            case IT_IFJump: {
                this->operand["index"] = this->bytecode.copy(1, -1);
            } break;
        }
    } catch(std::out_of_range ignored) {
        std::cout << "EXCEPTION" << std::endl;
    }
}

Instruction::Instruction(int opcode) {
    this->opcode = opcode;
    this->operand = {};
    this->setBytecode();
}

Instruction::Instruction(int opcode, std::unordered_map<std::string, ByteSeq> operand) {
    this->opcode = opcode;
    this->operand = operand;
    this->setBytecode();
}

std::string Instruction::toText() {
    std::string text = "";

    switch(this->opcode) {
        case IT_Unknown:
        return "; Unknown";

        case IT_Label:
        return "label\t" + Bytecode(this->operand["id"]).toHexString(false) + "\t" + Bytecode(this->operand["name"]).toString();

        case IT_Jump:
        return "jump\t" + Bytecode(this->operand["index"]).toHexString(false);

        case IT_IFJump:
        return "ifjump\t" + Bytecode(this->operand["index"]).toHexString(false);

        default:
        return "; Unknown_";
    }
}

void Instruction::setBytecode() {
    try {
        switch(this->opcode) {
            case IT_Unknown: {
                this->bytecode.append(IT_Unknown);
            } break;

            case IT_Label: {
                this->bytecode.append(IT_Label);
                this->bytecode.append(this->operand["id"]);
                this->bytecode.append(Instruction::escape(this->operand["name"]));
            } break;

            case IT_Jump: {
                this->bytecode.append(IT_Jump);
                this->bytecode.append(Instruction::escape(this->operand["index"]));
            } break;

            case IT_IFJump: {
                this->bytecode.append(IT_IFJump);
                this->bytecode.append(Instruction::escape(this->operand["index"]));
            } break;

            default: {
                this->bytecode.append(IT_Unknown);
            } break;
        }
    } catch(std::out_of_range ignored) {
        std::cout << "EXCEPTION" << std::endl;
    }
}



HeaderInfo::HeaderInfo() {}

HeaderInfo::HeaderInfo(ByteSeq bytes) {
    if(bytes.size() != HEADER_LEN)
        return;

    // 各データのバイト数変更に対応するためのインデックス管理
    int index = 0;

    int magicNumSize = MAGIC_NUMBER.size();
    this->magicNum = bytes.copy(index, index + magicNumSize - 1);
    index += magicNumSize;
}

ByteSeq HeaderInfo::toByteSeq() {
    ByteSeq bytes;

    bytes.append(MAGIC_NUMBER);
    bytes.append(ByteSeq(HEADER_LEN - bytes.size()));

    return bytes;
}



FuncData::FuncData() {}

FuncData::FuncData(ByteSeq id, ByteSeq name) {
    this->id = id;
    this->name = name;
}

FuncData::FuncData(ByteSeq id, ByteSeq name, int begin, int end) {
    this->id = id;
    this->name = name;
    this->begin = begin;
    this->end = end;
}



Bytecode::Bytecode() {}

Bytecode::Bytecode(ByteSeq source) {
    this->source = source;
}

Bytecode::Bytecode(int source) {
    std::stringstream ss;
    ss << std::hex << source;
    std::string hex = ss.str();
    ByteSeq hexBytes;
    bool isEvenDigNum = hex.length() % 2 == 0;

    if(!isEvenDigNum)
        hexBytes.push_back((Byte)std::stoi("0" + std::string { hex[0] }, nullptr, 16));

    for(int i = isEvenDigNum ? 0 : 1; i < hex.length(); i += 2)
        hexBytes.push_back((Byte)std::stoi(std::string { hex[i] } + std::string { hex[i + 1] }, nullptr, 16));

    this->source.push_back((Byte)source);
}

Bytecode::Bytecode(std::string source) {
    for(Byte src : source)
        this->source.push_back(src);
}

Bytecode::Bytecode(Node tree, std::string filePath, std::string sourceCode) {
    this->filePath = filePath;
    this->sourceCode = sourceCode;

    // ヘッダ部分

    HeaderInfo header;
    header.magicNum = MAGIC_NUMBER;
    this->append(header.toByteSeq());

    // ボディ部分

    for(Node node : tree.children)
        if(node.type == ND_DefFunc)
            this->funcData.push_back(FuncData(Bytecode(this->generateUUID()).source, Bytecode(node.tokenAt(0).string).source));

    InstList instList = this->toInstList(tree);
    instList.insert(instList.begin(), Instruction(IT_Jump, { { "index", FuncData::findByName(this->funcData, { 0x6D, 0x61, 0x69, 0x6E }).id } }));

    for(Instruction inst : instList) {
        this->append(inst.bytecode);
        this->append(IT_LineDiv);
    }

    this->source.pop_back();
    std::cout << std::endl;
}

Bytecode::Bytecode(std::vector<Bytecode> source) {
    for(Bytecode bc : source) {
        this->append(MAGIC_NUMBER);
        this->append(IT_LineDiv);
        this->append(bc);
    }
}

Bytecode Bytecode::append(Byte source) {
    this->source.append(source);
    return this->source;
}

Bytecode Bytecode::append(ByteSeq source) {
    this->source.append(source);
    return this->source;
}

Bytecode Bytecode::append(Bytecode source) {
    this->source.append(source.source);
    return this->source;
}

Bytecode Bytecode::pop() {
    this->source.pop_back();
    return this->source;
}

std::string Bytecode::toHexString(bool containSpace) {
    std::string res;

    for(int src : this->source) {
        std::stringstream ss;
        ss << std::hex << src;
        res += (src < 16 ? "0" : "") + ss.str() + (containSpace ? " " : "");
    }

    if(containSpace && res.length() > 0)
        res.pop_back();

    return res;
}

int Bytecode::toInt() {
    return *(unsigned int *)&this->source;
}

std::string Bytecode::toString() {
    std::string res;

    for(Byte srcChar : this->source)
        res += (char)srcChar;

    return res;
}

// LineSeq型の命令を取得
LineSeq Bytecode::divide() {
    LineSeq lines = {{}};

    try {
        for(int i = 0; i < this->source.size(); i++) {
            if(this->source.at(i) == IT_LineDiv || this->source.at(i) == IT_TokenDiv) {
                if(i + 1 < this->source.size() && this->source.at(i + 1) == this->source.at(i)) {
                    lines.back().append(this->source.at(i));
                    i++;
                } else {
                    lines.append();
                }
            } else {
                lines.back().append(this->source.at(i));
            }
        }
    } catch(std::out_of_range ignored) {
        std::cout << "EXCEPTION" << std::endl;
        return lines;
    }

    return lines;
}

InstList Bytecode::toInstList(Node node) {
    int index = 0;
    InstList instList;

    for(int i = 0; i < node.children.size(); i++) {
        InstList resLines = this->toInstList(node, i);
        instList.append(resLines);
    }

    return instList;
}

// ノードを調べてバイトコードに変換し、LineSeq型の行列を返します
// 基本的に scanNode(Node) により呼ばれます
InstList Bytecode::toInstList(Node parentNode, int &index) {
    Node node = parentNode.childAt(index);
    std::cout << std::endl << (int)index << " > " << parentNode.children.size() << " | " << (int)node.type << std::endl;
    InstList instList;

    try {

        switch(node.type) {
            case ND_Unknown: {std::cout<<"unknown"<<std::endl;
                instList.push_back(Instruction(IT_Unknown));
            } break;

            case ND_Root: {std::cout<<"root"<<std::endl;
                for(int i = 0; i < node.children.size(); i++) {
                    InstList resLines = this->toInstList(node, i);
                    instList.append(resLines);
                }
            } break;

            case ND_DefVar: {std::cout<<"defvar"<<std::endl;
                instList.push_back(Instruction(IT_LSPush, {}));//{}
                int i = 1;
                this->toInstList(node, i);
                lslen++;
            } break;

            case ND_InitVar: {std::cout<<"initvar"<<std::endl;
                Token token = node.tokenAt(2);
                ByteSeq value;

                if(token.type == TK_Number)
                    value = Bytecode(std::stoi(token.string)).source;
                else if(token.type == TK_String)
                    value = Bytecode(token.string).source;

                instList.push_back(Instruction(IT_LSPush, { { "index", value } }));
                lslen++;
            } break;

            case ND_DefFunc: {std::cout<<"deffunc"<<std::endl;
                ByteSeq funcName = Bytecode(node.tokenAt(0).string).source;
                ByteSeq funcID = Bytecode(FuncData::findByName(this->funcData, funcName).id).source;
                instList.push_back(Instruction(IT_Label, { { "id", funcID }, { "name", funcName } }));

                lllen += node.childAt(0).children.size();

                int i = 1;
                InstList insts = this->toInstList(node, i);
                instList.append(insts);
            } break;

            case ND_CallFunc: {std::cout<<"callfunc"<<std::endl;
                Token funcNameToken = node.tokenAt(0);
                std::string funcName = funcNameToken.string;
                ByteSeq funcID = FuncData::findByName(this->funcData, Bytecode(funcName).source).id;

                if(funcID.size() == 0)
                    Console::log(LogType_Error, "1822", { { "At", funcNameToken.getPositionText(this->filePath, this->sourceCode ) }, { "Id", funcName } }, false);

                instList.push_back(Instruction(IT_Jump, { { "id", funcID } }));
            } break;

            case ND_If: {std::cout<<"if"<<std::endl;
                // 次のノードに else / elseif がくれば

                if(index + 1 < parentNode.children.size()) {
                    int nextNodeType = parentNode.childAt(index + 1).type;

                    if(nextNodeType == ND_Else || nextNodeType == ND_ElseIf) {
                        //
                        std::cout<<"before else or elseif"<<std::endl;
                    } else {
                        std::cout << "not before else or elseif"<<nextNodeType<<std::endl;
                    }
                }

                // 条件式をチェック

                int i = 1;
                InstList procLines = this->toInstList(node, i);

                // procLineの行数をもとにIFJUMP命令を追加
                Bytecode lineIndex(IT_VarPref);
                lineIndex.append(Bytecode(procLines.size()));
                instList.push_back(Instruction(IT_IFJump, { { "index", lineIndex.source } }));

                instList.append(procLines);
            } break;

            case ND_Else: {std::cout<<"else"<<std::endl;
                int i = 0;
                InstList insts = this->toInstList(node, i);
                instList.append(insts);
            } break;
        }

        return instList;

    } catch(std::out_of_range ignored) {
        std::cout << "EXCEPTION" << std::endl;
        return instList;
    }
}

ByteSeq Bytecode::generateUUID() {
    ByteSeq res;
    std::random_device rand;
    std::mt19937 mt(rand());
    std::uniform_int_distribution<> rand256(0, 255);

    for(int i = 0; i < 16; i++)
        res.push_back(rand256(mt));

    return res;
}
