#include "syntaxhighlighter.h"

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{}

void SyntaxHighlighter::setLanguage(const QString &language) {
    buildRules(language);
    rehighlight();
}

static QTextCharFormat fmt(const QString &color, bool bold = false, bool italic = false) {
    QTextCharFormat f;
    f.setForeground(QColor(color));
    if (bold)   f.setFontWeight(QFont::Bold);
    if (italic) f.setFontItalic(true);
    return f;
}

void SyntaxHighlighter::buildRules(const QString &language) {
    m_rules.clear();
    m_hasBlockComment = false;

    // Color palette (VS Code Dark+ inspired)
    const QString clrKeyword  = "#569cd6"; // blue
    const QString clrType     = "#4ec9b0"; // teal
    const QString clrString   = "#ce9178"; // orange-brown
    const QString clrComment  = "#6a9955"; // green
    const QString clrNumber   = "#b5cea8"; // light green
    const QString clrPreproc  = "#c586c0"; // purple
    const QString clrFunc     = "#dcdcaa"; // yellow
    const QString clrBuiltin  = "#9cdcfe"; // light blue

    m_mlCommentFmt = fmt(clrComment, false, true);

    // --- C family ---
    static const QStringList cFamily = {
        "C", "C++", "C#", "Java", "JavaScript", "TypeScript",
        "Kotlin", "Swift", "PHP", "Rust", "Go", "Zig", "Scala"
    };

    if (cFamily.contains(language)) {
        // Keywords
        QStringList keywords;
        if (language == "Rust") {
            keywords = {"fn","let","mut","pub","use","mod","struct","enum","impl",
                        "trait","where","for","in","if","else","match","return",
                        "loop","while","break","continue","self","super","crate",
                        "true","false","None","Some","Ok","Err","async","await",
                        "move","ref","const","static","type","unsafe","extern"};
        } else if (language == "Go") {
            keywords = {"func","var","const","type","struct","interface","map",
                        "chan","go","defer","select","case","default","break",
                        "continue","fallthrough","return","if","else","for","range",
                        "switch","import","package","nil","true","false"};
        } else if (language == "C#") {
            keywords = {"abstract","as","base","bool","break","byte","case","catch",
                        "char","checked","class","const","continue","decimal","default",
                        "delegate","do","double","else","enum","event","explicit",
                        "extern","false","finally","fixed","float","for","foreach",
                        "goto","if","implicit","in","int","interface","internal","is",
                        "lock","long","namespace","new","null","object","operator","out",
                        "override","params","private","protected","public","readonly",
                        "ref","return","sbyte","sealed","short","sizeof","stackalloc",
                        "static","string","struct","switch","this","throw","true",
                        "try","typeof","uint","ulong","unchecked","unsafe","ushort",
                        "using","virtual","void","volatile","while","async","await",
                        "var","dynamic","yield"};
        } else if (language == "JavaScript" || language == "TypeScript") {
            keywords = {"break","case","catch","class","const","continue","debugger",
                        "default","delete","do","else","export","extends","false",
                        "finally","for","function","if","import","in","instanceof",
                        "let","new","null","return","super","switch","this","throw",
                        "true","try","typeof","undefined","var","void","while","with",
                        "yield","async","await","of","from","static","get","set"};
            if (language == "TypeScript")
                keywords << "type" << "interface" << "enum" << "namespace"
                         << "abstract" << "implements" << "declare" << "readonly"
                         << "as" << "is" << "keyof" << "never" << "unknown" << "any";
        } else {
            // C / C++ / Java / Swift / Kotlin / PHP / Zig / Scala
            keywords = {"auto","break","case","catch","class","const","continue",
                        "default","delete","do","else","enum","explicit","extern",
                        "false","final","finally","for","friend","goto","if","inline",
                        "long","namespace","new","nullptr","operator","override",
                        "private","protected","public","return","short","sizeof",
                        "static","struct","switch","template","this","throw","true",
                        "try","typedef","union","unsigned","using","virtual","void",
                        "volatile","while","import","package","extends","implements",
                        "interface","abstract","synchronized","native","transient",
                        "strictfp","assert","instanceof","super","int","float","double",
                        "bool","char","string","String","var","val","fun","object",
                        "companion","data","sealed","open","internal","when","init",
                        "constructor","by","lazy","it","null","let","mut","in","is",
                        "as","defer","guard","where","typealias","protocol","extension",
                        "computed","willSet","didSet","inout","some","any"};
        }

        Rule kw;
        kw.pattern = QRegularExpression("\\b(" + keywords.join("|") + ")\\b");
        kw.format  = fmt(clrKeyword, true);
        m_rules << kw;

        // Types / primitives
        Rule ty;
        ty.pattern = QRegularExpression("\\b(int|float|double|char|bool|void|long|short|"
                                        "unsigned|signed|size_t|uint8_t|uint16_t|uint32_t|"
                                        "uint64_t|int8_t|int16_t|int32_t|int64_t|"
                                        "String|str|u8|u16|u32|u64|i8|i16|i32|i64|"
                                        "f32|f64|usize|isize)\\b");
        ty.format  = fmt(clrType);
        m_rules << ty;

        // Preprocessor (C/C++)
        if (language == "C" || language == "C++") {
            Rule pp;
            pp.pattern = QRegularExpression("^\\s*#\\w+");
            pp.format  = fmt(clrPreproc);
            m_rules << pp;
        }

        // Function calls
        Rule fn;
        fn.pattern = QRegularExpression("\\b([a-zA-Z_][\\w]*)(?=\\s*\\()");
        fn.format  = fmt(clrFunc);
        m_rules << fn;

        // Double-quoted strings
        Rule dqs;
        dqs.pattern = QRegularExpression("\"(?:[^\"\\\\]|\\\\.)*\"");
        dqs.format  = fmt(clrString);
        m_rules << dqs;

        // Single-quoted strings / chars
        Rule sqs;
        sqs.pattern = QRegularExpression("'(?:[^'\\\\]|\\\\.)*'");
        sqs.format  = fmt(clrString);
        m_rules << sqs;

        // Numbers
        Rule num;
        num.pattern = QRegularExpression("\\b(0x[0-9a-fA-F]+|\\d+\\.?\\d*([eE][+-]?\\d+)?)\\b");
        num.format  = fmt(clrNumber);
        m_rules << num;

        // Line comment //
        Rule lc;
        lc.pattern = QRegularExpression("//[^\n]*");
        lc.format  = fmt(clrComment, false, true);
        m_rules << lc;

        // Block comment /* */
        m_hasBlockComment  = true;
        m_mlCommentStart   = QRegularExpression("/\\*");
        m_mlCommentEnd     = QRegularExpression("\\*/");

    } else if (language == "Python") {
        QStringList kws = {"False","None","True","and","as","assert","async","await",
                           "break","class","continue","def","del","elif","else","except",
                           "finally","for","from","global","if","import","in","is",
                           "lambda","nonlocal","not","or","pass","raise","return",
                           "try","while","with","yield"};
        Rule kw;
        kw.pattern = QRegularExpression("\\b(" + kws.join("|") + ")\\b");
        kw.format  = fmt(clrKeyword, true);
        m_rules << kw;

        // Builtins
        Rule bi;
        bi.pattern = QRegularExpression("\\b(print|len|range|type|isinstance|int|str|"
                                        "float|list|dict|set|tuple|bool|open|input|"
                                        "enumerate|zip|map|filter|sorted|reversed|"
                                        "super|property|staticmethod|classmethod|"
                                        "self|cls)\\b");
        bi.format  = fmt(clrBuiltin);
        m_rules << bi;

        // Decorators
        Rule dec;
        dec.pattern = QRegularExpression("@[\\w.]+");
        dec.format  = fmt(clrPreproc);
        m_rules << dec;

        // Triple-quoted strings (simplified)
        Rule tqs;
        tqs.pattern = QRegularExpression("\"\"\".*?\"\"\"|'''.*?'''");
        tqs.format  = fmt(clrString);
        m_rules << tqs;

        // Regular strings
        Rule s1;
        s1.pattern = QRegularExpression("\"(?:[^\"\\\\]|\\\\.)*\"|'(?:[^'\\\\]|\\\\.)*'");
        s1.format  = fmt(clrString);
        m_rules << s1;

        // f-strings prefix
        Rule fs;
        fs.pattern = QRegularExpression("f\"(?:[^\"\\\\]|\\\\.)*\"|f'(?:[^'\\\\]|\\\\.)*'");
        fs.format  = fmt(clrString);
        m_rules << fs;

        Rule num;
        num.pattern = QRegularExpression("\\b(0x[0-9a-fA-F]+|\\d+\\.?\\d*)\\b");
        num.format  = fmt(clrNumber);
        m_rules << num;

        Rule cmt;
        cmt.pattern = QRegularExpression("#[^\n]*");
        cmt.format  = fmt(clrComment, false, true);
        m_rules << cmt;

    } else if (language == "Lua") {
        QStringList kws = {"and","break","do","else","elseif","end","false","for",
                           "function","goto","if","in","local","nil","not","or",
                           "repeat","return","then","true","until","while"};
        Rule kw;
        kw.pattern = QRegularExpression("\\b(" + kws.join("|") + ")\\b");
        kw.format  = fmt(clrKeyword, true);
        m_rules << kw;

        Rule bi;
        bi.pattern = QRegularExpression("\\b(print|pairs|ipairs|tostring|tonumber|type|"
                                        "require|pcall|xpcall|error|assert|setmetatable|"
                                        "getmetatable|rawget|rawset|select|unpack|"
                                        "table|string|math|io|os|coroutine)\\b");
        bi.format  = fmt(clrBuiltin);
        m_rules << bi;

        Rule str;
        str.pattern = QRegularExpression("\"(?:[^\"\\\\]|\\\\.)*\"|'(?:[^'\\\\]|\\\\.)*'");
        str.format  = fmt(clrString);
        m_rules << str;

        Rule num;
        num.pattern = QRegularExpression("\\b(0x[0-9a-fA-F]+|\\d+\\.?\\d*)\\b");
        num.format  = fmt(clrNumber);
        m_rules << num;

        Rule cmt;
        cmt.pattern = QRegularExpression("--[^\n]*");
        cmt.format  = fmt(clrComment, false, true);
        m_rules << cmt;

        m_hasBlockComment = true;
        m_mlCommentStart  = QRegularExpression("--\\[\\[");
        m_mlCommentEnd    = QRegularExpression("\\]\\]");

    } else if (language == "Bash" || language == "Shell" || language == "PowerShell") {
        // Variables
        Rule var;
        var.pattern = QRegularExpression("\\$[\\{]?[a-zA-Z_][\\w]*[\\}]?");
        var.format  = fmt(clrBuiltin);
        m_rules << var;

        // Strings
        Rule str;
        str.pattern = QRegularExpression("\"(?:[^\"\\\\]|\\\\.)*\"|'[^']*'");
        str.format  = fmt(clrString);
        m_rules << str;

        // Shebang
        Rule shebang;
        shebang.pattern = QRegularExpression("^#!.*");
        shebang.format  = fmt(clrPreproc);
        m_rules << shebang;

        // Comments
        Rule cmt;
        cmt.pattern = QRegularExpression("#[^\n]*");
        cmt.format  = fmt(clrComment, false, true);
        m_rules << cmt;

        // Common commands
        QStringList cmds = {"echo","cd","ls","grep","find","sed","awk","cat","rm",
                            "cp","mv","mkdir","chmod","chown","sudo","apt","git",
                            "docker","curl","wget","if","then","else","fi","for",
                            "while","do","done","case","esac","function","return",
                            "export","source","alias","unset","read","exit","set"};
        Rule cmd;
        cmd.pattern = QRegularExpression("\\b(" + cmds.join("|") + ")\\b");
        cmd.format  = fmt(clrKeyword, true);
        m_rules << cmd;

    } else if (language == "SQL") {
        QStringList kws = {"SELECT","FROM","WHERE","AND","OR","NOT","INSERT","INTO",
                           "VALUES","UPDATE","SET","DELETE","CREATE","DROP","TABLE",
                           "INDEX","JOIN","LEFT","RIGHT","INNER","OUTER","FULL","ON",
                           "AS","ORDER","BY","GROUP","HAVING","LIMIT","OFFSET","NULL",
                           "IS","IN","LIKE","BETWEEN","EXISTS","UNION","ALL","DISTINCT",
                           "CASE","WHEN","THEN","ELSE","END","BEGIN","COMMIT","ROLLBACK",
                           "TRANSACTION","PRIMARY","KEY","FOREIGN","REFERENCES","UNIQUE",
                           "DEFAULT","NOT","CONSTRAINT","ALTER","ADD","COLUMN","IF",
                           "COUNT","SUM","AVG","MAX","MIN","COALESCE","NULLIF","CAST",
                           "WITH","RETURNING","OVER","PARTITION","ROW_NUMBER","RANK"};
        Rule kw;
        kw.pattern = QRegularExpression("\\b(" + kws.join("|") + ")\\b",
                                        QRegularExpression::CaseInsensitiveOption);
        kw.format  = fmt(clrKeyword, true);
        m_rules << kw;

        Rule str;
        str.pattern = QRegularExpression("'(?:[^'\\\\]|\\\\.)*'");
        str.format  = fmt(clrString);
        m_rules << str;

        Rule num;
        num.pattern = QRegularExpression("\\b\\d+\\.?\\d*\\b");
        num.format  = fmt(clrNumber);
        m_rules << num;

        Rule lc;
        lc.pattern = QRegularExpression("--[^\n]*");
        lc.format  = fmt(clrComment, false, true);
        m_rules << lc;

        m_hasBlockComment = true;
        m_mlCommentStart  = QRegularExpression("/\\*");
        m_mlCommentEnd    = QRegularExpression("\\*/");

    } else if (language == "YAML" || language == "TOML") {
        Rule key;
        key.pattern = QRegularExpression("^[\\w\\-\\.]+(?=\\s*[:=])");
        key.format  = fmt(clrBuiltin);
        m_rules << key;

        Rule str;
        str.pattern = QRegularExpression("\"(?:[^\"\\\\]|\\\\.)*\"|'[^']*'");
        str.format  = fmt(clrString);
        m_rules << str;

        Rule num;
        num.pattern = QRegularExpression("\\b(true|false|null|yes|no|on|off|\\d+\\.?\\d*)\\b",
                                         QRegularExpression::CaseInsensitiveOption);
        num.format  = fmt(clrNumber);
        m_rules << num;

        Rule cmt;
        cmt.pattern = QRegularExpression("#[^\n]*");
        cmt.format  = fmt(clrComment, false, true);
        m_rules << cmt;

    } else if (language == "JSON") {
        // Keys
        Rule key;
        key.pattern = QRegularExpression("\"(?:[^\"\\\\]|\\\\.)*\"(?=\\s*:)");
        key.format  = fmt(clrBuiltin);
        m_rules << key;

        // Values (strings)
        Rule str;
        str.pattern = QRegularExpression(":\\s*\"(?:[^\"\\\\]|\\\\.)*\"");
        str.format  = fmt(clrString);
        m_rules << str;

        Rule num;
        num.pattern = QRegularExpression(":\\s*(-?\\d+\\.?\\d*)");
        num.format  = fmt(clrNumber);
        m_rules << num;

        Rule kw;
        kw.pattern = QRegularExpression("\\b(true|false|null)\\b");
        kw.format  = fmt(clrKeyword);
        m_rules << kw;

    } else {
        // Generic: just highlight strings and comments
        Rule str;
        str.pattern = QRegularExpression("\"(?:[^\"\\\\]|\\\\.)*\"");
        str.format  = fmt(clrString);
        m_rules << str;

        Rule num;
        num.pattern = QRegularExpression("\\b\\d+\\.?\\d*\\b");
        num.format  = fmt(clrNumber);
        m_rules << num;
    }
}

void SyntaxHighlighter::highlightBlock(const QString &text) {
    // Apply single-line rules
    for (const Rule &rule : m_rules) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch m = it.next();
            setFormat(m.capturedStart(), m.capturedLength(), rule.format);
        }
    }

    // Handle block comments (state machine)
    if (!m_hasBlockComment) {
        setCurrentBlockState(0);
        return;
    }

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(m_mlCommentStart);

    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch = m_mlCommentEnd.match(text, startIndex);
        int endIndex  = endMatch.capturedStart();
        int commentLen;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLen = text.length() - startIndex;
        } else {
            commentLen = endIndex - startIndex + endMatch.capturedLength();
        }
        setFormat(startIndex, commentLen, m_mlCommentFmt);
        startIndex = text.indexOf(m_mlCommentStart, startIndex + commentLen);
    }

    if (previousBlockState() == 1 && currentBlockState() != 1)
        setCurrentBlockState(0);
}
