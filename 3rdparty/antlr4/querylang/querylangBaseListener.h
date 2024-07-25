
// Generated from querylang.g4 by ANTLR 4.7.2

#pragma once


#include "antlr4-runtime.h"
#include "querylangListener.h"


/**
 * This class provides an empty implementation of querylangListener,
 * which can be extended to create a listener which only needs to handle a subset
 * of the available methods.
 */
class  querylangBaseListener : public querylangListener {
public:

  virtual void enterQuery(querylangParser::QueryContext * /*ctx*/) override { }
  virtual void exitQuery(querylangParser::QueryContext * /*ctx*/) override { }

  virtual void enterInvalideQuery(querylangParser::InvalideQueryContext * /*ctx*/) override { }
  virtual void exitInvalideQuery(querylangParser::InvalideQueryContext * /*ctx*/) override { }

  virtual void enterPrimary(querylangParser::PrimaryContext * /*ctx*/) override { }
  virtual void exitPrimary(querylangParser::PrimaryContext * /*ctx*/) override { }

  virtual void enterBinaryExpression(querylangParser::BinaryExpressionContext * /*ctx*/) override { }
  virtual void exitBinaryExpression(querylangParser::BinaryExpressionContext * /*ctx*/) override { }

  virtual void enterSearchCondition(querylangParser::SearchConditionContext * /*ctx*/) override { }
  virtual void exitSearchCondition(querylangParser::SearchConditionContext * /*ctx*/) override { }

  virtual void enterDateSearch(querylangParser::DateSearchContext * /*ctx*/) override { }
  virtual void exitDateSearch(querylangParser::DateSearchContext * /*ctx*/) override { }

  virtual void enterBinaryDateSearch(querylangParser::BinaryDateSearchContext * /*ctx*/) override { }
  virtual void exitBinaryDateSearch(querylangParser::BinaryDateSearchContext * /*ctx*/) override { }

  virtual void enterDateSearchinfo(querylangParser::DateSearchinfoContext * /*ctx*/) override { }
  virtual void exitDateSearchinfo(querylangParser::DateSearchinfoContext * /*ctx*/) override { }

  virtual void enterAbsolutedate(querylangParser::AbsolutedateContext * /*ctx*/) override { }
  virtual void exitAbsolutedate(querylangParser::AbsolutedateContext * /*ctx*/) override { }

  virtual void enterRelativelydate(querylangParser::RelativelydateContext * /*ctx*/) override { }
  virtual void exitRelativelydate(querylangParser::RelativelydateContext * /*ctx*/) override { }

  virtual void enterPathSearch(querylangParser::PathSearchContext * /*ctx*/) override { }
  virtual void exitPathSearch(querylangParser::PathSearchContext * /*ctx*/) override { }

  virtual void enterNameSearch(querylangParser::NameSearchContext * /*ctx*/) override { }
  virtual void exitNameSearch(querylangParser::NameSearchContext * /*ctx*/) override { }

  virtual void enterSizeSearch(querylangParser::SizeSearchContext * /*ctx*/) override { }
  virtual void exitSizeSearch(querylangParser::SizeSearchContext * /*ctx*/) override { }

  virtual void enterTypeSearch(querylangParser::TypeSearchContext * /*ctx*/) override { }
  virtual void exitTypeSearch(querylangParser::TypeSearchContext * /*ctx*/) override { }

  virtual void enterDurationSearch(querylangParser::DurationSearchContext * /*ctx*/) override { }
  virtual void exitDurationSearch(querylangParser::DurationSearchContext * /*ctx*/) override { }

  virtual void enterMetaSearch(querylangParser::MetaSearchContext * /*ctx*/) override { }
  virtual void exitMetaSearch(querylangParser::MetaSearchContext * /*ctx*/) override { }

  virtual void enterQuantityCondition(querylangParser::QuantityConditionContext * /*ctx*/) override { }
  virtual void exitQuantityCondition(querylangParser::QuantityConditionContext * /*ctx*/) override { }

  virtual void enterContentSearch(querylangParser::ContentSearchContext * /*ctx*/) override { }
  virtual void exitContentSearch(querylangParser::ContentSearchContext * /*ctx*/) override { }

  virtual void enterComparison_type(querylangParser::Comparison_typeContext * /*ctx*/) override { }
  virtual void exitComparison_type(querylangParser::Comparison_typeContext * /*ctx*/) override { }

  virtual void enterFilename(querylangParser::FilenameContext * /*ctx*/) override { }
  virtual void exitFilename(querylangParser::FilenameContext * /*ctx*/) override { }

  virtual void enterString(querylangParser::StringContext * /*ctx*/) override { }
  virtual void exitString(querylangParser::StringContext * /*ctx*/) override { }

  virtual void enterIs_or_not(querylangParser::Is_or_notContext * /*ctx*/) override { }
  virtual void exitIs_or_not(querylangParser::Is_or_notContext * /*ctx*/) override { }


  virtual void enterEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void exitEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void visitTerminal(antlr4::tree::TerminalNode * /*node*/) override { }
  virtual void visitErrorNode(antlr4::tree::ErrorNode * /*node*/) override { }

};

