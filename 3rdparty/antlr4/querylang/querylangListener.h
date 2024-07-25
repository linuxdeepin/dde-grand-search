
// Generated from querylang.g4 by ANTLR 4.7.2

#pragma once


#include "antlr4-runtime.h"
#include "querylangParser.h"


/**
 * This interface defines an abstract listener for a parse tree produced by querylangParser.
 */
class  querylangListener : public antlr4::tree::ParseTreeListener {
public:

  virtual void enterQuery(querylangParser::QueryContext *ctx) = 0;
  virtual void exitQuery(querylangParser::QueryContext *ctx) = 0;

  virtual void enterInvalideQuery(querylangParser::InvalideQueryContext *ctx) = 0;
  virtual void exitInvalideQuery(querylangParser::InvalideQueryContext *ctx) = 0;

  virtual void enterPrimary(querylangParser::PrimaryContext *ctx) = 0;
  virtual void exitPrimary(querylangParser::PrimaryContext *ctx) = 0;

  virtual void enterBinaryExpression(querylangParser::BinaryExpressionContext *ctx) = 0;
  virtual void exitBinaryExpression(querylangParser::BinaryExpressionContext *ctx) = 0;

  virtual void enterSearchCondition(querylangParser::SearchConditionContext *ctx) = 0;
  virtual void exitSearchCondition(querylangParser::SearchConditionContext *ctx) = 0;

  virtual void enterDateSearch(querylangParser::DateSearchContext *ctx) = 0;
  virtual void exitDateSearch(querylangParser::DateSearchContext *ctx) = 0;

  virtual void enterBinaryDateSearch(querylangParser::BinaryDateSearchContext *ctx) = 0;
  virtual void exitBinaryDateSearch(querylangParser::BinaryDateSearchContext *ctx) = 0;

  virtual void enterDateSearchinfo(querylangParser::DateSearchinfoContext *ctx) = 0;
  virtual void exitDateSearchinfo(querylangParser::DateSearchinfoContext *ctx) = 0;

  virtual void enterAbsolutedate(querylangParser::AbsolutedateContext *ctx) = 0;
  virtual void exitAbsolutedate(querylangParser::AbsolutedateContext *ctx) = 0;

  virtual void enterRelativelydate(querylangParser::RelativelydateContext *ctx) = 0;
  virtual void exitRelativelydate(querylangParser::RelativelydateContext *ctx) = 0;

  virtual void enterPathSearch(querylangParser::PathSearchContext *ctx) = 0;
  virtual void exitPathSearch(querylangParser::PathSearchContext *ctx) = 0;

  virtual void enterNameSearch(querylangParser::NameSearchContext *ctx) = 0;
  virtual void exitNameSearch(querylangParser::NameSearchContext *ctx) = 0;

  virtual void enterSizeSearch(querylangParser::SizeSearchContext *ctx) = 0;
  virtual void exitSizeSearch(querylangParser::SizeSearchContext *ctx) = 0;

  virtual void enterTypeSearch(querylangParser::TypeSearchContext *ctx) = 0;
  virtual void exitTypeSearch(querylangParser::TypeSearchContext *ctx) = 0;

  virtual void enterDurationSearch(querylangParser::DurationSearchContext *ctx) = 0;
  virtual void exitDurationSearch(querylangParser::DurationSearchContext *ctx) = 0;

  virtual void enterMetaSearch(querylangParser::MetaSearchContext *ctx) = 0;
  virtual void exitMetaSearch(querylangParser::MetaSearchContext *ctx) = 0;

  virtual void enterQuantityCondition(querylangParser::QuantityConditionContext *ctx) = 0;
  virtual void exitQuantityCondition(querylangParser::QuantityConditionContext *ctx) = 0;

  virtual void enterContentSearch(querylangParser::ContentSearchContext *ctx) = 0;
  virtual void exitContentSearch(querylangParser::ContentSearchContext *ctx) = 0;

  virtual void enterComparison_type(querylangParser::Comparison_typeContext *ctx) = 0;
  virtual void exitComparison_type(querylangParser::Comparison_typeContext *ctx) = 0;

  virtual void enterFilename(querylangParser::FilenameContext *ctx) = 0;
  virtual void exitFilename(querylangParser::FilenameContext *ctx) = 0;

  virtual void enterString(querylangParser::StringContext *ctx) = 0;
  virtual void exitString(querylangParser::StringContext *ctx) = 0;

  virtual void enterIs_or_not(querylangParser::Is_or_notContext *ctx) = 0;
  virtual void exitIs_or_not(querylangParser::Is_or_notContext *ctx) = 0;


};

