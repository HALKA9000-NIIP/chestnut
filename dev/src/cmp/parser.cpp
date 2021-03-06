#pragma once

#include "parser.hpp"



Line::Line() {}

Line::Line(std::vector<Token> tokens) {
    std::vector<Token> line;
    int nest = 0;

    for(int i = 0; i < tokens.size(); i++) {
        if(tokens[i].type == TK_Indent) {
            nest++;
        } else if(tokens[i].type == TK_Comment) {
            continue;
        } else {
            line.push_back(tokens[i]);
        }
    }

    this->tokens = line;
    this->nest = nest;
    this->beginIndex = ((line.size() != 0) ? line[0].index : -1);
}



ParenSeq::ParenSeq() {}

ParenSeq::ParenSeq(std::string sourcePath, std::string source) {
    this->sourcePath = sourcePath;
    this->source = source;
}

// 括弧が渡されたら開き括弧と閉じ括弧を判断する
std::vector<Token> ParenSeq::getOrderedParens(std::vector<Token> tokens) {
    //int len = tokens.size();
    ByteSeq leftParens = { TK_LeftParen, TK_LeftBracket, TK_LeftBrace };
    ByteSeq rightParens = { TK_RightParen, TK_RightBracket, TK_RightBrace };

    for(int i = 0; i < tokens.size(); i++) {
        /*if(tokens[i].match(ByteSeq { TK_LeftParen, TK_LeftBracket, TK_LeftBrace })) {
            this->addOpenParen(tokens[i]);
        } else if(tokens[i].match(ByteSeq { TK_RightParen, TK_RightBracket, TK_RightBrace })) {
            this->addCloseParen(tokens[i]);
        } else if(TM())*/

        if(TMR(i, leftParens)) {
            this->addOpenParen(tokens[i]);
        } else if(TMR(i, rightParens)) {
            this->addCloseParen(tokens[i]);
        } else if(TM(i, TK_Comma)) {
            std::cout << "..." << std::endl;
        }
    }

    this->checkCloseParensFinally();
    return this->removeSurroundingParens(this->parens);
}

// 閉じ括弧が渡されたら、不正な括弧がないかを確認してネストを書き変える
void ParenSeq::addCloseParen(Token token) {
    Token expectingCloseParen = this->latestOpenParen.getOpenParen();
    int nest = this->nestOfParens[this->getNestIndex(token.type)];

    // 階層が0以下なのでエラー
    if(nest <= 0) {
        Console::log(LogType_Error, 7904, { { "At", token.getPositionText(this->sourcePath, this->source) }, { "Unexpected", token.string } });
        return;
    }

    // 最後に開いた括弧の種類が異なるのでエラー
    if(token.type == expectingCloseParen.type) {
        Console::log(LogType_Error, 7904, { { "At", token.getPositionText(this->sourcePath, this->source) }, { "Unexpected", token.string }, { "Expected", expectingCloseParen.string } });
        return;
    }

    this->parens.push_back(token);
    this->nestOfParens[this->getNestIndex(token.type)]--;
}

// 開き括弧が渡されたら、不正な括弧がないかを確認してネストを書き変える
void ParenSeq::addOpenParen(Token token) {
    this->latestOpenParen = token;
    this->parens.push_back(token);
    this->nestOfParens[this->getNestIndex(token.type)]++;
}

// 一番最後に閉じられてない括弧がないかチェック
void ParenSeq::checkCloseParensFinally() {
    // parensを表示
    /*for(int i = 0; i < parens.size(); i++)
        std::cout<<parens[i].string<<std::endl;*/

    std::string expectedParen = "";

    if(this->nestOfParens[0] > 0)
        expectedParen = ")";
    else if(this->nestOfParens[1] > 0)
        expectedParen = "]";
    else if(this->nestOfParens[2] > 0)
        expectedParen = "}";

    if(expectedParen != "")
        Console::log(LogType_Error, 5916, { { "Expected", expectedParen } });
}

// 外側を取り除いた括弧が不正でないかチェックする
// もし不正ならば取り除いていない状態の括弧を返す
std::vector<Token> ParenSeq::removeSurroundingParens(std::vector<Token> tokens) {
    // 渡された括弧の数が1以下ならばtokensを返す
    if(tokens.size() <= 1)
        return tokens;

    std::vector<Token> insideParens;

    // 内側の括弧を取り出す
    for(int i = 1; i < tokens.size() - 1; i++)
        insideParens.push_back(tokens[i]);

    // 内側の括弧の数が1以下ならばtokensを返す
    if(insideParens.size() <= 1)
        return tokens;

    // 最初または最後の括弧が不正ならばtokensを返す
    if(insideParens[0].getOpenParen().type != TK_Unknown
            && insideParens[insideParens.size() - 1].getCloseParen().type != TK_Unknown)
        return tokens;

    // 括弧が不正でなければチェックを繰り返す
    return ParenSeq::removeSurroundingParens(insideParens);
}

