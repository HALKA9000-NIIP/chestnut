#pragma once

#include "../shared/command.cpp"

#include "lexer.hpp"



Lexer::Lexer() {}

Lexer::Lexer(std::string filePath, std::string source, Command cmd) {
    this->filePath = filePath;
    this->source = source;
    this->cmd = cmd;
}

std::vector<Token> Lexer::splitTokens() {
    std::vector<Token> tokenList;
    Token token;

    do {
        token = getNextToken();
        tokenList.push_back(token);
        //std::cout << (int)token.type << "\t" << ((token.string == "\n") ? "\\n" : token.string) << std::endl;//
    } while(token.type != TK_EndOfFile);

    // 最後に括弧のエラーを出す
    for(Token paren : openParens) {
        Console::log(LogType_Error, 698, { { "At", Token::getPositionText(this->cmd.at("-i"), source, paren.index + 1) }, { "Expected", "'" + paren.getCloseParen().string + "'" } });
    }

    if(Console::errored || (Console::warned && !this->cmd.existsKey("-miss")))
        exit(-1);

    return tokenList;
}

Token Lexer::getNextToken() {
    index++;

    #define MATCH_STR(ch1)                  (source[index] == ch1)
    #define MATCH_STR_2(ch1, ch2)           (index < source.length() - 1 && source[index] == ch1 && source[index + 1] == ch2)
    #define MATCH_STR_3(ch1, ch2, ch3)      (index < source.length() - 3 && source[index] == ch1 && source[index + 1] == ch2 && source[index + 2] == ch3)
    #define MATCH_STR_4(ch1, ch2, ch3, ch4) (index < source.length() - 3 && source[index] == ch1 && source[index + 1] == ch2 && source[index + 2] == ch3 && source[index + 3] == ch4)
    #define MATCH_STR_M1(ch1)               (index > 0 && source[index - 1] == ch1)

    if(index >= source.size())
        return Token(TK_EndOfFile, "", index);

    else if(MATCH_STR('!'))
        return Token(TK_Exclamation, std::string { source[index] }, index);

    else if(MATCH_STR('?'))
        return Token(TK_Question, std::string { source[index] }, index);

    else if(MATCH_STR('~'))
        return Token(TK_Tilde, std::string { source[index] }, index);

    else if(MATCH_STR('+'))
        return Token(TK_Plus, std::string { source[index] }, index);

    else if(MATCH_STR('-'))
        return Token(TK_Hyphen, std::string { source[index] }, index);

    else if(MATCH_STR_2('*', '/'))
        Console::log(LogType_Error, 699, { { "At", Token::getPositionText(this->cmd.at("-i"), source, index++) }, { "Unexpected", "*/" } });

    else if(MATCH_STR('*'))
        return Token(TK_Asterisk, std::string { source[index] }, index);

    else if(MATCH_STR_2('/', '*')) {
        std::string comment;
        int start = index;

        for(index++; index < source.length() - 1; index++) {
            if(source[index] == '*' && source[index + 1] == '/') {
                index++;
                return Token(TK_Comment, comment, start);
            }

            comment += std::string { source[index] };
        }

        //checkParenFinally();

        // コメントアウトに閉じがない場合はエラー
        Console::log(LogType_Error, 1562, { { "At", Token::getPositionText(this->cmd.at("-i"), source, start + 2) }, { "Expected", "'*/'" } }, true);
    }

    else if(MATCH_STR_2('/', '/')) {
        std::string comment;
        int start = index;

        for(index++; index < source.length(); index++) {
            if(source[index] == '\n') {
                index--;
                break;
            }

            comment += std::string { source[index] };
        }

        return Token(TK_Comment, comment, start);
    }

    else if(MATCH_STR('/'))
        return Token(TK_Slash, std::string { source[index] }, index);

    else if(MATCH_STR('%'))
        return Token(TK_Percent, std::string { source[index] }, index);

    else if(MATCH_STR('^'))
        return Token(TK_Tilde, std::string { source[index] }, index);

    else if(MATCH_STR('='))
        return Token(TK_Equal, std::string { source[index] }, index);

    else if(MATCH_STR('|'))
        return Token(TK_Pipe, std::string { source[index] }, index);

    else if(MATCH_STR('&'))
        return Token(TK_Ampersand, std::string { source[index] }, index);

    else if(MATCH_STR('.'))
        return Token(TK_Period, std::string { source[index] }, index);

    else if(MATCH_STR(','))
        return Token(TK_Comma, std::string { source[index] }, index);

    else if(MATCH_STR(':'))
        return Token(TK_Colon, std::string { source[index] }, index);

    else if(MATCH_STR(';'))
        return Token(TK_Semicolon, std::string { source[index] }, index);

    else if(MATCH_STR('(')) {
        Token tk = Token(TK_LeftParen, std::string { source[index] }, index);
        validateParen(tk);
        return tk;
    }

    else if(MATCH_STR(')')) {
        Token tk = Token(TK_RightParen, std::string { source[index] }, index);
        validateParen(tk);
        return tk;
    }

    else if(MATCH_STR('[')) {
        Token tk = Token(TK_LeftBracket, std::string { source[index] }, index);
        validateParen(tk);
        return tk;
    }

    else if(MATCH_STR(']')) {
        Token tk = Token(TK_RightBracket, std::string { source[index] }, index);
        validateParen(tk);
        return tk;
    }

    else if(MATCH_STR('<'))
        return Token(TK_LeftAngleBracket, std::string { source[index] }, index);

    else if(MATCH_STR('>'))
        return Token(TK_RightAngleBracket, std::string { source[index] }, index);

    else if(MATCH_STR('{')) {
        Token tk = Token(TK_LeftBrace, std::string { source[index] }, index);
        validateParen(tk);
        return tk;
    }

    else if(MATCH_STR('}')) {
        Token tk = Token(TK_RightBrace, std::string { source[index] }, index);
        validateParen(tk);
        return tk;
    }

    else if(MATCH_STR(' ')) {
        // インデントでない場合は次のトークンを返す
        for(int i = index - 1; i >= 0; i--) {
            if(source[i] == ' ') {
                continue;
            } else if(source[i] == '\n') {
                break;
            } else {
                return getNextToken();
            }
        }

        int start = index;

        if(MATCH_STR_4(' ', ' ', ' ', ' ')) {
            // 正しいインデントの場合
            index += 3;
            return Token(TK_Indent, "    ", start);
        } else {
            // インデントの数が不正な場合
            for(index++; index < source.length(); index++) {
                if(!MATCH_STR(' ')) {
                    index--;
                    break;
                }
            }

            Console::log(LogType_Error, 7903, { { "At", Token::getPositionText(this->cmd.at("-i"), source, start) } });
            return getNextToken();
        }
    }

    else if(MATCH_STR('\n')) {
        for(index++; index < source.length(); index++) {
            if(!MATCH_STR('\n')) {
                index--;
                break;
            }
        }

        return Token(TK_NewLine, std::string { source[index] }, index);
    }

    else if(std::regex_match(std::string { source[index] }, std::regex("[0-9]"))) {
        std::string num;
        int start = index;

        for(; index < source.length(); index++) {
            if(std::regex_match(std::string { source[index] }, std::regex("[0-9]"))) {
                num += std::string { source[index] };
            } else {
                index--; break;
            }
        }

        return Token(TK_Number, num, start);
    }

    else if(MATCH_STR('\'')) {
        std::string ch = "";
        int start = index;

        if(source[index + 2] == '\'') {
            // 普通のchar値
            ch = std::string { source[index + 1] };
            index += 2;
        } else if(source[index + 1] == '\\' && source[index + 3] == '\'') {
            // エスケープを含むchar値
            ch = "\\" + std::string { source[index + 2] };
            index += 3;
        } else if(source[index + 1] == '\'') {
            // 文字の入っていない文字値
            Console::log(LogType_Error, 4139, { { "At", Token::getPositionText(this->cmd.at("-i"), source, start + 1) }, { "Expected", "{$LogDetailValue_ACharValue}" } });
            index += 1;
        } else {
            bool closed = false;
            int i;

            for(index++; index < source.length(); index++) {
                if(MATCH_STR('\'')) {
                    closed = true;
                    break;
                }
            }

            if(closed) {
                // 文字値が2文字以上の場合 (tooLongCharacterLengthエラー)
                Console::log(LogType_Error, 2471, { { "At", Token::getPositionText(this->cmd.at("-i"), source, start + 1) } });
            } else {
                //checkParenFinally();
                // 閉じクォーテーションがない場合 (EOFエラー)
                Console::log(LogType_Error, 1562, { { "At", Token::getPositionText(this->cmd.at("-i"), source, start + 2) }, { "Expected", "'''" } }, true);
            }

            return getNextToken();
        }

        return Token(TK_Char, std::string { ch }, start);
    }

    else if(MATCH_STR('\"')) {
        std::string str;
        int start = index;

        for(index++; index < source.length(); index++) {
            if(source[index] != '\"') {
                str += source[index];
            } else {
                return Token(TK_String, str, start);
            }
        }

        //checkParenFinally();
        Console::log(LogType_Error, 1562, { { "At", Token::getPositionText(this->cmd.at("-i"), source, start + 1) }, { "Expected", "'\"'" } }, true);
    }

    else {
        std::string tk;
        int start = index;

        while(index < source.length()) {
            // 正規表現では字句解析で判定済みの文字のみを使用してください (無限ループ防止)
            if(!std::regex_match(std::string { source[index] }, std::regex("[!?~+\\-*/%^=&|.,:;()\\[\\]<>{}'\" \n]"))) {
                tk += source[index];
                index++;
            } else {
                index--;
                break;
            }
        }

        if(std::regex_match(tk, std::regex("bol|break|byt|default|case|catch|chr|class|continue|dbl|elif|else|false|flt|for|if|import|int|lon|new|null|obj|package|private|public|return|sht|str|switch|true|ubyt|usht|uint|ulon|void"))) {
            return Token(TK_Keyword, tk, start);
        } else {
            return Token(TK_Identifier, tk, start);
        }
    }

    return Token(TK_Unknown, std::string { source[index] });
}

