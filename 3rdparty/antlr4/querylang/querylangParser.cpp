
// Generated from querylang.g4 by ANTLR 4.7.2


#include "querylangListener.h"
#include "querylangVisitor.h"

#include "querylangParser.h"


using namespace antlrcpp;
using namespace antlr4;

querylangParser::querylangParser(TokenStream *input) : Parser(input) {
  _interpreter = new atn::ParserATNSimulator(this, _atn, _decisionToDFA, _sharedContextCache);
}

querylangParser::~querylangParser() {
  delete _interpreter;
}

std::string querylangParser::getGrammarFileName() const {
  return "querylang.g4";
}

const std::vector<std::string>& querylangParser::getRuleNames() const {
  return _ruleNames;
}

dfa::Vocabulary& querylangParser::getVocabulary() const {
  return _vocabulary;
}


//----------------- QueryContext ------------------------------------------------------------------

querylangParser::QueryContext::QueryContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

querylangParser::PrimaryContext* querylangParser::QueryContext::primary() {
  return getRuleContext<querylangParser::PrimaryContext>(0);
}

querylangParser::InvalideQueryContext* querylangParser::QueryContext::invalideQuery() {
  return getRuleContext<querylangParser::InvalideQueryContext>(0);
}


size_t querylangParser::QueryContext::getRuleIndex() const {
  return querylangParser::RuleQuery;
}

void querylangParser::QueryContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterQuery(this);
}

void querylangParser::QueryContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitQuery(this);
}


antlrcpp::Any querylangParser::QueryContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<querylangVisitor*>(visitor))
    return parserVisitor->visitQuery(this);
  else
    return visitor->visitChildren(this);
}

querylangParser::QueryContext* querylangParser::query() {
  QueryContext *_localctx = _tracker.createInstance<QueryContext>(_ctx, getState());
  enterRule(_localctx, 0, querylangParser::RuleQuery);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(46);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case querylangParser::T__3:
      case querylangParser::T__5:
      case querylangParser::T__8:
      case querylangParser::T__9:
      case querylangParser::T__10:
      case querylangParser::T__12:
      case querylangParser::T__13:
      case querylangParser::T__14:
      case querylangParser::T__17:
      case querylangParser::T__19:
      case querylangParser::T__20: {
        enterOuterAlt(_localctx, 1);
        setState(44);
        primary();
        break;
      }

      case querylangParser::T__0: {
        enterOuterAlt(_localctx, 2);
        setState(45);
        invalideQuery();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- InvalideQueryContext ------------------------------------------------------------------

querylangParser::InvalideQueryContext::InvalideQueryContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

querylangParser::StringContext* querylangParser::InvalideQueryContext::string() {
  return getRuleContext<querylangParser::StringContext>(0);
}


size_t querylangParser::InvalideQueryContext::getRuleIndex() const {
  return querylangParser::RuleInvalideQuery;
}

void querylangParser::InvalideQueryContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterInvalideQuery(this);
}

void querylangParser::InvalideQueryContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitInvalideQuery(this);
}


antlrcpp::Any querylangParser::InvalideQueryContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<querylangVisitor*>(visitor))
    return parserVisitor->visitInvalideQuery(this);
  else
    return visitor->visitChildren(this);
}

querylangParser::InvalideQueryContext* querylangParser::invalideQuery() {
  InvalideQueryContext *_localctx = _tracker.createInstance<InvalideQueryContext>(_ctx, getState());
  enterRule(_localctx, 2, querylangParser::RuleInvalideQuery);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(48);
    match(querylangParser::T__0);
    setState(49);
    string();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PrimaryContext ------------------------------------------------------------------

querylangParser::PrimaryContext::PrimaryContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<querylangParser::BinaryExpressionContext *> querylangParser::PrimaryContext::binaryExpression() {
  return getRuleContexts<querylangParser::BinaryExpressionContext>();
}

querylangParser::BinaryExpressionContext* querylangParser::PrimaryContext::binaryExpression(size_t i) {
  return getRuleContext<querylangParser::BinaryExpressionContext>(i);
}


size_t querylangParser::PrimaryContext::getRuleIndex() const {
  return querylangParser::RulePrimary;
}

void querylangParser::PrimaryContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPrimary(this);
}

void querylangParser::PrimaryContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPrimary(this);
}


antlrcpp::Any querylangParser::PrimaryContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<querylangVisitor*>(visitor))
    return parserVisitor->visitPrimary(this);
  else
    return visitor->visitChildren(this);
}

querylangParser::PrimaryContext* querylangParser::primary() {
  PrimaryContext *_localctx = _tracker.createInstance<PrimaryContext>(_ctx, getState());
  enterRule(_localctx, 4, querylangParser::RulePrimary);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(51);
    binaryExpression();
    setState(58);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == querylangParser::T__1

    || _la == querylangParser::T__2) {
      setState(56);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case querylangParser::T__1: {
          setState(52);
          match(querylangParser::T__1);
          setState(53);
          binaryExpression();
          break;
        }

        case querylangParser::T__2: {
          setState(54);
          match(querylangParser::T__2);
          setState(55);
          binaryExpression();
          break;
        }

      default:
        throw NoViableAltException(this);
      }
      setState(60);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BinaryExpressionContext ------------------------------------------------------------------

querylangParser::BinaryExpressionContext::BinaryExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

querylangParser::PrimaryContext* querylangParser::BinaryExpressionContext::primary() {
  return getRuleContext<querylangParser::PrimaryContext>(0);
}

querylangParser::SearchConditionContext* querylangParser::BinaryExpressionContext::searchCondition() {
  return getRuleContext<querylangParser::SearchConditionContext>(0);
}


size_t querylangParser::BinaryExpressionContext::getRuleIndex() const {
  return querylangParser::RuleBinaryExpression;
}

void querylangParser::BinaryExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBinaryExpression(this);
}

void querylangParser::BinaryExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBinaryExpression(this);
}


antlrcpp::Any querylangParser::BinaryExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<querylangVisitor*>(visitor))
    return parserVisitor->visitBinaryExpression(this);
  else
    return visitor->visitChildren(this);
}