int ParenSeq::getNestIndex(Byte type) {
    switch(type) {
        case TK_LeftParen:
        case TK_RightParen:
        return 0;

        case TK_LeftBracket:
        case TK_RightBracket:
        return 1;

        case TK_LeftBrace:
        case TK_RightBrace:
        return 2;

        default:
        return -1;
    }
}



Parser::Parser() {}

Parser::Parser(std::string sourcePath, std::string source, std::vector<Token> tokens) {
    this->sourcePath = sourcePath;
    this->source = source;
    this->tokens = tokens;
    this->lines = this->getLines();
}

std::vector<Line> Parser::getLines() {
    std::vector<Line> lines;
    int index = 0;

    while(true) {
        std::vector<Token> ln;

        while(true) {
            Token tk = this->tokens[index];

            if(tk.type == TK_EndOfFile || tk.type == TK_NewLine) {
                index++;
                break;
            } else {
                ln.push_back(tk);
                index++;
            }
        }

        if(ln.size() > 0)
            lines.push_back(Line(ln));

        if(index >= this->tokens.size())
            break;
    }

    return lines;
}

Node Parser::parse() {
    for(; this->lineIndex < this->lines.size(); )
        this->tree.addChild(this->scanNextLine());

    this->tree.print();
    return this->tree;
}

Node Parser::scanNextLine() {
    Node node = this->getNode(CURR_LINE);
    // lineIndexはノード取得後に変更してください
    this->lineIndex++;
    return node;
}

Node Parser::scanNextNest(Byte nodeType) {
    Node node(nodeType);
    Line baseLine = CURR_LINE;

    while(true) {
        this->lineIndex++;

        // 行インデックスがLineのサイズを超える場合
        if(this->lineIndex >= this->lines.size())
            break;

        Line currentLine = CURR_LINE;
        int nestDeff = baseLine.nest - currentLine.nest;

        // ネストがベース行と同じか、それよりも浅い場合
        if(nestDeff >= 0) {
            this->lineIndex--;
            break;
        }

        // ネストがチェック中のネストよりも深い場合
        // scanNextLine内でscanNextNestが呼ばれるべき
        // よりネストの深い行はチェックする必要なし

        if(nestDeff < -1)
            continue;

        node.addChild(this->getNode(currentLine));
    }

    return node;
}

Node Parser::getNode(Line line, Byte defaultType) {
    return this->getNode(line.tokens, line.nest, defaultType);
}

