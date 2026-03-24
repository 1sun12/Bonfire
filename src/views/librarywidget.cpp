#include "librarywidget.h"
#include "../database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QApplication>
#include <algorithm>

// ---------- Shard row delegate ----------
class ShardDelegate : public QStyledItemDelegate {
public:
    explicit ShardDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override {
        return QSize(0, 36);
    }

    void paint(QPainter *p, const QStyleOptionViewItem &opt,
               const QModelIndex &index) const override
    {
        p->save();

        // Background
        if (opt.state & QStyle::State_Selected)
            p->fillRect(opt.rect, QColor(9, 71, 113));
        else if (opt.state & QStyle::State_MouseOver)
            p->fillRect(opt.rect, QColor(42, 42, 42));
        else
            p->fillRect(opt.rect, QColor(26, 26, 26));

        auto shard = index.data(Qt::UserRole).value<Shard>();
        QFont font = QApplication::font();
        p->setFont(font);
        QFontMetrics fm(font);

        int y      = opt.rect.top() + opt.rect.height() / 2;
        int left   = opt.rect.left() + 10;
        int right  = opt.rect.right() - 10;

        // -- Language badge (left) --
        QString lang = shard.language.isEmpty() ? "?" : shard.language;
        QColor  bgc(langColor(shard.language));
        int badgeW = fm.horizontalAdvance(lang) + 10;
        QRect badgeR(left, y - 9, badgeW, 18);
        p->setPen(Qt::NoPen);
        p->setBrush(bgc);
        p->drawRoundedRect(badgeR, 3, 3);
        p->setPen(Qt::white);
        QFont mono("Monospace");
        mono.setPointSize(font.pointSize() - 1);
        p->setFont(mono);
        p->drawText(badgeR, Qt::AlignCenter, lang);
        left += badgeW + 10;
        p->setFont(font);

        // -- Familiarity badge (right) --
        QString fam   = shard.familiarity;
        QColor  famc(famColor(fam));
        int famW = fm.horizontalAdvance(fam) + 10;
        QRect famR(right - famW, y - 9, famW, 18);
        p->setPen(Qt::NoPen);
        p->setBrush(famc);
        p->drawRoundedRect(famR, 3, 3);
        p->setPen(Qt::white);
        p->setFont(mono);
        p->drawText(famR, Qt::AlignCenter, fam);
        right -= famW + 8;
        p->setFont(font);

        // -- Category (right of title area) --
        QString cat   = shard.category;
        int catW = fm.horizontalAdvance(cat);
        p->setPen(QColor(85, 85, 85));
        p->drawText(right - catW, y + fm.ascent() / 2, cat);
        right -= catW + 12;

        // -- Review clock indicator --
        if (shard.reviewEnabled) {
            p->setPen(QColor(245, 158, 11));
            p->drawText(right - 10, y + fm.ascent() / 2, "●");
            right -= 18;
        }

        // -- Title --
        p->setPen(QColor(204, 204, 204));
        QString title = fm.elidedText(shard.title, Qt::ElideRight, right - left - 4);
        p->drawText(left, y + fm.ascent() / 2, title);

        // Bottom separator
        p->setPen(QColor(40, 40, 40));
        p->drawLine(opt.rect.bottomLeft(), opt.rect.bottomRight());

        p->restore();
    }
};

// ---------- LibraryWidget ----------

static bool fuzzyMatch(const QString &text, const QString &query) {
    if (query.trimmed().isEmpty()) return true;
    QString t = text.toLower();
    for (const QString &word : query.toLower().split(' ', Qt::SkipEmptyParts))
        if (!t.contains(word)) return false;
    return true;
}