querylangParser::BinaryExpressionContext* querylangParser::binaryExpression() {
  BinaryExpressionContext *_localctx = _tracker.createInstance<BinaryExpressionContext>(_ctx, getState());
  enterRule(_localctx, 6, querylangParser::RuleBinaryExpression);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(66);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 3, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(61);
      match(querylangParser::T__3);
      setState(62);
      primary();
      setState(63);
      match(querylangParser::T__4);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(65);
      searchCondition();
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SearchConditionContext ------------------------------------------------------------------

querylangParser::SearchConditionContext::SearchConditionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

querylangParser::DateSearchContext* querylangParser::SearchConditionContext::dateSearch() {
  return getRuleContext<querylangParser::DateSearchContext>(0);
}

querylangParser::PathSearchContext* querylangParser::SearchConditionContext::pathSearch() {
  return getRuleContext<querylangParser::PathSearchContext>(0);
}

querylangParser::NameSearchContext* querylangParser::SearchConditionContext::nameSearch() {
  return getRuleContext<querylangParser::NameSearchContext>(0);
}

querylangParser::SizeSearchContext* querylangParser::SearchConditionContext::sizeSearch() {
  return getRuleContext<querylangParser::SizeSearchContext>(0);
}

querylangParser::TypeSearchContext* querylangParser::SearchConditionContext::typeSearch() {
  return getRuleContext<querylangParser::TypeSearchContext>(0);
}

querylangParser::DurationSearchContext* querylangParser::SearchConditionContext::durationSearch() {
  return getRuleContext<querylangParser::DurationSearchContext>(0);
}

querylangParser::MetaSearchContext* querylangParser::SearchConditionContext::metaSearch() {
  return getRuleContext<querylangParser::MetaSearchContext>(0);
}

querylangParser::QuantityConditionContext* querylangParser::SearchConditionContext::quantityCondition() {
  return getRuleContext<querylangParser::QuantityConditionContext>(0);
}

querylangParser::ContentSearchContext* querylangParser::SearchConditionContext::contentSearch() {
  return getRuleContext<querylangParser::ContentSearchContext>(0);
}


size_t querylangParser::SearchConditionContext::getRuleIndex() const {
  return querylangParser::RuleSearchCondition;
}

void querylangParser::SearchConditionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSearchCondition(this);
}

void querylangParser::SearchConditionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSearchCondition(this);
}


antlrcpp::Any querylangParser::SearchConditionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<querylangVisitor*>(visitor))
    return parserVisitor->visitSearchCondition(this);
  else
    return visitor->visitChildren(this);
}

querylangParser::SearchConditionContext* querylangParser::searchCondition() {
  SearchConditionContext *_localctx = _tracker.createInstance<SearchConditionContext>(_ctx, getState());
  enterRule(_localctx, 8, querylangParser::RuleSearchCondition);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(77);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case querylangParser::T__3:
      case querylangParser::T__5: {
        enterOuterAlt(_localctx, 1);
        setState(68);
        dateSearch();
        break;
      }

      case querylangParser::T__8: {
        enterOuterAlt(_localctx, 2);
        setState(69);
        pathSearch();
        break;
      }

      case querylangParser::T__9: {
        enterOuterAlt(_localctx, 3);
        setState(70);
        nameSearch();
        break;
      }

      case querylangParser::T__10: {
        enterOuterAlt(_localctx, 4);
        setState(71);
        sizeSearch();
        break;
      }

      case querylangParser::T__12: {
        enterOuterAlt(_localctx, 5);
        setState(72);
        typeSearch();
        break;
      }

      case querylangParser::T__13: {
        enterOuterAlt(_localctx, 6);
        setState(73);
        durationSearch();
        break;
      }

      case querylangParser::T__14: {
        enterOuterAlt(_localctx, 7);
        setState(74);
        metaSearch();
        break;
      }

      case querylangParser::T__17: {
        enterOuterAlt(_localctx, 8);
        setState(75);
        quantityCondition();
        break;
      }

      case querylangParser::T__19:
      case querylangParser::T__20: {
        enterOuterAlt(_localctx, 9);
        setState(76);
        contentSearch();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DateSearchContext ------------------------------------------------------------------

querylangParser::DateSearchContext::DateSearchContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<querylangParser::BinaryDateSearchContext *> querylangParser::DateSearchContext::binaryDateSearch() {
  return getRuleContexts<querylangParser::BinaryDateSearchContext>();
}

querylangParser::BinaryDateSearchContext* querylangParser::DateSearchContext::binaryDateSearch(size_t i) {
  return getRuleContext<querylangParser::BinaryDateSearchContext>(i);
}


size_t querylangParser::DateSearchContext::getRuleIndex() const {
  return querylangParser::RuleDateSearch;
}

void querylangParser::DateSearchContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDateSearch(this);
}

void querylangParser::DateSearchContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDateSearch(this);
}


antlrcpp::Any querylangParser::DateSearchContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<querylangVisitor*>(visitor))
    return parserVisitor->visitDateSearch(this);
  else
    return visitor->visitChildren(this);
}

querylangParser::DateSearchContext* querylangParser::dateSearch() {
  DateSearchContext *_localctx = _tracker.createInstance<DateSearchContext>(_ctx, getState());
  enterRule(_localctx, 10, querylangParser::RuleDateSearch);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(79);
    binaryDateSearch();
    setState(86);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 6, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(84);
        _errHandler->sync(this);
        switch (_input->LA(1)) {
          case querylangParser::T__1: {
            setState(80);
            match(querylangParser::T__1);
            setState(81);
            binaryDateSearch();
            break;
          }

          case querylangParser::T__2: {
            setState(82);
            match(querylangParser::T__2);
            setState(83);
            binaryDateSearch();
            break;
          }

        default:
          throw NoViableAltException(this);
        } 
      }
      setState(88);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 6, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BinaryDateSearchContext ------------------------------------------------------------------

querylangParser::BinaryDateSearchContext::BinaryDateSearchContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

querylangParser::DateSearchContext* querylangParser::BinaryDateSearchContext::dateSearch() {
  return getRuleContext<querylangParser::DateSearchContext>(0);
}

querylangParser::DateSearchinfoContext* querylangParser::BinaryDateSearchContext::dateSearchinfo() {
  return getRuleContext<querylangParser::DateSearchinfoContext>(0);
}


size_t querylangParser::BinaryDateSearchContext::getRuleIndex() const {
  return querylangParser::RuleBinaryDateSearch;
}

void querylangParser::BinaryDateSearchContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBinaryDateSearch(this);
}

void querylangParser::BinaryDateSearchContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBinaryDateSearch(this);
}


antlrcpp::Any querylangParser::BinaryDateSearchContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<querylangVisitor*>(visitor))
    return parserVisitor->visitBinaryDateSearch(this);
  else
    return visitor->visitChildren(this);
}

