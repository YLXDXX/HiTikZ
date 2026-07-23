#include "snippet_manager.h"
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QUuid>
#include <QJsonDocument>
#include <QSet>
#include <QProcess>
#include <QTemporaryDir>
#include <QDebug>
#include <QSaveFile>
#include <algorithm>

int SnippetManager::fuzzyMatchScore(const QString &query, const QString &target)
{
    if (query.isEmpty()) return 100;

    QString normQuery = query.normalized(QString::NormalizationForm_C).toCaseFolded();
    QString normTarget = target.normalized(QString::NormalizationForm_C).toCaseFolded();

    int score = 0;
    int qi = 0;
    int consecutive = 0;

    for (int ti = 0; ti < normTarget.length() && qi < normQuery.length(); ++ti) {
        if (normQuery[qi] == normTarget[ti]) {
            score += 10 + consecutive * 5;
            consecutive++;
            qi++;
        } else {
            consecutive = 0;
        }
    }

    return (qi == normQuery.length()) ? score : 0;
}

void SnippetManager::ensureSearchIndexBuilt() const
{
    if (m_searchIndex.built) return;

    m_searchIndex.bigramIndex.clear();
    m_searchIndex.allTexts.clear();
    m_searchIndex.allIds.clear();

    QList<Snippet> all = getAllSnippets();
    QList<Snippet> presets = getAllPresets();
    all.append(presets);

    for (int idx = 0; idx < all.size(); ++idx) {
        const Snippet &s = all.at(idx);
        m_searchIndex.allIds.append(s.id);

        QString text = (s.name + " " + s.description)
            .normalized(QString::NormalizationForm_C)
            .toCaseFolded();
        m_searchIndex.allTexts.append(text);

        for (int i = 0; i < text.length() - 1; ++i) {
            QString bigram = text.mid(i, 2);
            m_searchIndex.bigramIndex[bigram].insert(idx);
        }
    }

    m_searchIndex.built = true;
}

void SnippetManager::addSnippetToSearchIndex(const Snippet &s) const
{
    if (!m_searchIndex.built) return;

    QString text = (s.name + " " + s.description)
        .normalized(QString::NormalizationForm_C)
        .toCaseFolded();

    int idx = m_searchIndex.allTexts.size();
    m_searchIndex.allIds.append(s.id);
    m_searchIndex.allTexts.append(text);

    for (int i = 0; i < text.length() - 1; ++i) {
        QString bigram = text.mid(i, 2);
        m_searchIndex.bigramIndex[bigram].insert(idx);
    }
}

void SnippetManager::removeSnippetFromSearchIndex(const QString &id) const
{
    if (!m_searchIndex.built) return;

    int targetIdx = -1;
    for (int i = 0; i < m_searchIndex.allIds.size(); ++i) {
        if (m_searchIndex.allIds[i] == id) {
            targetIdx = i;
            break;
        }
    }
    if (targetIdx < 0) return;

    const QString &text = m_searchIndex.allTexts[targetIdx];
    for (int i = 0; i < text.length() - 1; ++i) {
        QString bigram = text.mid(i, 2);
        m_searchIndex.bigramIndex[bigram].remove(targetIdx);
    }

    int lastIdx = m_searchIndex.allTexts.size() - 1;
    if (targetIdx != lastIdx) {
        const QString &lastText = m_searchIndex.allTexts[lastIdx];
        const QString &lastId = m_searchIndex.allIds[lastIdx];
        for (int i = 0; i < lastText.length() - 1; ++i) {
            QString bigram = lastText.mid(i, 2);
            m_searchIndex.bigramIndex[bigram].remove(lastIdx);
            m_searchIndex.bigramIndex[bigram].insert(targetIdx);
        }
        m_searchIndex.allTexts[targetIdx] = lastText;
        m_searchIndex.allIds[targetIdx] = lastId;
    }
    m_searchIndex.allTexts.removeLast();
    m_searchIndex.allIds.removeLast();
}