LibraryWidget::LibraryWidget(Database *db, QWidget *parent)
    : QWidget(parent), m_db(db)
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(16, 16, 16, 8);
    root->setSpacing(8);

    // --- Search + new ---
    auto *topRow = new QHBoxLayout;
    topRow->setSpacing(8);
    m_search = new QLineEdit;
    m_search->setPlaceholderText("Search shards... (title, language, code, tags)");
    m_search->setClearButtonEnabled(true);
    topRow->addWidget(m_search, 1);
    auto *newBtn = new QPushButton("+ New");
    newBtn->setObjectName("primaryBtn");
    topRow->addWidget(newBtn);
    root->addLayout(topRow);

    // --- Filter row ---
    auto *filterRow = new QHBoxLayout;
    filterRow->setSpacing(6);

    m_filterLang = new QComboBox; m_filterLang->addItem("All languages");
    m_filterCat  = new QComboBox; m_filterCat->addItem("All categories");
    m_filterFam  = new QComboBox; m_filterFam->addItem("All familiarity");
    m_filterTag  = new QComboBox; m_filterTag->addItem("All tags");
    m_sort       = new QComboBox;
    m_sort->addItems({"Recently modified", "Alphabetical", "Familiarity (shaky first)"});

    for (const QString &cat : categories())
        m_filterCat->addItem(cat);
    for (const QString &fam : familiarities())
        m_filterFam->addItem(fam);

    filterRow->addWidget(m_filterLang, 1);
    filterRow->addWidget(m_filterCat, 1);
    filterRow->addWidget(m_filterFam, 1);
    filterRow->addWidget(m_filterTag, 1);
    filterRow->addWidget(m_sort, 1);
    root->addLayout(filterRow);

    // --- Count label ---
    m_countLbl = new QLabel("0 shards");
    m_countLbl->setObjectName("mutedText");
    root->addWidget(m_countLbl);

    // --- List ---
    m_list = new QListWidget;
    m_list->setItemDelegate(new ShardDelegate(m_list));
    m_list->setFrameShape(QFrame::NoFrame);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_list->setMouseTracking(true);
    root->addWidget(m_list, 1);

    // Connections
    connect(m_search,     &QLineEdit::textChanged, this, &LibraryWidget::applyFilter);
    connect(m_filterLang, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LibraryWidget::applyFilter);
    connect(m_filterCat,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LibraryWidget::applyFilter);
    connect(m_filterFam,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LibraryWidget::applyFilter);
    connect(m_filterTag,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LibraryWidget::applyFilter);
    connect(m_sort,       QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LibraryWidget::applyFilter);
    connect(m_list,       &QListWidget::itemDoubleClicked, this, &LibraryWidget::onItemDoubleClicked);
    connect(newBtn,       &QPushButton::clicked, this, &LibraryWidget::requestNewShard);
}

void LibraryWidget::refresh() {
    m_allShards = m_db->allShards();

    // Rebuild language and tag filter options
    QStringList langs = {"All languages"};
    QStringList tags  = {"All tags"};
    QSet<QString> seenLangs, seenTags;
    for (const Shard &s : m_allShards) {
        if (!s.language.isEmpty() && !seenLangs.contains(s.language)) {
            seenLangs.insert(s.language);
            langs << s.language;
        }
        for (const QString &t : s.tags) {
            if (!seenTags.contains(t)) {
                seenTags.insert(t);
                tags << t;
            }
        }
    }
    std::sort(langs.begin() + 1, langs.end());
    std::sort(tags.begin() + 1, tags.end());

    // Repopulate combos preserving selection
    auto resetCombo = [](QComboBox *cb, const QStringList &items) {
        QString cur = cb->currentText();
        cb->blockSignals(true);
        cb->clear();
        cb->addItems(items);
        int idx = cb->findText(cur);
        cb->setCurrentIndex(idx >= 0 ? idx : 0);
        cb->blockSignals(false);
    };
    resetCombo(m_filterLang, langs);
    resetCombo(m_filterTag,  tags);

    applyFilter();
}

QList<Shard> LibraryWidget::filteredSorted() const {
    QString q    = m_search->text();
    QString lang = (m_filterLang->currentIndex() > 0) ? m_filterLang->currentText() : "";
    QString cat  = (m_filterCat->currentIndex()  > 0) ? m_filterCat->currentText()  : "";
    QString fam  = (m_filterFam->currentIndex()  > 0) ? m_filterFam->currentText()  : "";
    QString tag  = (m_filterTag->currentIndex()  > 0) ? m_filterTag->currentText()  : "";
    int sortIdx  = m_sort->currentIndex();

    QList<Shard> res;
    for (const Shard &s : m_allShards) {
        if (!lang.isEmpty() && s.language != lang) continue;
        if (!cat.isEmpty()  && s.category != cat)  continue;
        if (!fam.isEmpty()  && s.familiarity != fam) continue;
        if (!tag.isEmpty()  && !s.tags.contains(tag)) continue;
        if (!fuzzyMatch(s.searchText(), q)) continue;
        res << s;
    }

    const QStringList famOrder = {"shaky", "fresh", "solid", "mastered"};
    if (sortIdx == 0) {
        // already sorted by modified_at desc from DB
    } else if (sortIdx == 1) {
        std::sort(res.begin(), res.end(), [](const Shard &a, const Shard &b){
            return a.title.toLower() < b.title.toLower();
        });
    } else {
        std::sort(res.begin(), res.end(), [&famOrder](const Shard &a, const Shard &b){
            return famOrder.indexOf(a.familiarity) < famOrder.indexOf(b.familiarity);
        });
    }
    return res;
}

void LibraryWidget::applyFilter() {
    QList<Shard> res = filteredSorted();
    populateList(res);
    m_countLbl->setText(QString("%1 shard%2").arg(res.size()).arg(res.size() != 1 ? "s" : ""));
}

void LibraryWidget::populateList(const QList<Shard> &shards) {
    m_list->clear();
    for (const Shard &s : shards) {
        auto *item = new QListWidgetItem(m_list);
        item->setData(Qt::UserRole, QVariant::fromValue(s));
        item->setSizeHint(QSize(0, 36));
    }
}

void LibraryWidget::onItemDoubleClicked() {
    auto *item = m_list->currentItem();
    if (!item) return;
    auto shard = item->data(Qt::UserRole).value<Shard>();
    emit openShard(shard.id);
}