querylangParser::BinaryDateSearchContext* querylangParser::binaryDateSearch() {
  BinaryDateSearchContext *_localctx = _tracker.createInstance<BinaryDateSearchContext>(_ctx, getState());
  enterRule(_localctx, 12, querylangParser::RuleBinaryDateSearch);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(94);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case querylangParser::T__3: {
        enterOuterAlt(_localctx, 1);
        setState(89);
        match(querylangParser::T__3);
        setState(90);
        dateSearch();
        setState(91);
        match(querylangParser::T__4);
        break;
      }

      case querylangParser::T__5: {
        enterOuterAlt(_localctx, 2);
        setState(93);
        dateSearchinfo();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DateSearchinfoContext ------------------------------------------------------------------

querylangParser::DateSearchinfoContext::DateSearchinfoContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

querylangParser::Comparison_typeContext* querylangParser::DateSearchinfoContext::comparison_type() {
  return getRuleContext<querylangParser::Comparison_typeContext>(0);
}

querylangParser::AbsolutedateContext* querylangParser::DateSearchinfoContext::absolutedate() {
  return getRuleContext<querylangParser::AbsolutedateContext>(0);
}

querylangParser::RelativelydateContext* querylangParser::DateSearchinfoContext::relativelydate() {
  return getRuleContext<querylangParser::RelativelydateContext>(0);
}


size_t querylangParser::DateSearchinfoContext::getRuleIndex() const {
  return querylangParser::RuleDateSearchinfo;
}

void querylangParser::DateSearchinfoContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDateSearchinfo(this);
}

void querylangParser::DateSearchinfoContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDateSearchinfo(this);
}


antlrcpp::Any querylangParser::DateSearchinfoContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<querylangVisitor*>(visitor))
    return parserVisitor->visitDateSearchinfo(this);
  else
    return visitor->visitChildren(this);
}

querylangParser::DateSearchinfoContext* querylangParser::dateSearchinfo() {
  DateSearchinfoContext *_localctx = _tracker.createInstance<DateSearchinfoContext>(_ctx, getState());
  enterRule(_localctx, 14, querylangParser::RuleDateSearchinfo);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(104);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 8, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(96);
      match(querylangParser::T__5);
      setState(97);
      comparison_type();
      setState(98);
      absolutedate();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(100);
      match(querylangParser::T__5);
      setState(101);
      comparison_type();
      setState(102);
      relativelydate();
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AbsolutedateContext ------------------------------------------------------------------

querylangParser::AbsolutedateContext::AbsolutedateContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

querylangParser::StringContext* querylangParser::AbsolutedateContext::string() {
  return getRuleContext<querylangParser::StringContext>(0);
}


size_t querylangParser::AbsolutedateContext::getRuleIndex() const {
  return querylangParser::RuleAbsolutedate;
}

void querylangParser::AbsolutedateContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterAbsolutedate(this);
}

void querylangParser::AbsolutedateContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitAbsolutedate(this);
}


antlrcpp::Any querylangParser::AbsolutedateContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<querylangVisitor*>(visitor))
    return parserVisitor->visitAbsolutedate(this);
  else
    return visitor->visitChildren(this);
}

querylangParser::AbsolutedateContext* querylangParser::absolutedate() {
  AbsolutedateContext *_localctx = _tracker.createInstance<AbsolutedateContext>(_ctx, getState());
  enterRule(_localctx, 16, querylangParser::RuleAbsolutedate);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(106);
    string();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- RelativelydateContext ------------------------------------------------------------------

querylangParser::RelativelydateContext::RelativelydateContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

querylangParser::StringContext* querylangParser::RelativelydateContext::string() {
  return getRuleContext<querylangParser::StringContext>(0);
}


size_t querylangParser::RelativelydateContext::getRuleIndex() const {
  return querylangParser::RuleRelativelydate;
}

void querylangParser::RelativelydateContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRelativelydate(this);
}

void querylangParser::RelativelydateContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRelativelydate(this);
}


antlrcpp::Any querylangParser::RelativelydateContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<querylangVisitor*>(visitor))
    return parserVisitor->visitRelativelydate(this);
  else
    return visitor->visitChildren(this);
}

querylangParser::RelativelydateContext* querylangParser::relativelydate() {
  RelativelydateContext *_localctx = _tracker.createInstance<RelativelydateContext>(_ctx, getState());
  enterRule(_localctx, 18, querylangParser::RuleRelativelydate);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(108);
    match(querylangParser::T__6);
    setState(109);
    match(querylangParser::T__7);
    setState(110);
    string();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PathSearchContext ------------------------------------------------------------------

querylangParser::PathSearchContext::PathSearchContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

querylangParser::Is_or_notContext* querylangParser::PathSearchContext::is_or_not() {
  return getRuleContext<querylangParser::Is_or_notContext>(0);
}

querylangParser::StringContext* querylangParser::PathSearchContext::string() {
  return getRuleContext<querylangParser::StringContext>(0);
}


size_t querylangParser::PathSearchContext::getRuleIndex() const {
  return querylangParser::RulePathSearch;
}

void querylangParser::PathSearchContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPathSearch(this);
}

void querylangParser::PathSearchContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPathSearch(this);
}


antlrcpp::Any querylangParser::PathSearchContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<querylangVisitor*>(visitor))
    return parserVisitor->visitPathSearch(this);
  else
    return visitor->visitChildren(this);
}

querylangParser::PathSearchContext* querylangParser::pathSearch() {
  PathSearchContext *_localctx = _tracker.createInstance<PathSearchContext>(_ctx, getState());
  enterRule(_localctx, 20, querylangParser::RulePathSearch);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(112);
    match(querylangParser::T__8);
    setState(113);
    is_or_not();
    setState(114);
    string();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- NameSearchContext ------------------------------------------------------------------

querylangParser::NameSearchContext::NameSearchContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

querylangParser::StringContext* querylangParser::NameSearchContext::string() {
  return getRuleContext<querylangParser::StringContext>(0);
}


size_t querylangParser::NameSearchContext::getRuleIndex() const {
  return querylangParser::RuleNameSearch;
}

void querylangParser::NameSearchContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNameSearch(this);
}

void querylangParser::NameSearchContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNameSearch(this);
}


antlrcpp::Any querylangParser::NameSearchContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<querylangVisitor*>(visitor))
    return parserVisitor->visitNameSearch(this);
  else
    return visitor->visitChildren(this);
}

