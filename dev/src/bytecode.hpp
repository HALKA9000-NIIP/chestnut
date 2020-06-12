#pragma once

#define HEADER_LEN      128
#define MAGIC_NUMBER    ByteSeq { 0x4F, 0x52, 0x49, 0x4E, 0x43, 0x48, 0x41, 0x4E }



struct Instruction {

    Byte opcode = IT_Unknown;
    std::unordered_map<std::string, ByteSeq> operand;
    ByteSeq bytecode;

    Instruction(ByteSeq bytes);

    Instruction(int opcode);

    Instruction(int opcode, std::unordered_map<std::string, ByteSeq> operand);

    std::string toText();

    void setBytecode();

    static ByteSeq escape(ByteSeq source) {
        for(int i = 0; i < source.size(); i++) {
            if(source[i] == IT_LineDiv || source[i] == IT_TokenDiv) {
                source.push_back(source[i]);
                i++;
            }
        }

        return source;
    }

};

typedef vector_ext<Instruction> InstList;



struct HeaderInfo {

    ByteSeq magicNum;

    HeaderInfo();

    HeaderInfo(ByteSeq bytes);

    ByteSeq toByteSeq();

};



struct FuncData {

    ByteSeq id;
    ByteSeq name;
    InstList instList;
    int begin;
    int end;

    FuncData();

    FuncData(ByteSeq id, ByteSeq name);

    FuncData(ByteSeq id, ByteSeq name, int begin, int end);

    static FuncData findById(std::vector<FuncData> fd, ByteSeq id) {
        for(FuncData f : fd)
            if(f.id == id)
                return f;

        return FuncData();
    }

    static FuncData findByName(std::vector<FuncData> fd, ByteSeq nm) {
        for(FuncData f : fd)
            if(f.name == nm)
                return f;

        return FuncData();
    }

};



//relpath
struct Bytecode {

    ByteSeq source;

    Bytecode();

    Bytecode(ByteSeq src);

    Bytecode(int src);

    Bytecode(std::string src);

    Bytecode(Node tree, std::string filePath, std::string sourceCode);

    Bytecode(std::vector<Bytecode> src);

    Bytecode append(Byte src);

    Bytecode append(ByteSeq source);

    Bytecode append(Bytecode src);

    Bytecode pop();

    std::string toHexString(bool containSpace = true);

    int toInt();

    std::string toString();

    LineSeq divide();

private:

    LineSeq lines;
    std::string filePath;
    std::string sourceCode;
    std::vector<FuncData> funcData;
    std::string spaceName;
    std::string className;
    int lslen = 0;
    int lllen = 0;

    InstList toInstList(Node node);

    InstList toInstList(Node parentNode, int &index);

    ByteSeq generateUUID();
};