void Lexer::validateParen(Token token) {
    if(token.match(ByteSeq { TK_LeftParen, TK_LeftBracket, TK_LeftBrace })) {
        nest.at(token.type).first++;
        nest.at(token.type).second.push_back(token);
        openParens.push_back(token);
        return;
    }

    if(token.match(ByteSeq { TK_RightParen, TK_RightBracket, TK_RightBrace })) {
        if(openParens.size() == 0) {
            Console::log(LogType_Error, 699, { { "At", token.getPositionText(filePath, source) }, { "Unexpected", "'" + token.string + "'" } });
            return;
        }

        Token paren = token.getOpenParen();
        Token latestOpenParen = openParens.back();

        if(latestOpenParen.type == TK_Unknown) {
            Console::log(LogType_Error, 699, { { "At", token.getPositionText(filePath, source) }, { "Unexpected", "'" + token.string + "'" } });
            return;
        }

        if(latestOpenParen.type != paren.type) {
            Console::log(LogType_Error, 698, { { "At", Token::getPositionText(this->cmd.at("-i"), source, latestOpenParen.index + 1) }, { "Expected", "'" + latestOpenParen.getCloseParen().string + "'" } });
            nest.at(paren.type).first--;
            nest.at(latestOpenParen.type).first--;
            openParens.pop_back();
            //openParens.pop_back();
            return;
        }

        if(nest.at(paren.type).first < 0) {
            Console::log(LogType_Error, 699, { { "At", token.getPositionText(filePath, source) }, { "Unexpected", "'" + token.string + "'" } });
            return;
        }

        nest.at(paren.type).first--;
        openParens.pop_back();
    }
}