querylangParser::NameSearchContext* querylangParser::nameSearch() {
  NameSearchContext *_localctx = _tracker.createInstance<NameSearchContext>(_ctx, getState());
  enterRule(_localctx, 22, querylangParser::RuleNameSearch);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(116);
    match(querylangParser::T__9);
    setState(117);
    string();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SizeSearchContext ------------------------------------------------------------------

querylangParser::SizeSearchContext::SizeSearchContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

querylangParser::Comparison_typeContext* querylangParser::SizeSearchContext::comparison_type() {
  return getRuleContext<querylangParser::Comparison_typeContext>(0);
}

querylangParser::StringContext* querylangParser::SizeSearchContext::string() {
  return getRuleContext<querylangParser::StringContext>(0);
}

querylangParser::FilenameContext* querylangParser::SizeSearchContext::filename() {
  return getRuleContext<querylangParser::FilenameContext>(0);
}


size_t querylangParser::SizeSearchContext::getRuleIndex() const {
  return querylangParser::RuleSizeSearch;
}

void querylangParser::SizeSearchContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSizeSearch(this);
}

void querylangParser::SizeSearchContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSizeSearch(this);
}


antlrcpp::Any querylangParser::SizeSearchContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<querylangVisitor*>(visitor))
    return parserVisitor->visitSizeSearch(this);
  else
    return visitor->visitChildren(this);
}

querylangParser::SizeSearchContext* querylangParser::sizeSearch() {
  SizeSearchContext *_localctx = _tracker.createInstance<SizeSearchContext>(_ctx, getState());
  enterRule(_localctx, 24, querylangParser::RuleSizeSearch);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(128);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 9, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(119);
      match(querylangParser::T__10);
      setState(120);
      comparison_type();
      setState(121);
      string();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(123);
      match(querylangParser::T__10);
      setState(124);
      comparison_type();
      setState(125);
      match(querylangParser::T__11);
      setState(126);
      filename();
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TypeSearchContext ------------------------------------------------------------------

querylangParser::TypeSearchContext::TypeSearchContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

querylangParser::Is_or_notContext* querylangParser::TypeSearchContext::is_or_not() {
  return getRuleContext<querylangParser::Is_or_notContext>(0);
}

querylangParser::StringContext* querylangParser::TypeSearchContext::string() {
  return getRuleContext<querylangParser::StringContext>(0);
}


size_t querylangParser::TypeSearchContext::getRuleIndex() const {
  return querylangParser::RuleTypeSearch;
}

void querylangParser::TypeSearchContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterTypeSearch(this);
}

void querylangParser::TypeSearchContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitTypeSearch(this);
}


antlrcpp::Any querylangParser::TypeSearchContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<querylangVisitor*>(visitor))
    return parserVisitor->visitTypeSearch(this);
  else
    return visitor->visitChildren(this);
}

querylangParser::TypeSearchContext* querylangParser::typeSearch() {
  TypeSearchContext *_localctx = _tracker.createInstance<TypeSearchContext>(_ctx, getState());
  enterRule(_localctx, 26, querylangParser::RuleTypeSearch);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(130);
    match(querylangParser::T__12);
    setState(131);
    is_or_not();
    setState(132);
    string();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DurationSearchContext ------------------------------------------------------------------

querylangParser::DurationSearchContext::DurationSearchContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

querylangParser::Comparison_typeContext* querylangParser::DurationSearchContext::comparison_type() {
  return getRuleContext<querylangParser::Comparison_typeContext>(0);
}

querylangParser::StringContext* querylangParser::DurationSearchContext::string() {
  return getRuleContext<querylangParser::StringContext>(0);
}


size_t querylangParser::DurationSearchContext::getRuleIndex() const {
  return querylangParser::RuleDurationSearch;
}

void querylangParser::DurationSearchContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDurationSearch(this);
}

void querylangParser::DurationSearchContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDurationSearch(this);
}


antlrcpp::Any querylangParser::DurationSearchContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<querylangVisitor*>(visitor))
    return parserVisitor->visitDurationSearch(this);
  else
    return visitor->visitChildren(this);
}

querylangParser::DurationSearchContext* querylangParser::durationSearch() {
  DurationSearchContext *_localctx = _tracker.createInstance<DurationSearchContext>(_ctx, getState());
  enterRule(_localctx, 28, querylangParser::RuleDurationSearch);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(134);
    match(querylangParser::T__13);
    setState(135);
    comparison_type();
    setState(136);
    string();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- MetaSearchContext ------------------------------------------------------------------

querylangParser::MetaSearchContext::MetaSearchContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<querylangParser::StringContext *> querylangParser::MetaSearchContext::string() {
  return getRuleContexts<querylangParser::StringContext>();
}

querylangParser::StringContext* querylangParser::MetaSearchContext::string(size_t i) {
  return getRuleContext<querylangParser::StringContext>(i);
}

querylangParser::Is_or_notContext* querylangParser::MetaSearchContext::is_or_not() {
  return getRuleContext<querylangParser::Is_or_notContext>(0);
}


size_t querylangParser::MetaSearchContext::getRuleIndex() const {
  return querylangParser::RuleMetaSearch;
}

void querylangParser::MetaSearchContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterMetaSearch(this);
}

void querylangParser::MetaSearchContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitMetaSearch(this);
}


antlrcpp::Any querylangParser::MetaSearchContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<querylangVisitor*>(visitor))
    return parserVisitor->visitMetaSearch(this);
  else
    return visitor->visitChildren(this);
}

querylangParser::MetaSearchContext* querylangParser::metaSearch() {
  MetaSearchContext *_localctx = _tracker.createInstance<MetaSearchContext>(_ctx, getState());
  enterRule(_localctx, 30, querylangParser::RuleMetaSearch);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(138);
    match(querylangParser::T__14);
    setState(139);
    match(querylangParser::T__15);
    setState(140);
    string();
    setState(141);
    match(querylangParser::T__1);
    setState(142);
    match(querylangParser::T__16);
    setState(143);
    is_or_not();
    setState(144);
    string();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- QuantityConditionContext ------------------------------------------------------------------

querylangParser::QuantityConditionContext::QuantityConditionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* querylangParser::QuantityConditionContext::NUMBER_VALUE() {
  return getToken(querylangParser::NUMBER_VALUE, 0);
}


size_t querylangParser::QuantityConditionContext::getRuleIndex() const {
  return querylangParser::RuleQuantityCondition;
}

void querylangParser::QuantityConditionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterQuantityCondition(this);
}

void querylangParser::QuantityConditionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitQuantityCondition(this);
}


antlrcpp::Any querylangParser::QuantityConditionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<querylangVisitor*>(visitor))
    return parserVisitor->visitQuantityCondition(this);
  else
    return visitor->visitChildren(this);
}

