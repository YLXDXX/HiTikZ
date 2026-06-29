#include <QCoreApplication>
#include <QRegularExpression>
#include <QDebug>
#include <cassert>
#include <cstdio>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    int failed = 0;

    // Test 1: $...$ inline math regex
    {
        QRegularExpression re(QStringLiteral("\\$[^\\$]*\\$"));
        assert(re.match(QStringLiteral("$x^2+y^2$")).hasMatch());
        assert(re.match(QStringLiteral("text $\\alpha$ more")).hasMatch());
        assert(!re.match(QStringLiteral("unmatched $dollar")).hasMatch());
        fprintf(stderr, "PASS: Test 1 - $...$ inline math regex\n");
    }

    // Test 2: \\(...\\) inline math regex
    {
        QRegularExpression re(QStringLiteral("\\\\\\(.*?\\\\\\)"));
        auto m = re.match(QStringLiteral("\\(x^2+y^2\\)"));
        assert(m.hasMatch());
        assert(m.captured() == QStringLiteral("\\(x^2+y^2\\)"));

        auto m2 = re.match(QStringLiteral("text \\(\\alpha\\) more"));
        assert(m2.hasMatch());

        auto m3 = re.match(QStringLiteral("no math (plain parens)"));
        assert(!m3.hasMatch());

        fprintf(stderr, "PASS: Test 2 - \\(...\\) inline math regex\n");
    }

    // Test 3: \\[...\\] display math regex
    {
        QRegularExpression re(QString::fromUtf8(R"(\\\[.*?\\\])"));
        auto m = re.match(QStringLiteral("\\[E=mc^2\\]"));
        assert(m.hasMatch());
        assert(m.captured() == QStringLiteral("\\[E=mc^2\\]"));

        auto m2 = re.match(QStringLiteral("text \\[\\sum_{i=1}^n\\] more"));
        assert(m2.hasMatch());

        auto m3 = re.match(QStringLiteral("not math [plain brackets]"));
        assert(!m3.hasMatch());

        fprintf(stderr, "PASS: Test 3 - \\[...\\] display math regex\n");
    }

    if (failed > 0) {
        fprintf(stderr, "\n%d test(s) failed!\n", failed);
        return 1;
    }
    fprintf(stderr, "\nAll highlighter regex tests passed!\n");
    return 0;
}
