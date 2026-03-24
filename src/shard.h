#pragma once
#include <QString>
#include <QStringList>
#include <QMap>
#include <QMetaType>

struct Shard {
    QString id;
    QString title;
    QString language;
    QString code;
    QString description;
    QStringList tags;
    QString category;     // snippet, pattern, boilerplate, one-liner, troubleshoot, concept, config, cheatsheet
    QString familiarity;  // fresh, shaky, solid, mastered
    QString source;
    QStringList relatedIds;
    QString createdAt;    // ISO 8601
    QString modifiedAt;   // ISO 8601
    QString lastReviewed; // ISO 8601, may be empty
    bool reviewEnabled = false;
    int reviewInterval = 0;
    int reviewRepetitions = 0;
    double reviewEase = 2.5;
    QString reviewNext;   // YYYY-MM-DD

    bool isValid() const { return !id.isEmpty() && !title.isEmpty(); }

    QString searchText() const {
        return title + " " + language + " " + category + " " +
               tags.join(" ") + " " + description + " " + code;
    }
};

inline QString langColor(const QString &lang) {
    static const QMap<QString, QString> colors = {
        {"C",          "#555555"}, {"C++",        "#f34b7d"}, {"C#",         "#178600"},
        {"Lua",        "#000080"}, {"Python",      "#3572A5"}, {"Bash",       "#89e051"},
        {"Rust",       "#dea584"}, {"JavaScript",  "#f1e05a"}, {"TypeScript", "#3178c6"},
        {"Git",        "#F05032"}, {"Docker",      "#384d54"}, {"SQL",        "#e38c00"},
        {"Go",         "#00ADD8"}, {"Ruby",        "#701516"}, {"Java",       "#b07219"},
        {"CSS",        "#563d7c"}, {"HTML",        "#e34c26"}, {"PHP",        "#4F5D95"},
        {"Kotlin",     "#A97BFF"}, {"Swift",       "#F05138"}, {"Zig",        "#ec915c"},
        {"Haskell",    "#5e5086"}, {"Perl",        "#0298c3"}, {"R",          "#198CE7"},
        {"Scala",      "#c22d40"}, {"Shell",       "#89e051"}, {"PowerShell", "#012456"},
        {"YAML",       "#cb171e"}, {"JSON",        "#555555"}, {"TOML",       "#9c4221"},
        {"Makefile",   "#427819"}, {"Assembly",    "#6E4C13"}, {"Vim",        "#199f4b"},
        {"Nix",        "#7e7eff"}, {"KQL",         "#0078D4"}
    };
    return colors.value(lang, "#555555");
}

inline QString famColor(const QString &fam) {
    static const QMap<QString, QString> colors = {
        {"fresh",    "#f59e0b"}, {"shaky",    "#ef4444"},
        {"solid",    "#3b82f6"}, {"mastered", "#10b981"}
    };
    return colors.value(fam, "#555555");
}

inline const QStringList &defaultLanguages() {
    static const QStringList langs = {
        "Assembly", "Bash", "C", "C#", "C++", "CSS", "Docker",
        "Git", "Go", "Haskell", "HTML", "Java", "JavaScript", "JSON",
        "Kotlin", "KQL", "Lua", "Makefile", "Nix", "Perl", "PHP",
        "PowerShell", "Python", "R", "Ruby", "Rust", "Scala", "Shell",
        "SQL", "Swift", "TOML", "TypeScript", "Vim", "YAML", "Zig"
    };
    return langs;
}

inline const QStringList &categories() {
    static const QStringList cats = {
        "snippet", "pattern", "boilerplate", "one-liner",
        "troubleshoot", "concept", "config", "cheatsheet"
    };
    return cats;
}

inline const QStringList &familiarities() {
    static const QStringList fams = {"fresh", "shaky", "solid", "mastered"};
    return fams;
}

Q_DECLARE_METATYPE(Shard)
