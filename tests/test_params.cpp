#include <QString>
#include <QRegularExpression>
#include <cstdio>

static QString resolveParamsFromCode(const QString &code)
{
    QString result = code;
    QRegularExpression re("%\\s*@param:\\s*(\\w+)=(\\S+)");
    QRegularExpressionMatchIterator it = re.globalMatch(code);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString varName = match.captured(1);
        QString defaultValue = match.captured(2);
        result.replace("@@" + varName + "@@", defaultValue);
    }
    return result;
}

int main() {
    int failed = 0;

    // Test 1: Single parameter substitution
    {
        QString code =
            "% @param: xmax=7.5\n"
            "\\begin{tikzpicture}\n"
            "\\draw (0,0) -- (@@xmax@@, 0);\n"
            "\\end{tikzpicture}";
        QString result = resolveParamsFromCode(code);
        if (!result.contains("7.5")) {
            fprintf(stderr, "FAIL: Test 1a - param not substituted\n");
            failed++;
        } else if (result.contains("@@xmax@@")) {
            fprintf(stderr, "FAIL: Test 1b - @@xmax@@ still present\n");
            failed++;
        } else {
            fprintf(stderr, "PASS: Test 1 - Single parameter substitution\n");
        }
    }

    // Test 2: Multiple parameters
    {
        QString code =
            "% @param: width=5\n"
            "% @param: height=3\n"
            "\\draw (0,0) rectangle (@@width@@, @@height@@);";
        QString result = resolveParamsFromCode(code);
        if (!result.contains("5") || !result.contains("3")) {
            fprintf(stderr, "FAIL: Test 2a - multiple params not substituted\n");
            failed++;
        } else if (result.contains("@@width@@") || result.contains("@@height@@")) {
            fprintf(stderr, "FAIL: Test 2b - @@vars@@ still present\n");
            failed++;
        } else {
            fprintf(stderr, "PASS: Test 2 - Multiple parameters\n");
        }
    }

    // Test 3: Negative values
    {
        QString code =
            "% @param: xmin=-2.5\n"
            "\\draw (@@xmin@@, 0) -- (0, 0);";
        QString result = resolveParamsFromCode(code);
        if (result.contains("-2.5")) {
            fprintf(stderr, "PASS: Test 3 - Negative values\n");
        } else {
            fprintf(stderr, "FAIL: Test 3 - negative value not substituted\n");
            failed++;
        }
    }

    // Test 4: No params in code
    {
        QString code = "\\draw (0,0) -- (1,1);";
        QString result = resolveParamsFromCode(code);
        if (result == code) {
            fprintf(stderr, "PASS: Test 4 - No params, code unchanged\n");
        } else {
            fprintf(stderr, "FAIL: Test 4 - code changed unexpectedly\n");
            failed++;
        }
    }

    // Test 5: Zero value
    {
        QString code =
            "% @param: offset=0\n"
            "\\node at (@@offset@@, 0) {};";
        QString result = resolveParamsFromCode(code);
        if (result.contains("0")) {
            fprintf(stderr, "PASS: Test 5 - Zero value\n");
        } else {
            fprintf(stderr, "FAIL: Test 5 - zero value not substituted\n");
            failed++;
        }
    }

    // Test 6: Param regex parsing
    {
        QString code = "% @param: foo=bar   % @param: baz=42";
        QRegularExpression re("%\\s*@param:\\s*(\\w+)=(\\S+)");
        QRegularExpressionMatchIterator it = re.globalMatch(code);

        int count = 0;
        QStringList names, values;
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            names.append(match.captured(1));
            values.append(match.captured(2));
            count++;
        }

        if (count == 2 && names[0] == "foo" && values[0] == "bar"
            && names[1] == "baz" && values[1] == "42") {
            fprintf(stderr, "PASS: Test 6 - Param regex parsing\n");
        } else {
            fprintf(stderr, "FAIL: Test 6 - regex parsing mismatch count=%d\n", count);
            failed++;
        }
    }

    if (failed > 0) {
        fprintf(stderr, "\n%d test(s) failed!\n", failed);
        return 1;
    }

    fprintf(stderr, "\nAll parameter substitution tests passed!\n");
    return 0;
}