QList<SearchResult> SnippetManager::searchSnippets(const QString &query, bool includePresets) const
{
    QList<SearchResult> results;

    if (query.isEmpty() || query.length() < 2) {
        QList<Snippet> all = getAllSnippets();
        if (includePresets) {
            QList<Snippet> presets = getAllPresets();
            all.append(presets);
        }
        for (const Snippet &s : all) {
            if (query.isEmpty()) {
                SearchResult r;
                r.snippet = s;
                r.score = 0;
                results.append(r);
            } else {
                int nameScore = fuzzyMatchScore(query, s.name);
                int descScore = fuzzyMatchScore(query, s.description) / 3;
                int totalScore = nameScore * 2 + descScore;
                if (totalScore > 0) {
                    SearchResult r;
                    r.snippet = s;
                    r.score = totalScore;
                    results.append(r);
                }
            }
        }
        std::sort(results.begin(), results.end(),
            [](const SearchResult &a, const SearchResult &b) {
                return a.score > b.score;
            });
        return results;
    }

    ensureSearchIndexBuilt();

    QString normQuery = query.normalized(QString::NormalizationForm_C).toCaseFolded();
    QSet<int> candidates;

    bool firstBigram = true;
    for (int i = 0; i < normQuery.length() - 1; ++i) {
        QString bigram = normQuery.mid(i, 2);
        if (m_searchIndex.bigramIndex.contains(bigram)) {
            if (firstBigram) {
                candidates = m_searchIndex.bigramIndex[bigram];
                firstBigram = false;
            } else {
                candidates.intersect(m_searchIndex.bigramIndex[bigram]);
            }
        }
    }

    if (candidates.isEmpty()) return results;

    QList<Snippet> all = getAllSnippets();
    if (includePresets) {
        QList<Snippet> presets = getAllPresets();
        all.append(presets);
    }

    QHash<QString, Snippet> idToSnippet;
    for (const Snippet &s : all)
        idToSnippet[s.id] = s;

    for (int idx : candidates) {
        if (idx >= m_searchIndex.allIds.size()) continue;
        QString sid = m_searchIndex.allIds[idx];
        if (!idToSnippet.contains(sid)) continue;
        Snippet s = idToSnippet[sid];

        int nameScore = fuzzyMatchScore(query, s.name);
        int descScore = fuzzyMatchScore(query, s.description) / 3;
        int totalScore = nameScore * 2 + descScore;

        if (totalScore > 0) {
            SearchResult r;
            r.snippet = s;
            r.score = totalScore;
            results.append(r);
        }
    }

    std::sort(results.begin(), results.end(),
        [](const SearchResult &a, const SearchResult &b) {
            return a.score > b.score;
        });

    return results;
}

QStringList SnippetManager::getAllCategories(bool includePresets) const
{
    QSet<QString> catSet;

    QStringList persisted = loadAllPersistedCategories();
    for (const QString &cat : persisted)
        catSet.insert(cat);

    QStringList raw;
    if (!includePresets) {
        QList<Snippet> all = getAllSnippets();
        for (const Snippet &s : all) {
            if (!s.category.isEmpty()) catSet.insert(s.category);
        }
    } else {
        ensureCountsCached();
        for (auto it = m_cachedCategoryCounts.constBegin(); it != m_cachedCategoryCounts.constEnd(); ++it)
            catSet.insert(it.key());
    }

    for (const QString &cat : catSet) {
        QString path = cat;
        int slash = path.lastIndexOf('/');
        while (slash > 0) {
            catSet.insert(path.left(slash));
            slash = path.lastIndexOf('/', slash - 1);
        }
    }

    raw = catSet.values();
    QStringList savedOrder = loadCategoryOrder();
    QStringList sorted;
    for (const QString &cat : savedOrder) {
        if (raw.contains(cat)) {
            sorted.append(cat);
            raw.removeAll(cat);
        }
    }
    raw.sort();
    sorted.append(raw);
    return sorted;
}

QMap<QString, int> SnippetManager::getCategoryCounts(bool includePresets) const
{
    if (!includePresets) {
        QMap<QString, int> counts;
        QList<Snippet> all = getAllSnippets();
        for (const Snippet &s : all) {
            if (!s.category.isEmpty()) counts[s.category]++;
        }
        return counts;
    }

    ensureCountsCached();
    return m_cachedCategoryCounts;
}

int SnippetManager::getUncategorizedCount(bool includePresets) const
{
    int count = 0;
    QList<Snippet> all = getAllSnippets();
    for (const Snippet &s : all) {
        if (s.category.isEmpty()) count++;
    }
    if (includePresets) {
        QList<Snippet> presets = getAllPresets();
        for (const Snippet &s : presets) {
            if (s.category.isEmpty()) count++;
        }
    }
    return count;
}

void SnippetManager::ensureCountsCached() const
{
    if (m_countsCached) return;

    m_cachedCategoryCounts.clear();
    QList<Snippet> all = getAllSnippets();
    for (const Snippet &s : all) {
        if (!s.category.isEmpty())
            m_cachedCategoryCounts[s.category]++;
    }
    QList<Snippet> presets = getAllPresets();
    for (const Snippet &s : presets) {
        if (!s.category.isEmpty())
            m_cachedCategoryCounts[s.category]++;
    }
    m_countsCached = true;
}