querylangParser::QuantityConditionContext* querylangParser::quantityCondition() {
  QuantityConditionContext *_localctx = _tracker.createInstance<QuantityConditionContext>(_ctx, getState());
  enterRule(_localctx, 32, querylangParser::RuleQuantityCondition);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(146);
    match(querylangParser::T__17);
    setState(147);
    match(querylangParser::T__18);
    setState(148);
    match(querylangParser::NUMBER_VALUE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ContentSearchContext ------------------------------------------------------------------

querylangParser::ContentSearchContext::ContentSearchContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

querylangParser::StringContext* querylangParser::ContentSearchContext::string() {
  return getRuleContext<querylangParser::StringContext>(0);
}

querylangParser::FilenameContext* querylangParser::ContentSearchContext::filename() {
  return getRuleContext<querylangParser::FilenameContext>(0);
}


size_t querylangParser::ContentSearchContext::getRuleIndex() const {
  return querylangParser::RuleContentSearch;
}

void querylangParser::ContentSearchContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterContentSearch(this);
}

void querylangParser::ContentSearchContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitContentSearch(this);
}


antlrcpp::Any querylangParser::ContentSearchContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<querylangVisitor*>(visitor))
    return parserVisitor->visitContentSearch(this);
  else
    return visitor->visitChildren(this);
}

querylangParser::ContentSearchContext* querylangParser::contentSearch() {
  ContentSearchContext *_localctx = _tracker.createInstance<ContentSearchContext>(_ctx, getState());
  enterRule(_localctx, 34, querylangParser::RuleContentSearch);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(154);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case querylangParser::T__19: {
        enterOuterAlt(_localctx, 1);
        setState(150);
        match(querylangParser::T__19);
        setState(151);
        string();
        break;
      }

      case querylangParser::T__20: {
        enterOuterAlt(_localctx, 2);
        setState(152);
        match(querylangParser::T__20);
        setState(153);
        filename();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Comparison_typeContext ------------------------------------------------------------------

querylangParser::Comparison_typeContext::Comparison_typeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t querylangParser::Comparison_typeContext::getRuleIndex() const {
  return querylangParser::RuleComparison_type;
}

void querylangParser::Comparison_typeContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterComparison_type(this);
}

void querylangParser::Comparison_typeContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitComparison_type(this);
}


antlrcpp::Any querylangParser::Comparison_typeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<querylangVisitor*>(visitor))
    return parserVisitor->visitComparison_type(this);
  else
    return visitor->visitChildren(this);
}

querylangParser::Comparison_typeContext* querylangParser::comparison_type() {
  Comparison_typeContext *_localctx = _tracker.createInstance<Comparison_typeContext>(_ctx, getState());
  enterRule(_localctx, 36, querylangParser::RuleComparison_type);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(156);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << querylangParser::T__18)
      | (1ULL << querylangParser::T__21)
      | (1ULL << querylangParser::T__22)
      | (1ULL << querylangParser::T__23)
      | (1ULL << querylangParser::T__24)
      | (1ULL << querylangParser::T__25))) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FilenameContext ------------------------------------------------------------------

querylangParser::FilenameContext::FilenameContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

querylangParser::StringContext* querylangParser::FilenameContext::string() {
  return getRuleContext<querylangParser::StringContext>(0);
}


size_t querylangParser::FilenameContext::getRuleIndex() const {
  return querylangParser::RuleFilename;
}

void querylangParser::FilenameContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFilename(this);
}

void querylangParser::FilenameContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFilename(this);
}


antlrcpp::Any querylangParser::FilenameContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<querylangVisitor*>(visitor))
    return parserVisitor->visitFilename(this);
  else
    return visitor->visitChildren(this);
}

querylangParser::FilenameContext* querylangParser::filename() {
  FilenameContext *_localctx = _tracker.createInstance<FilenameContext>(_ctx, getState());
  enterRule(_localctx, 38, querylangParser::RuleFilename);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(158);
    string();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StringContext ------------------------------------------------------------------

querylangParser::StringContext::StringContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* querylangParser::StringContext::STRING_VALUE() {
  return getToken(querylangParser::STRING_VALUE, 0);
}


size_t querylangParser::StringContext::getRuleIndex() const {
  return querylangParser::RuleString;
}

void querylangParser::StringContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterString(this);
}

void querylangParser::StringContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitString(this);
}


antlrcpp::Any querylangParser::StringContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<querylangVisitor*>(visitor))
    return parserVisitor->visitString(this);
  else
    return visitor->visitChildren(this);
}

querylangParser::StringContext* querylangParser::string() {
  StringContext *_localctx = _tracker.createInstance<StringContext>(_ctx, getState());
  enterRule(_localctx, 40, querylangParser::RuleString);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(160);
    match(querylangParser::STRING_VALUE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Is_or_notContext ------------------------------------------------------------------

querylangParser::Is_or_notContext::Is_or_notContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t querylangParser::Is_or_notContext::getRuleIndex() const {
  return querylangParser::RuleIs_or_not;
}

void querylangParser::Is_or_notContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterIs_or_not(this);
}

void querylangParser::Is_or_notContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<querylangListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitIs_or_not(this);
}


antlrcpp::Any querylangParser::Is_or_notContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<querylangVisitor*>(visitor))
    return parserVisitor->visitIs_or_not(this);
  else
    return visitor->visitChildren(this);
}

