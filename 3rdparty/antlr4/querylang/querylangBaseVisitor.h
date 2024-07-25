
// Generated from querylang.g4 by ANTLR 4.7.2

#pragma once


#include "antlr4-runtime.h"
#include "querylangVisitor.h"


/**
 * This class provides an empty implementation of querylangVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  querylangBaseVisitor : public querylangVisitor {
public:

  virtual antlrcpp::Any visitQuery(querylangParser::QueryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInvalideQuery(querylangParser::InvalideQueryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPrimary(querylangParser::PrimaryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBinaryExpression(querylangParser::BinaryExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSearchCondition(querylangParser::SearchConditionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDateSearch(querylangParser::DateSearchContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBinaryDateSearch(querylangParser::BinaryDateSearchContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDateSearchinfo(querylangParser::DateSearchinfoContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAbsolutedate(querylangParser::AbsolutedateContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitRelativelydate(querylangParser::RelativelydateContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPathSearch(querylangParser::PathSearchContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNameSearch(querylangParser::NameSearchContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSizeSearch(querylangParser::SizeSearchContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTypeSearch(querylangParser::TypeSearchContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDurationSearch(querylangParser::DurationSearchContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMetaSearch(querylangParser::MetaSearchContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitQuantityCondition(querylangParser::QuantityConditionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitContentSearch(querylangParser::ContentSearchContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitComparison_type(querylangParser::Comparison_typeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFilename(querylangParser::FilenameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitString(querylangParser::StringContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIs_or_not(querylangParser::Is_or_notContext *ctx) override {
    return visitChildren(ctx);
  }


};