Node Parser::getNode(std::vector<Token> tokens, int nest, Byte defaultType) {
    try {
        int len = tokens.size();

        if(len == 0)
            return Node(ND_Unknown);

        /*std::cout << "tk: ";
        for(Token tk : tokens)
            std::cout << tk.string << " ";
        std::cout << std::endl;*/

        // DEFFUNC
        if(nest == 0 && TM(0, TK_Identifier) && TM(1, TK_LeftParen) && TM(-1, TK_RightParen)) {
            Node node(ND_DefFunc);
            node.addToken(Token(TK_Identifier, SA(0)));
            Node args(ND_Args);

            if(len >= 4) {
                std::vector<Token> ag;
                int nestInArgs = 0;

                for(int i = 2; i < len - 1; i++) {
                    // todo: コンマを括弧チェック後に変換させる？
                    if(TM(i, TK_Comma) && nestInArgs == 0) {
                        node.addChild(this->getNode(ag));
                        ag.clear();
                    } else if(i == len - 2) {
                        ag.push_back(A(i));
                    } else {
                        if(A(i).match(ByteSeq { TK_LeftParen, TK_LeftBracket, TK_LeftBrace }))
                            nestInArgs++;
                        else if(A(i).match(ByteSeq { TK_RightParen, TK_RightBracket, TK_RightBrace }))
                            nestInArgs--;
                        ag.push_back(A(i));
                    }
                }
            }

            node.addChild(args);
            node.addChild(this->scanNextNest());
            return node;
        }

        // CALLFUNC
        if(nest >= 1 && len >= 3 && TM(0, TK_Identifier) && TM(1, TK_LeftParen) && TM(-1, TK_RightParen)) {
            Node node(ND_CallFunc);
            node.addToken(A(0));

            int nest = 0;

            if(len >= 4) {
                Node args(ND_Args);
                std::vector<Token> ag;

                for(int i = 2; i < len - 1; i++) {
                    if(TM(i, TK_Comma) && nest == 0) {
                        args.addChild(this->getNode(ag));
                        ag.clear();
                    } else if(i == len - 2) {
                        ag.push_back(A(i));
                        args.addChild(getNode(ag));
                    } else {
                        if(A(i).isOpenParen())
                            nest++;
                        else if(A(i).isCloseParen())
                            nest--;

                        ag.push_back(A(i));
                    }
                }

                node.addChild(args);
            }

            return node;
        }

        // DEFVAR
        if(len == 2 && (TM(0, TK_Identifier) || TM(0, TK_Keyword)) && TM(1, TK_Identifier)) {
            Node node(ND_DefVar);
            node.addToken(A(0));
            node.addToken(A(1));
            return node;
        }

        // DEFVAR (ARRAY)
        if(len == 4 && (TM(0, TK_Identifier) || TM(0, TK_Keyword)) && TM(1, TK_LeftBracket) && TM(2, TK_RightBracket) && TM(3, TK_Identifier)) {
            Node node(ND_DefVar);
            node.addToken(A(0));
            node.addToken(A(1));
            node.addToken(A(2));
            node.addToken(A(3));
            return node;
        }

        // IF
        if(len >= 2 && M(0, TK_Keyword, "if")) {
            Node node(ND_If);
            node.addChild(this->getNode(this->copy(1, len - 1, tokens)));
            node.addChild(this->scanNextNest());
            return node;
        }

        // ELSE
        if(len == 1 && M(0, TK_Keyword, "else")) {
            Node node(ND_Else);
            node.addChild(this->scanNextNest());
            return node;
        }

        // INITVAR todo: 複数トークンの型名にも対応させる
        if(len >= 4 && (TM(0, TK_Identifier) || TM(0, TK_Keyword)) && TM(1, TK_Identifier) && TM(2, TK_Equal)) {
            Node node(ND_InitVar);
            node.addToken(A(0));
            node.addToken(A(1));

            node.addChild(this->getNode(copy(3, len - 1, tokens)));

            /*if(len == 4) {
                node.addToken(A(3));
            } else {
                std::vector<Token> rs;
                for(int i = 3; i < len; i++)
                    rs.push_back(A(i));
                node.addChild(getNode(rs));
            }*/

            return node;
        }

        // LOOP
        if(len >= 4 && M(0, TK_Keyword, "for") && TM(1, TK_LeftParen) && TM(-1, TK_RightParen)) {
            Node node(ND_Loop);
            std::vector<std::vector<Token>> semicolonDiv = {};

            for(Token tk : tokens)
                if(tk.type == TK_Semicolon)
                    semicolonDiv.push_back({});
                else
                    semicolonDiv[semicolonDiv.size() - 1].push_back(tk);

            if(semicolonDiv.size() == 1) {
                // Example: for(true)
                node.addChild(this->getNode(copy(1, len - 1, tokens), ND_Loop_Cond));
            } else if(semicolonDiv.size() == 3) {
                // Example: for(i = 0; i < 5; i++)
                std::vector<Token> item;

                node.addChild(this->getNode(semicolonDiv[0], ND_Loop_Init));
                node.addChild(this->getNode(semicolonDiv[1], ND_Loop_Cond));
                node.addChild(this->getNode(semicolonDiv[2], ND_Loop_Change));
            } else {
                //構文エラー
            }

            return node;
        }



        /* 括弧チェック */

        ParenSeq parenSeq(this->sourcePath, this->source);
        std::vector<Token> orderedTokens = parenSeq.getOrderedParens(tokens);

        len = tokens.size();
        bool enclosed;
        bool containsParen;

        /*std::cout << "\ttk exp: ";
        for(Token t : tokens)
            std::cout << t.string << " ";
        std::cout << std::endl;*/

        /* 論理式 */

        Node logicExpNode = this->getLogicalExpressionNode(tokens);

        if(logicExpNode.type != ND_Unknown)
            return logicExpNode;

        /* 比較式 */

        Node compExpNode = this->getCompareExpressionNode(tokens);

        if(compExpNode.type != ND_Unknown)
            return compExpNode;

        /* 計算式 */



        // トークンの長さが１かどうかは、最後にチェックしてください
        // (最初にチェックすると、else文などが弾かれてしまうため)
        if(len == 1)
            return Node(defaultType, tokens);

    } catch(std::out_of_range ignored) {
        std::cout << "EXCEPTION" << std::endl;
    }

    // todo: add an error 'unknown syntax'
    return Node(ND_Unknown);
}

