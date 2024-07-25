
// Generated from querylang.g4 by ANTLR 4.7.2

#pragma once


#include "antlr4-runtime.h"
#include "querylangParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by querylangParser.
 */
class  querylangVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by querylangParser.
   */
    virtual antlrcpp::Any visitQuery(querylangParser::QueryContext *context) = 0;

    virtual antlrcpp::Any visitInvalideQuery(querylangParser::InvalideQueryContext *context) = 0;

    virtual antlrcpp::Any visitPrimary(querylangParser::PrimaryContext *context) = 0;

    virtual antlrcpp::Any visitBinaryExpression(querylangParser::BinaryExpressionContext *context) = 0;

    virtual antlrcpp::Any visitSearchCondition(querylangParser::SearchConditionContext *context) = 0;

    virtual antlrcpp::Any visitDateSearch(querylangParser::DateSearchContext *context) = 0;

    virtual antlrcpp::Any visitBinaryDateSearch(querylangParser::BinaryDateSearchContext *context) = 0;

    virtual antlrcpp::Any visitDateSearchinfo(querylangParser::DateSearchinfoContext *context) = 0;

    virtual antlrcpp::Any visitAbsolutedate(querylangParser::AbsolutedateContext *context) = 0;

    virtual antlrcpp::Any visitRelativelydate(querylangParser::RelativelydateContext *context) = 0;

    virtual antlrcpp::Any visitPathSearch(querylangParser::PathSearchContext *context) = 0;

    virtual antlrcpp::Any visitNameSearch(querylangParser::NameSearchContext *context) = 0;

    virtual antlrcpp::Any visitSizeSearch(querylangParser::SizeSearchContext *context) = 0;

    virtual antlrcpp::Any visitTypeSearch(querylangParser::TypeSearchContext *context) = 0;

    virtual antlrcpp::Any visitDurationSearch(querylangParser::DurationSearchContext *context) = 0;

    virtual antlrcpp::Any visitMetaSearch(querylangParser::MetaSearchContext *context) = 0;

    virtual antlrcpp::Any visitQuantityCondition(querylangParser::QuantityConditionContext *context) = 0;

    virtual antlrcpp::Any visitContentSearch(querylangParser::ContentSearchContext *context) = 0;

    virtual antlrcpp::Any visitComparison_type(querylangParser::Comparison_typeContext *context) = 0;

    virtual antlrcpp::Any visitFilename(querylangParser::FilenameContext *context) = 0;

    virtual antlrcpp::Any visitString(querylangParser::StringContext *context) = 0;

    virtual antlrcpp::Any visitIs_or_not(querylangParser::Is_or_notContext *context) = 0;


};