querylangParser::Is_or_notContext* querylangParser::is_or_not() {
  Is_or_notContext *_localctx = _tracker.createInstance<Is_or_notContext>(_ctx, getState());
  enterRule(_localctx, 42, querylangParser::RuleIs_or_not);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(162);
    _la = _input->LA(1);
    if (!(_la == querylangParser::T__15

    || _la == querylangParser::T__26)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

// Static vars and initialization.
std::vector<dfa::DFA> querylangParser::_decisionToDFA;
atn::PredictionContextCache querylangParser::_sharedContextCache;

// We own the ATN which in turn owns the ATN states.
atn::ATN querylangParser::_atn;
std::vector<uint16_t> querylangParser::_serializedATN;

std::vector<std::string> querylangParser::_ruleNames = {
  "query", "invalideQuery", "primary", "binaryExpression", "searchCondition", 
  "dateSearch", "binaryDateSearch", "dateSearchinfo", "absolutedate", "relativelydate", 
  "pathSearch", "nameSearch", "sizeSearch", "typeSearch", "durationSearch", 
  "metaSearch", "quantityCondition", "contentSearch", "comparison_type", 
  "filename", "string", "is_or_not"
};

std::vector<std::string> querylangParser::_literalNames = {
  "", "'ERROR'", "'AND'", "'OR'", "'('", "')'", "'DATE'", "'CURRENT'", "'-'", 
  "'PATH'", "'NAME CONTAINS'", "'SIZE'", "'FILE_SIZE'", "'TYPE'", "'DURATION'", 
  "'META_TYPE'", "'IS'", "'META_VALUE'", "'QUANTITY'", "'='", "'CONTENT CONTAINS'", 
  "'CONTENT EQUALS FILE'", "'<'", "'>'", "'!='", "'<='", "'>='", "'IS NOT'"
};

std::vector<std::string> querylangParser::_symbolicNames = {
  "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
  "", "", "", "", "", "", "", "", "", "", "STRING_VALUE", "NUMBER_VALUE", 
  "WS"
};

dfa::Vocabulary querylangParser::_vocabulary(_literalNames, _symbolicNames);

std::vector<std::string> querylangParser::_tokenNames;

querylangParser::Initializer::Initializer() {
	for (size_t i = 0; i < _symbolicNames.size(); ++i) {
		std::string name = _vocabulary.getLiteralName(i);
		if (name.empty()) {
			name = _vocabulary.getSymbolicName(i);
		}

		if (name.empty()) {
			_tokenNames.push_back("<INVALID>");
		} else {
      _tokenNames.push_back(name);
    }
	}

  _serializedATN = {
    0x3, 0x608b, 0xa72a, 0x8133, 0xb9ed, 0x417c, 0x3be7, 0x7786, 0x5964, 
    0x3, 0x20, 0xa7, 0x4, 0x2, 0x9, 0x2, 0x4, 0x3, 0x9, 0x3, 0x4, 0x4, 0x9, 
    0x4, 0x4, 0x5, 0x9, 0x5, 0x4, 0x6, 0x9, 0x6, 0x4, 0x7, 0x9, 0x7, 0x4, 
    0x8, 0x9, 0x8, 0x4, 0x9, 0x9, 0x9, 0x4, 0xa, 0x9, 0xa, 0x4, 0xb, 0x9, 
    0xb, 0x4, 0xc, 0x9, 0xc, 0x4, 0xd, 0x9, 0xd, 0x4, 0xe, 0x9, 0xe, 0x4, 
    0xf, 0x9, 0xf, 0x4, 0x10, 0x9, 0x10, 0x4, 0x11, 0x9, 0x11, 0x4, 0x12, 
    0x9, 0x12, 0x4, 0x13, 0x9, 0x13, 0x4, 0x14, 0x9, 0x14, 0x4, 0x15, 0x9, 
    0x15, 0x4, 0x16, 0x9, 0x16, 0x4, 0x17, 0x9, 0x17, 0x3, 0x2, 0x3, 0x2, 
    0x5, 0x2, 0x31, 0xa, 0x2, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x4, 0x3, 
    0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x7, 0x4, 0x3b, 0xa, 0x4, 0xc, 0x4, 
    0xe, 0x4, 0x3e, 0xb, 0x4, 0x3, 0x5, 0x3, 0x5, 0x3, 0x5, 0x3, 0x5, 0x3, 
    0x5, 0x5, 0x5, 0x45, 0xa, 0x5, 0x3, 0x6, 0x3, 0x6, 0x3, 0x6, 0x3, 0x6, 
    0x3, 0x6, 0x3, 0x6, 0x3, 0x6, 0x3, 0x6, 0x3, 0x6, 0x5, 0x6, 0x50, 0xa, 
    0x6, 0x3, 0x7, 0x3, 0x7, 0x3, 0x7, 0x3, 0x7, 0x3, 0x7, 0x7, 0x7, 0x57, 
    0xa, 0x7, 0xc, 0x7, 0xe, 0x7, 0x5a, 0xb, 0x7, 0x3, 0x8, 0x3, 0x8, 0x3, 
    0x8, 0x3, 0x8, 0x3, 0x8, 0x5, 0x8, 0x61, 0xa, 0x8, 0x3, 0x9, 0x3, 0x9, 
    0x3, 0x9, 0x3, 0x9, 0x3, 0x9, 0x3, 0x9, 0x3, 0x9, 0x3, 0x9, 0x5, 0x9, 
    0x6b, 0xa, 0x9, 0x3, 0xa, 0x3, 0xa, 0x3, 0xb, 0x3, 0xb, 0x3, 0xb, 0x3, 
    0xb, 0x3, 0xc, 0x3, 0xc, 0x3, 0xc, 0x3, 0xc, 0x3, 0xd, 0x3, 0xd, 0x3, 
    0xd, 0x3, 0xe, 0x3, 0xe, 0x3, 0xe, 0x3, 0xe, 0x3, 0xe, 0x3, 0xe, 0x3, 
    0xe, 0x3, 0xe, 0x3, 0xe, 0x5, 0xe, 0x83, 0xa, 0xe, 0x3, 0xf, 0x3, 0xf, 
    0x3, 0xf, 0x3, 0xf, 0x3, 0x10, 0x3, 0x10, 0x3, 0x10, 0x3, 0x10, 0x3, 
    0x11, 0x3, 0x11, 0x3, 0x11, 0x3, 0x11, 0x3, 0x11, 0x3, 0x11, 0x3, 0x11, 
    0x3, 0x11, 0x3, 0x12, 0x3, 0x12, 0x3, 0x12, 0x3, 0x12, 0x3, 0x13, 0x3, 
    0x13, 0x3, 0x13, 0x3, 0x13, 0x5, 0x13, 0x9d, 0xa, 0x13, 0x3, 0x14, 0x3, 
    0x14, 0x3, 0x15, 0x3, 0x15, 0x3, 0x16, 0x3, 0x16, 0x3, 0x17, 0x3, 0x17, 
    0x3, 0x17, 0x2, 0x2, 0x18, 0x2, 0x4, 0x6, 0x8, 0xa, 0xc, 0xe, 0x10, 
    0x12, 0x14, 0x16, 0x18, 0x1a, 0x1c, 0x1e, 0x20, 0x22, 0x24, 0x26, 0x28, 
    0x2a, 0x2c, 0x2, 0x4, 0x4, 0x2, 0x15, 0x15, 0x18, 0x1c, 0x4, 0x2, 0x12, 
    0x12, 0x1d, 0x1d, 0x2, 0xa2, 0x2, 0x30, 0x3, 0x2, 0x2, 0x2, 0x4, 0x32, 
    0x3, 0x2, 0x2, 0x2, 0x6, 0x35, 0x3, 0x2, 0x2, 0x2, 0x8, 0x44, 0x3, 0x2, 
    0x2, 0x2, 0xa, 0x4f, 0x3, 0x2, 0x2, 0x2, 0xc, 0x51, 0x3, 0x2, 0x2, 0x2, 
    0xe, 0x60, 0x3, 0x2, 0x2, 0x2, 0x10, 0x6a, 0x3, 0x2, 0x2, 0x2, 0x12, 
    0x6c, 0x3, 0x2, 0x2, 0x2, 0x14, 0x6e, 0x3, 0x2, 0x2, 0x2, 0x16, 0x72, 
    0x3, 0x2, 0x2, 0x2, 0x18, 0x76, 0x3, 0x2, 0x2, 0x2, 0x1a, 0x82, 0x3, 
    0x2, 0x2, 0x2, 0x1c, 0x84, 0x3, 0x2, 0x2, 0x2, 0x1e, 0x88, 0x3, 0x2, 
    0x2, 0x2, 0x20, 0x8c, 0x3, 0x2, 0x2, 0x2, 0x22, 0x94, 0x3, 0x2, 0x2, 
    0x2, 0x24, 0x9c, 0x3, 0x2, 0x2, 0x2, 0x26, 0x9e, 0x3, 0x2, 0x2, 0x2, 
    0x28, 0xa0, 0x3, 0x2, 0x2, 0x2, 0x2a, 0xa2, 0x3, 0x2, 0x2, 0x2, 0x2c, 
    0xa4, 0x3, 0x2, 0x2, 0x2, 0x2e, 0x31, 0x5, 0x6, 0x4, 0x2, 0x2f, 0x31, 
    0x5, 0x4, 0x3, 0x2, 0x30, 0x2e, 0x3, 0x2, 0x2, 0x2, 0x30, 0x2f, 0x3, 
    0x2, 0x2, 0x2, 0x31, 0x3, 0x3, 0x2, 0x2, 0x2, 0x32, 0x33, 0x7, 0x3, 
    0x2, 0x2, 0x33, 0x34, 0x5, 0x2a, 0x16, 0x2, 0x34, 0x5, 0x3, 0x2, 0x2, 
    0x2, 0x35, 0x3c, 0x5, 0x8, 0x5, 0x2, 0x36, 0x37, 0x7, 0x4, 0x2, 0x2, 
    0x37, 0x3b, 0x5, 0x8, 0x5, 0x2, 0x38, 0x39, 0x7, 0x5, 0x2, 0x2, 0x39, 
    0x3b, 0x5, 0x8, 0x5, 0x2, 0x3a, 0x36, 0x3, 0x2, 0x2, 0x2, 0x3a, 0x38, 
    0x3, 0x2, 0x2, 0x2, 0x3b, 0x3e, 0x3, 0x2, 0x2, 0x2, 0x3c, 0x3a, 0x3, 
    0x2, 0x2, 0x2, 0x3c, 0x3d, 0x3, 0x2, 0x2, 0x2, 0x3d, 0x7, 0x3, 0x2, 
    0x2, 0x2, 0x3e, 0x3c, 0x3, 0x2, 0x2, 0x2, 0x3f, 0x40, 0x7, 0x6, 0x2, 
    0x2, 0x40, 0x41, 0x5, 0x6, 0x4, 0x2, 0x41, 0x42, 0x7, 0x7, 0x2, 0x2, 
    0x42, 0x45, 0x3, 0x2, 0x2, 0x2, 0x43, 0x45, 0x5, 0xa, 0x6, 0x2, 0x44, 
    0x3f, 0x3, 0x2, 0x2, 0x2, 0x44, 0x43, 0x3, 0x2, 0x2, 0x2, 0x45, 0x9, 
    0x3, 0x2, 0x2, 0x2, 0x46, 0x50, 0x5, 0xc, 0x7, 0x2, 0x47, 0x50, 0x5, 
    0x16, 0xc, 0x2, 0x48, 0x50, 0x5, 0x18, 0xd, 0x2, 0x49, 0x50, 0x5, 0x1a, 
    0xe, 0x2, 0x4a, 0x50, 0x5, 0x1c, 0xf, 0x2, 0x4b, 0x50, 0x5, 0x1e, 0x10, 
    0x2, 0x4c, 0x50, 0x5, 0x20, 0x11, 0x2, 0x4d, 0x50, 0x5, 0x22, 0x12, 
    0x2, 0x4e, 0x50, 0x5, 0x24, 0x13, 0x2, 0x4f, 0x46, 0x3, 0x2, 0x2, 0x2, 
    0x4f, 0x47, 0x3, 0x2, 0x2, 0x2, 0x4f, 0x48, 0x3, 0x2, 0x2, 0x2, 0x4f, 
    0x49, 0x3, 0x2, 0x2, 0x2, 0x4f, 0x4a, 0x3, 0x2, 0x2, 0x2, 0x4f, 0x4b, 
    0x3, 0x2, 0x2, 0x2, 0x4f, 0x4c, 0x3, 0x2, 0x2, 0x2, 0x4f, 0x4d, 0x3, 
    0x2, 0x2, 0x2, 0x4f, 0x4e, 0x3, 0x2, 0x2, 0x2, 0x50, 0xb, 0x3, 0x2, 
    0x2, 0x2, 0x51, 0x58, 0x5, 0xe, 0x8, 0x2, 0x52, 0x53, 0x7, 0x4, 0x2, 
    0x2, 0x53, 0x57, 0x5, 0xe, 0x8, 0x2, 0x54, 0x55, 0x7, 0x5, 0x2, 0x2, 
    0x55, 0x57, 0x5, 0xe, 0x8, 0x2, 0x56, 0x52, 0x3, 0x2, 0x2, 0x2, 0x56, 
    0x54, 0x3, 0x2, 0x2, 0x2, 0x57, 0x5a, 0x3, 0x2, 0x2, 0x2, 0x58, 0x56, 
    0x3, 0x2, 0x2, 0x2, 0x58, 0x59, 0x3, 0x2, 0x2, 0x2, 0x59, 0xd, 0x3, 
    0x2, 0x2, 0x2, 0x5a, 0x58, 0x3, 0x2, 0x2, 0x2, 0x5b, 0x5c, 0x7, 0x6, 
    0x2, 0x2, 0x5c, 0x5d, 0x5, 0xc, 0x7, 0x2, 0x5d, 0x5e, 0x7, 0x7, 0x2, 
    0x2, 0x5e, 0x61, 0x3, 0x2, 0x2, 0x2, 0x5f, 0x61, 0x5, 0x10, 0x9, 0x2, 
    0x60, 0x5b, 0x3, 0x2, 0x2, 0x2, 0x60, 0x5f, 0x3, 0x2, 0x2, 0x2, 0x61, 
    0xf, 0x3, 0x2, 0x2, 0x2, 0x62, 0x63, 0x7, 0x8, 0x2, 0x2, 0x63, 0x64, 
    0x5, 0x26, 0x14, 0x2, 0x64, 0x65, 0x5, 0x12, 0xa, 0x2, 0x65, 0x6b, 0x3, 
    0x2, 0x2, 0x2, 0x66, 0x67, 0x7, 0x8, 0x2, 0x2, 0x67, 0x68, 0x5, 0x26, 
    0x14, 0x2, 0x68, 0x69, 0x5, 0x14, 0xb, 0x2, 0x69, 0x6b, 0x3, 0x2, 0x2, 
    0x2, 0x6a, 0x62, 0x3, 0x2, 0x2, 0x2, 0x6a, 0x66, 0x3, 0x2, 0x2, 0x2, 
    0x6b, 0x11, 0x3, 0x2, 0x2, 0x2, 0x6c, 0x6d, 0x5, 0x2a, 0x16, 0x2, 0x6d, 
    0x13, 0x3, 0x2, 0x2, 0x2, 0x6e, 0x6f, 0x7, 0x9, 0x2, 0x2, 0x6f, 0x70, 
    0x7, 0xa, 0x2, 0x2, 0x70, 0x71, 0x5, 0x2a, 0x16, 0x2, 0x71, 0x15, 0x3, 
    0x2, 0x2, 0x2, 0x72, 0x73, 0x7, 0xb, 0x2, 0x2, 0x73, 0x74, 0x5, 0x2c, 
    0x17, 0x2, 0x74, 0x75, 0x5, 0x2a, 0x16, 0x2, 0x75, 0x17, 0x3, 0x2, 0x2, 
    0x2, 0x76, 0x77, 0x7, 0xc, 0x2, 0x2, 0x77, 0x78, 0x5, 0x2a, 0x16, 0x2, 
    0x78, 0x19, 0x3, 0x2, 0x2, 0x2, 0x79, 0x7a, 0x7, 0xd, 0x2, 0x2, 0x7a, 
    0x7b, 0x5, 0x26, 0x14, 0x2, 0x7b, 0x7c, 0x5, 0x2a, 0x16, 0x2, 0x7c, 
    0x83, 0x3, 0x2, 0x2, 0x2, 0x7d, 0x7e, 0x7, 0xd, 0x2, 0x2, 0x7e, 0x7f, 
    0x5, 0x26, 0x14, 0x2, 0x7f, 0x80, 0x7, 0xe, 0x2, 0x2, 0x80, 0x81, 0x5, 
    0x28, 0x15, 0x2, 0x81, 0x83, 0x3, 0x2, 0x2, 0x2, 0x82, 0x79, 0x3, 0x2, 
    0x2, 0x2, 0x82, 0x7d, 0x3, 0x2, 0x2, 0x2, 0x83, 0x1b, 0x3, 0x2, 0x2, 
    0x2, 0x84, 0x85, 0x7, 0xf, 0x2, 0x2, 0x85, 0x86, 0x5, 0x2c, 0x17, 0x2, 
    0x86, 0x87, 0x5, 0x2a, 0x16, 0x2, 0x87, 0x1d, 0x3, 0x2, 0x2, 0x2, 0x88, 
    0x89, 0x7, 0x10, 0x2, 0x2, 0x89, 0x8a, 0x5, 0x26, 0x14, 0x2, 0x8a, 0x8b, 
    0x5, 0x2a, 0x16, 0x2, 0x8b, 0x1f, 0x3, 0x2, 0x2, 0x2, 0x8c, 0x8d, 0x7, 
    0x11, 0x2, 0x2, 0x8d, 0x8e, 0x7, 0x12, 0x2, 0x2, 0x8e, 0x8f, 0x5, 0x2a, 
    0x16, 0x2, 0x8f, 0x90, 0x7, 0x4, 0x2, 0x2, 0x90, 0x91, 0x7, 0x13, 0x2, 
    0x2, 0x91, 0x92, 0x5, 0x2c, 0x17, 0x2, 0x92, 0x93, 0x5, 0x2a, 0x16, 
    0x2, 0x93, 0x21, 0x3, 0x2, 0x2, 0x2, 0x94, 0x95, 0x7, 0x14, 0x2, 0x2, 
    0x95, 0x96, 0x7, 0x15, 0x2, 0x2, 0x96, 0x97, 0x7, 0x1f, 0x2, 0x2, 0x97, 
    0x23, 0x3, 0x2, 0x2, 0x2, 0x98, 0x99, 0x7, 0x16, 0x2, 0x2, 0x99, 0x9d, 
    0x5, 0x2a, 0x16, 0x2, 0x9a, 0x9b, 0x7, 0x17, 0x2, 0x2, 0x9b, 0x9d, 0x5, 
    0x28, 0x15, 0x2, 0x9c, 0x98, 0x3, 0x2, 0x2, 0x2, 0x9c, 0x9a, 0x3, 0x2, 
    0x2, 0x2, 0x9d, 0x25, 0x3, 0x2, 0x2, 0x2, 0x9e, 0x9f, 0x9, 0x2, 0x2, 
    0x2, 0x9f, 0x27, 0x3, 0x2, 0x2, 0x2, 0xa0, 0xa1, 0x5, 0x2a, 0x16, 0x2, 
    0xa1, 0x29, 0x3, 0x2, 0x2, 0x2, 0xa2, 0xa3, 0x7, 0x1e, 0x2, 0x2, 0xa3, 
    0x2b, 0x3, 0x2, 0x2, 0x2, 0xa4, 0xa5, 0x9, 0x3, 0x2, 0x2, 0xa5, 0x2d, 
    0x3, 0x2, 0x2, 0x2, 0xd, 0x30, 0x3a, 0x3c, 0x44, 0x4f, 0x56, 0x58, 0x60, 
    0x6a, 0x82, 0x9c, 
  };

  atn::ATNDeserializer deserializer;
  _atn = deserializer.deserialize(_serializedATN);

  size_t count = _atn.getNumberOfDecisions();
  _decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    _decisionToDFA.emplace_back(_atn.getDecisionState(i), i);
  }
}

querylangParser::Initializer querylangParser::_init;