// 式でなかった場合のtypeはUNKNOWN
// ここで判断
// 変数'len' および 引数名'tokens' は変更しないでください (A(i)を使用するため)
Node Parser::getLogicalExpressionNode(std::vector<Token> tokens) {
    Node node(ND_Logic);
    std::vector<Token> side;
    int len = tokens.size();
    int nest = 0;

    for(int i = 0; i < tokens.size(); i++) {
        switch(A(i).type) {
            case TK_Ampersand:
            case TK_Pipe:
            if(nest == 0 && i < len - 1 && TA(i) == TA(i + 1)) {
                node.addChild(this->getNode(side, ND_Root)); // N_ITEM
                side.clear();

                // TA(i) と TA(i + 1) は同じなので、ここでチェックする必要なし
                if(TM(i, TK_Pipe)) {
                    node.addChild(Node(ND_Operator, {}, { A(i) }));
                    i++;
                } else if(TM(i, TK_Ampersand)) {
                    node.addChild(Node(ND_Operator, {}, { A(i) }));
                    i++;
                }
            }
            break;

            default:
            if(A(i).isOpenParen())
                nest++;
            else if(A(i).isCloseParen())
                nest--;

            side.push_back(A(i));
            break;
        }
    }

    if(node.children.size() > 0)
        return node;
    else
        return Node(ND_Unknown);
}

Node Parser::getCompareExpressionNode(std::vector<Token> tokens) {
    Node node(ND_Compare);
    Byte opeType = ND_Unknown;
    std::vector<Token> leftside;
    std::vector<Token> rightside;
    int len = tokens.size();
    int nest = 0;

    for(int i = 0; i < tokens.size(); i++) {//std::cout<<(int)opeType<<" "<<i<<" "<<tokens.size()<<std::endl;
        if(opeType == ND_Unknown) {
            // 左辺: まだ比較演算子がきていない
            if(A(i).isOpenParen()) {
                nest++;
            } else if(A(i).isCloseParen()) {
                nest--;
            } else {
                opeType = this->getOpeType(tokens, i);

                switch(opeType) {
                    case ND_Unknown:
                    leftside.push_back(A(i));
                    break;

                    case ND_Less:
                    case ND_Greater:
                    break;

                    case ND_Equal:
                    case ND_LessEqual:
                    case ND_GreaterEqual:
                    i++;
                    break;
                }
            }
        } else {
            // 右辺: 比較演算子がすでにきた
            rightside.push_back(A(i));
        }
    }

    /*for(Token tk : leftside)
        std::cout<<"lt: "<<tk.string<<std::endl;*/

    /*for(Token tk : rightside)
        std::cout<<"rt: "<<tk.string<<std::endl;*/

    if(leftside.size() == 0 || rightside.size() == 0)
        return Node(ND_Unknown);

    node.addChild(this->getNode(leftside));
    node.addChild(this->getNode(rightside));

    if(node.childAt(0).type != ND_Unknown && node.childAt(1).type != ND_Unknown)
        return Node(ND_Unknown);

    return node;
}

// 引数名'tokens' および 変数'len' を変更しないでください (マクロ'A(i)' を使用するため)
Byte Parser::getOpeType(std::vector<Token> tokens, int index) {
    int len = tokens.size();
    Byte tokenType = TA(index);

    if(index < len - 1) {
        // 2文字の演算子

        Byte secondTokenType = TA(index + 1);

        if(tokenType == TK_Equal && secondTokenType == TK_Equal)
            return ND_Equal;
        else if(tokenType == TK_LeftAngleBracket && secondTokenType == TK_Equal)
            return ND_LessEqual;
        else if(tokenType == TK_RightAngleBracket && secondTokenType == TK_Equal)
            return ND_GreaterEqual;

        // N_EQUALなどの場合は呼び出し元で判断してi++する
    }

    // 1文字の演算子

    if(tokenType == TK_LeftAngleBracket)
        return ND_Less;
    else if(tokenType == TK_RightAngleBracket)
        return ND_Greater;

    // unknownの場合は leftside.push_back(A(i)); をする
    return ND_Unknown;
}

// 優先度高い             優先度低い
// true → ope1 < ope2   false → ope1 >= ope2
bool Parser::compareOpe(std::string ope1, std::string ope2) {
    if(std::regex_match(ope1, std::regex("[~]")))
        return (std::regex_match(ope2, std::regex("[+\\-*/]")));

    if(std::regex_match(ope1, std::regex("[+\\-]")))
        return (std::regex_match(ope2, std::regex("[*/]")));

    if(std::regex_match(ope1, std::regex("[*/]")))
        return (std::regex_match(ope2, std::regex("[]")));

    return false;
}

std::vector<Token> Parser::copy(int begin, int length, std::vector<Token> src) {
    std::vector<Token> res;

    for(int i = begin; i < begin + length; i++)
        res.push_back(src.at(i));

    return res;
}
