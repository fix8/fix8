
#include <QtGui>
#include "editHighLighter.h"

EditHighLighter::EditHighLighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{


    HighlightingRule rule;

    variableFormat.setFontWeight(QFont::Bold);
    variableFormat.setForeground(Qt::green);
    rule.pattern = QRegExp("\\$[0-9]+\\b");
    rule.format = variableFormat;
    highlightingRules.append(rule);
    badVariableFormat.setFontWeight(QFont::Bold);
    badVariableFormat.setForeground(Qt::red);
    rule.pattern = QRegExp("\\$[A-Za-z]+\\b");
    rule.format = badVariableFormat;
    highlightingRules.append(rule);


    singleLineCommentFormat.setForeground(Qt::red);
    rule.pattern = QRegExp("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(Qt::red);
    quotationFormat.setForeground(QColor("darkblue"));
    rule.pattern = QRegExp("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);
    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);
    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");
}
void EditHighLighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    setCurrentBlockState(0);
    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = commentStartExpression.indexIn(text);
    while (startIndex >= 0) {
        int endIndex = commentEndExpression.indexIn(text, startIndex);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                    + commentEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
    }
}

void EditHighLighter::setColumHeaders(QStringList &ch)
{
    HighlightingRule rule;
    highlightingRules.clear();
    columnHeaders.clear();
    columnHeaders.clear();
    columnHeaders = ch;
    keywordFormat.setForeground(QColor("darkgreen"));
    keywordFormat.setFontWeight(QFont::Bold);
    foreach (const QString &column, columnHeaders) {
        rule.pattern = QRegExp(column);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    singleLineCommentFormat.setForeground(Qt::red);
    rule.pattern = QRegExp("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(Qt::red);
    quotationFormat.setForeground(Qt::green);
    rule.pattern = QRegExp("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);
    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);
}
