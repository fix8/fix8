#ifndef EDIT_HIGHLIGHTER_H
#define EDIT_HIGHLIGHTER_H

#include <QSyntaxHighlighter>

#include <QHash>
#include <QTextCharFormat>
QT_BEGIN_NAMESPACE


class QTextDocument;

QT_END_NAMESPACE
class ModelSchema;

//! [0]
class EditHighLighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    EditHighLighter(QTextDocument *parent = 0);
    void setColumHeaders(QStringList &ch);
protected:
    void highlightBlock(const QString &text);

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegExp commentStartExpression;
    QRegExp commentEndExpression;
    QRegExp headerFormat;
    QTextCharFormat keywordFormat;
    QTextCharFormat variableFormat;
    QTextCharFormat badVariableFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
    QStringList columnHeaders;
};
//! [0]

#endif
