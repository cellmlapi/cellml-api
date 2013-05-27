#define MODULE_CONTAINS_SRuS

#include "SRuSImpl.hxx"

// Find the CellMLElement corresponding to a given DOM node.
static already_AddRefd<iface::cellml_api::CellMLElement>
xmlToCellML(iface::cellml_api::Model* aModel, iface::dom::Node* aNode) throw()
{
  RETURN_INTO_OBJREF(doc, iface::dom::Document, aNode->ownerDocument());
  std::list<iface::dom::Node*> l;
  scoped_destroy<std::list<iface::dom::Node*> > lRAII
    (l,
     new container_destructor<std::list<iface::dom::Node*> >
     (
      new objref_destructor<iface::dom::Node>()
     ));
  
  // Build a list giving the path through the document...
  ObjRef<iface::dom::Node> n(aNode);
  while (n)
  {
    n->add_ref();
    l.push_front(n);

    n = already_AddRefd<iface::dom::Node>(n->parentNode());
  }

  // Get rid of anything up to and including the document element...
  while (!l.empty())
  {
    DECLARE_QUERY_INTERFACE_OBJREF(el, l.front(), dom::Element);
    l.front()->release_ref();
    l.pop_front();
    if (el != NULL)
      break;
  }

  ObjRef<iface::cellml_api::CellMLElement> cur(aModel);
  while (!l.empty())
  {
    DECLARE_QUERY_INTERFACE_OBJREF(el, l.front(), dom::Element);
    // It's an attribute, text node, or processing instruction in an element...
    // just match to the element, the remaining specificity is unneeded.
    if (el == NULL)
      break;

    RETURN_INTO_OBJREF(ces, iface::cellml_api::CellMLElementSet, cur->childElements());
    RETURN_INTO_OBJREF(cei, iface::cellml_api::CellMLElementIterator, ces->iterate());
    bool found = false;
    while (true)
    {
      RETURN_INTO_OBJREF(ce, iface::cellml_api::CellMLElement, cei->next());
      if (ce == NULL)
        break;
      DECLARE_QUERY_INTERFACE_OBJREF(dce, ce, cellml_api::CellMLDOMElement);
      if (dce != NULL)
      {
        RETURN_INTO_OBJREF(del, iface::dom::Element, dce->domElement());
        if (!CDA_objcmp(del, el))
        {
          found = true;
          cur = ce;
          l.front()->release_ref();
          l.pop_front();
          break;
        }
      }
    }

    if (!found)
      return NULL;
  }

  cur->add_ref();
  return cur.getPointer();
}

bool
CDA_SRuSProcessor::supportsModellingLanguage(const std::wstring& aLang)
  throw()
{
  if (aLang == L"http://www.cellml.org/cellml/1.1#" ||
      aLang == L"http://www.cellml.org/cellml/1.0#")
    return true;

  // In future, make this extensible so we can support other modelling languages.
  return false;
}

CDA_SRuSTransformedModel::CDA_SRuSTransformedModel(iface::dom::Document* aDoc, iface::SProS::Model* aModel)
  : mDocument(aDoc), mSEDMLModel(aModel) {}

already_AddRefd<iface::dom::Document>
CDA_SRuSTransformedModel::xmlDocument() throw()
{
  mDocument->add_ref();
  return mDocument.getPointer();
}

already_AddRefd<iface::XPCOM::IObject>
CDA_SRuSTransformedModel::modelDocument() throw(std::exception&)
{
  ensureModelOrRaise();
  mModel->add_ref();
  return mModel.getPointer();
}

already_AddRefd<iface::SProS::Model>
CDA_SRuSTransformedModel::sedmlModel() throw()
{
  mSEDMLModel->add_ref();
  return mSEDMLModel.getPointer();
}

void
CDA_SRuSTransformedModel::ensureModelOrRaise()
{
  if (mModel)
    return;
  
  RETURN_INTO_OBJREF(cb, iface::cellml_api::CellMLBootstrap, CreateCellMLBootstrap());
  RETURN_INTO_OBJREF(ml, iface::cellml_api::ModelLoader, cb->modelLoader());
  DECLARE_QUERY_INTERFACE_OBJREF(dml, ml, cellml_api::DOMModelLoader);
  try
  {
    mModel = already_AddRefd<iface::cellml_api::Model>(dml->createFromDOMDocument(mDocument));
  }
  catch (...)
  {
  }
  
  if (mModel == NULL)
    throw iface::SRuS::SRuSException();
}

class CDA_ScopedIncrement
{
public:
  CDA_ScopedIncrement(uint32_t& aInt)
    : mInt(aInt)
  {
    mInt++;
  }

  ~CDA_ScopedIncrement()
  {
    mInt--;
  }

private:
  uint32_t& mInt;
};

already_AddRefd<iface::SRuS::TransformedModel>
CDA_SRuSProcessor::buildOneModel(iface::SProS::Model* aModel)
  throw(std::exception&)
{
  try
  {
    RETURN_INTO_WSTRING(lang, aModel->language());
    if (lang != L"urn:sedml:language:cellml" &&
        lang != L"urn:sedml:language:cellml.1_0" &&
        lang != L"urn:sedml:language:cellml.1_1")
    {
      // To do: Support other languages here...
      throw iface::SRuS::SRuSException();
    }

    // Get the URI...
    RETURN_INTO_WSTRING(uri, aModel->source());

    RETURN_INTO_OBJREF(cb, iface::cellml_api::CellMLBootstrap, CreateCellMLBootstrap());
    ObjRef<iface::dom::Document> doc;

    // Check if it is an identifier for another model...
    RETURN_INTO_OBJREF(cmb, iface::SProS::Base, aModel->parent());
    DECLARE_QUERY_INTERFACE_OBJREF(cm, cmb, SProS::SEDMLElement);
    CDA_ScopedIncrement rdsi(mRecursionDepth);
    if (cm != NULL && mRecursionDepth <= 50)
    {
      RETURN_INTO_OBJREF(ms, iface::SProS::ModelSet, cm->models());
      RETURN_INTO_OBJREF(refmod, iface::SProS::Model, ms->getModelByIdentifier(uri));
      if (refmod != NULL)
      {
        RETURN_INTO_OBJREF(tmrefmod, iface::SRuS::TransformedModel, buildOneModel(refmod));
        doc = already_AddRefd<iface::dom::Document>(tmrefmod->xmlDocument());
      }
    }
    if (doc == NULL)
    {
      RETURN_INTO_OBJREF(seb, iface::SProS::Base, aModel->parent());
      DECLARE_QUERY_INTERFACE_OBJREF(se, seb, SProS::SEDMLElement);
      RETURN_INTO_WSTRING(baseURL, se->originalURL());
      RETURN_INTO_WSTRING(absURI, cb->makeURLAbsolute(baseURL, uri));
      uri = absURI;

      // Load it...
      RETURN_INTO_OBJREF(ml, iface::cellml_api::DOMURLLoader, cb->localURLLoader());
      doc = already_AddRefd<iface::dom::Document>(ml->loadDocument(uri));
      if (doc != NULL)
      {
        RETURN_INTO_OBJREF(de, iface::dom::Element, doc->documentElement());
        de->setAttributeNS(L"http://www.w3.org/XML/1998/namespace", L"base", uri);
      }
    }
    
    RETURN_INTO_OBJREF(xe, iface::xpath::XPathEvaluator, CreateXPathEvaluator());
    
    // Apply the transformations to the document...
    RETURN_INTO_OBJREF(cs, iface::SProS::ChangeSet, aModel->changes());
    RETURN_INTO_OBJREF(ci, iface::SProS::ChangeIterator, cs->iterateChanges());
    while (true)
    {
      RETURN_INTO_OBJREF(c, iface::SProS::Change, ci->nextChange());
      if (c == NULL)
        break;

      RETURN_INTO_OBJREF(de, iface::dom::Element, c->domElement());
      RETURN_INTO_OBJREF(resolver, iface::xpath::XPathNSResolver,
                         xe->createNSResolver(de));

      RETURN_INTO_WSTRING(targ, c->target());
      RETURN_INTO_OBJREF(xr, iface::xpath::XPathResult,
                         xe->evaluate(targ, doc, resolver,
                                      iface::xpath::XPathResult::UNORDERED_NODE_ITERATOR_TYPE,
                                      NULL));
      
      RETURN_INTO_OBJREF(n, iface::dom::Node, xr->iterateNext());
      if (n == NULL)
        throw iface::SRuS::SRuSException();
      
      RETURN_INTO_OBJREF(n2, iface::dom::Node, xr->iterateNext());
      if (n2)
        throw iface::SRuS::SRuSException();

      DECLARE_QUERY_INTERFACE_OBJREF(ca, c, SProS::ChangeAttribute);
      if (ca != NULL)
      {
        DECLARE_QUERY_INTERFACE_OBJREF(at, n, dom::Attr);
        if (at == NULL) // XPath target is not an attribute?
          throw iface::SRuS::SRuSException();
        RETURN_INTO_WSTRING(v, ca->newValue());
        at->value(v);
        continue;
      }

      DECLARE_QUERY_INTERFACE_OBJREF(ax, c, SProS::AddXML);
      if (ax != NULL)
      {
        RETURN_INTO_OBJREF(nl, iface::dom::NodeList, ax->anyXML());
        RETURN_INTO_OBJREF(np, iface::dom::Node, n->parentNode());

        RETURN_INTO_OBJREF(axnl, iface::dom::NodeList, ax->anyXML());
        for (uint32_t i = 0, l = axnl->length(); i < l; i++)
        {
          RETURN_INTO_OBJREF(nn, iface::dom::Node, axnl->item(i));
          np->insertBefore(nn, n)->release_ref();
        }

        // Deliberate fall-through, ChangeXML is a specific type of AddXML.
      }

      DECLARE_QUERY_INTERFACE_OBJREF(cx, c, SProS::ChangeXML);
      DECLARE_QUERY_INTERFACE_OBJREF(rx, c, SProS::RemoveXML);
      if (cx != NULL || rx != NULL)
      {
        RETURN_INTO_OBJREF(np, iface::dom::Node, n->parentNode());
        RETURN_INTO_OBJREF(tmp, iface::dom::Node, np->removeChild(n));
        continue;
      }

      DECLARE_QUERY_INTERFACE_OBJREF(cc, c, SProS::ComputeChange);
      if (cc != NULL)
      {
        SEDMLMathEvaluator sme;

        // Set parameters...
        RETURN_INTO_OBJREF(ps, iface::SProS::ParameterSet, cc->parameters());
        RETURN_INTO_OBJREF(pi, iface::SProS::ParameterIterator, ps->iterateParameters());
        while (true)
        {
          RETURN_INTO_OBJREF(p, iface::SProS::Parameter, pi->nextParameter());
          if (p == NULL)
            break;
          
          RETURN_INTO_WSTRING(pid, p->id());
          sme.setVariable(pid, p->value());
        }

        // Set variables...
        RETURN_INTO_OBJREF(vs, iface::SProS::VariableSet, cc->variables());
        RETURN_INTO_OBJREF(vi, iface::SProS::VariableIterator, vs->iterateVariables());
        while (true)
        {
          RETURN_INTO_OBJREF(v, iface::SProS::Variable, vi->nextVariable());
          if (v == NULL)
            break;

          RETURN_INTO_WSTRING(targ, v->target());
          RETURN_INTO_OBJREF(xr, iface::xpath::XPathResult,
                             xe->evaluate(targ, doc, resolver,
                                          iface::xpath::XPathResult::NUMBER_TYPE, NULL));
          RETURN_INTO_WSTRING(vid, v->id());
          sme.setVariable(vid, xr->numberValue());
        }

        RETURN_INTO_OBJREF(m, iface::mathml_dom::MathMLMathElement, cc->math());
        double v = sme.eval(m);

        RETURN_INTO_WSTRING(targ, cc->target());
        RETURN_INTO_OBJREF(xr, iface::xpath::XPathResult,
                           xe->evaluate(targ, doc, resolver,
                                        iface::xpath::XPathResult::FIRST_ORDERED_NODE_TYPE, NULL));
        RETURN_INTO_OBJREF(n, iface::dom::Node, xr->singleNodeValue());
        wchar_t vstr[30];
        any_swprintf(vstr, 30, L"%f", v);
        n->nodeValue(vstr);
      }
    }

    return new CDA_SRuSTransformedModel(doc, aModel);
  }
  catch (...)
  {
    throw iface::SRuS::SRuSException();
  }
}

CDA_SRuSTransformedModelSet::CDA_SRuSTransformedModelSet()
  : mTransformedRAII(mTransformed, new container_destructor<std::vector<iface::SRuS::TransformedModel*> >(new objref_destructor<iface::SRuS::TransformedModel>())) {}

uint32_t
CDA_SRuSTransformedModelSet::length() throw()
{
  return mTransformed.size();
}

already_AddRefd<iface::SRuS::TransformedModel>
CDA_SRuSTransformedModelSet::item(uint32_t aIdx)
  throw(std::exception&)
{
  if (aIdx >= length())
    throw iface::SRuS::SRuSException();
  
  iface::SRuS::TransformedModel* ret = mTransformed[aIdx];
  ret->add_ref();
  return ret;
}

already_AddRefd<iface::SRuS::TransformedModel>
CDA_SRuSTransformedModelSet::getItemByID(const std::wstring& aMatchID)
  throw(std::exception&)
{
  for (std::vector<iface::SRuS::TransformedModel*>::iterator i = mTransformed.begin();
       i != mTransformed.end(); i++)
  {
    RETURN_INTO_OBJREF(sm, iface::SProS::Model, (*i)->sedmlModel());
    RETURN_INTO_WSTRING(idStr, sm->id());
    if (idStr == aMatchID)
    {
      (*i)->add_ref();
      return (*i);
    }
  }
  
  return NULL;
}

void
CDA_SRuSTransformedModelSet::addTransformed
(iface::SRuS::TransformedModel* aTransformed)
{
  aTransformed->add_ref();
  mTransformed.push_back(aTransformed);
}

already_AddRefd<iface::SRuS::TransformedModelSet>
CDA_SRuSProcessor::buildAllModels(iface::SProS::SEDMLElement* aElement)
  throw()
{
  RETURN_INTO_OBJREF(tms, CDA_SRuSTransformedModelSet, new CDA_SRuSTransformedModelSet());
  RETURN_INTO_OBJREF(ms, iface::SProS::ModelSet, aElement->models());
  RETURN_INTO_OBJREF(mi, iface::SProS::ModelIterator, ms->iterateModels());
  while (true)
  {
    RETURN_INTO_OBJREF(m, iface::SProS::Model, mi->nextModel());
    if (m == NULL)
      break;

    RETURN_INTO_OBJREF(tm, iface::SRuS::TransformedModel, buildOneModel(m));
    tms->addTransformed(tm);
  }

  tms->add_ref();
  return tms.getPointer();
}

CDA_SRuSGeneratedData::CDA_SRuSGeneratedData(iface::SProS::DataGenerator* aDG)
  : mDataGenerator(aDG) {}

already_AddRefd<iface::SProS::DataGenerator>
CDA_SRuSGeneratedData::sedmlDataGenerator()
  throw()
{
  mDataGenerator->add_ref();
  return mDataGenerator.getPointer();
}

uint32_t
CDA_SRuSGeneratedData::length() throw()
{
  return mData.size();
}

double
CDA_SRuSGeneratedData::dataPoint(uint32_t idex)
  throw(std::exception&)
{
  if (idex >= length())
    throw iface::SRuS::SRuSException();
  
  return mData[idex];
}

CDA_SRuSGeneratedDataSet::CDA_SRuSGeneratedDataSet()
  : mDataRAII(mData, new container_destructor
              <std::vector<iface::SRuS::GeneratedData*> >
              (new objref_destructor<iface::SRuS::GeneratedData>())) {}

uint32_t
CDA_SRuSGeneratedDataSet::length()
  throw()
{
  return mData.size();
}

already_AddRefd<iface::SRuS::GeneratedData>
CDA_SRuSGeneratedDataSet::item(uint32_t aIdx)
  throw(std::exception&)
{
  if (aIdx >= length())
    throw iface::SRuS::SRuSException();
  
  iface::SRuS::GeneratedData* d = mData[aIdx];
  d->add_ref();
  return d;
}

void
CDA_SRuSSimulationState::resetToInitial()
{
  for (std::map<std::wstring, CDA_SRuSModelSimulationState>::iterator it
         = mPerModelState.begin(); it != mPerModelState.end(); it++)
  {
    it->second.mCurrentData = it->second.mInitialData;
    it->second.mOverrideConstants.clear();
  }
}

CDA_SRuSSimulationStep::CDA_SRuSSimulationStep
(CDA_SRuSSimulationState* aState, CDA_SRuSSimulationStep* aSuccessor)
  : mState(aState), mSuccessor(aSuccessor) {}

already_AddRefd<CDA_SRuSSimulationStep>
CDA_SRuSSimulationStepLoop::shallowClone()
{
  return new CDA_SRuSSimulationStepLoop(mState, mSuccessor, mNumPoints, mCurrentIndex,
                                        mRanges, mSetValues, mLoopChain);
}

CDA_SRuSSimulationStepDropUntil::CDA_SRuSSimulationStepDropUntil
(CDA_SRuSSimulationState* aState,
 CDA_SRuSSimulationStep* aSuccessor)
  : CDA_SRuSSimulationStep(aState, aSuccessor) {}

CDA_SRuSRawResultProcessor::CDA_SRuSRawResultProcessor
(
 iface::SRuS::GeneratedDataMonitor* aMonitor,
 iface::cellml_services::CodeInformation* aCodeInfo,
 const std::map<std::wstring, std::list<std::pair<std::wstring, int32_t> > >&
 aVarInfoByDataGeneratorId,
 const std::map<std::wstring, iface::SProS::DataGenerator*>& aDataGeneratorsById
)
  : mMonitor(aMonitor), mCodeInfo(aCodeInfo),
    mAggregateMode(0), mVarInfoByDataGeneratorId(aVarInfoByDataGeneratorId),
    mDataGeneratorsById(aDataGeneratorsById),
    mDataGeneratorsByIdRAII(mDataGeneratorsById,
                            new container_destructor<std::map<std::wstring, iface::SProS::DataGenerator*> >(new pair_both_destructor<const std::wstring, iface::SProS::DataGenerator*>(new void_destructor<const std::wstring>, new objref_destructor<iface::SProS::DataGenerator>()))), mTotalN(0)
{
  uint32_t aic = mCodeInfo->algebraicIndexCount();
  uint32_t ric = mCodeInfo->rateIndexCount();
  mRecSize = 2 * ric + aic + 1;
  
  for (std::map<std::wstring, iface::SProS::DataGenerator*>::iterator i =
         mDataGeneratorsById.begin(); i != mDataGeneratorsById.end(); i++)
    (*i).second->add_ref();
}

void
CDA_SRuSRawResultProcessor::computedConstants(const std::vector<double>& aValues)
  throw()
{
  mConstants = aValues;
}

void
CDA_SRuSRawResultProcessor::done()
  throw(std::exception&)
{
  try
  {
    if (mAggregateMode == 1)
    {
      RETURN_INTO_OBJREF(gds, CDA_SRuSGeneratedDataSet, new CDA_SRuSGeneratedDataSet());
      for (std::map<std::wstring, std::map<std::wstring, std::vector<double> > >::iterator i =
             mAggregateData.begin(); i != mAggregateData.end(); i++)
      {
        iface::SProS::DataGenerator* dg = mDataGeneratorsById[(*i).first];
        RETURN_INTO_OBJREF(m, iface::mathml_dom::MathMLMathElement, dg->math());
        
        SEDMLMathEvaluatorWithAggregate smea((*i).second);
        // Set parameters too...
        RETURN_INTO_OBJREF(ps, iface::SProS::ParameterSet, dg->parameters());
        RETURN_INTO_OBJREF(pi, iface::SProS::ParameterIterator, ps->iterateParameters());
        while (true)
        {
          RETURN_INTO_OBJREF(p, iface::SProS::Parameter, pi->nextParameter());
          if (p == NULL)
            break;
          
          RETURN_INTO_WSTRING(pid, p->id());
          smea.setVariable(pid, p->value());
        }
        
        RETURN_INTO_OBJREF(gd, CDA_SRuSGeneratedData, new CDA_SRuSGeneratedData(dg));
        for (uint32_t j = 0; j < mTotalN; j++)
        {
          for (std::map<std::wstring, std::vector<double> >::iterator vi = (*i).second.begin();
               vi != (*i).second.end(); vi++)
            smea.setVariable((*vi).first, (*vi).second[j]);
          gd->mData.push_back(smea.eval(m));
        }
        
        gd->add_ref();
        gds->mData.push_back(gd);
      }
    }
    mMonitor->done();
  }
  catch (...) {}
}

void
CDA_SRuSRawResultProcessor::failed(const std::string& aErrorMessage)
  throw(std::exception&)
{
  try
  {
    mMonitor->failure(aErrorMessage);
  }
  catch (...)
  {
  }
}

void
CDA_SRuSRawResultProcessor::results(const std::vector<double>& state)
  throw(std::exception&)
{
  uint32_t n = state.size() / mRecSize;
  mTotalN += n;
  if (mAggregateMode == 1)
  {
    for (std::map<std::wstring, std::list<std::pair<std::wstring, int32_t> > >::iterator i =
           mVarInfoByDataGeneratorId.begin(); i != mVarInfoByDataGeneratorId.end(); i++)
      for (std::list<std::pair<std::wstring, int32_t> >::iterator li = (*i).second.begin();
           li != (*i).second.end(); li++)
      {
        int32_t idx = (*li).second;
        std::vector<double>& l = mAggregateData[(*i).first][(*li).first];
        for (uint32_t j = 0; j < n; j++)
          l.push_back(idx < 0 ? mConstants[-1 - idx] : state[j * mRecSize + idx]);
      }
    return;
  }

  SEDMLMathEvaluator sme;
  if (mAggregateMode == 0)
    sme.setExploreEverything(true);
  
  RETURN_INTO_OBJREF(gds, CDA_SRuSGeneratedDataSet, new CDA_SRuSGeneratedDataSet());
  
  for (std::map<std::wstring, std::list<std::pair<std::wstring, int32_t> > >::iterator i =
         mVarInfoByDataGeneratorId.begin(); i != mVarInfoByDataGeneratorId.end(); i++)
  {
    iface::SProS::DataGenerator* dg = mDataGeneratorsById[(*i).first];
    // Set parameters too...
    RETURN_INTO_OBJREF(ps, iface::SProS::ParameterSet, dg->parameters());
    RETURN_INTO_OBJREF(pi, iface::SProS::ParameterIterator, ps->iterateParameters());
    while (true)
    {
      RETURN_INTO_OBJREF(p, iface::SProS::Parameter, pi->nextParameter());
      if (p == NULL)
        break;
      
      RETURN_INTO_WSTRING(pid, p->id());
      sme.setVariable(pid, p->value());
    }
    
    RETURN_INTO_OBJREF(gd, CDA_SRuSGeneratedData, new CDA_SRuSGeneratedData(dg));
    
    for (uint32_t j = 0; j < n; j++)
    {
      for (std::list<std::pair<std::wstring, int32_t> >::iterator li = (*i).second.begin();
           li != (*i).second.end(); li++)
        sme.setVariable((*li).first, (*li).second < 0 ?
                        mConstants[-1 - (*li).second] :
                        state[j * mRecSize + (*li).second]);
      
      try
      {
        RETURN_INTO_OBJREF(m, iface::mathml_dom::MathMLMathElement, dg->math());
        double v = sme.eval(m);
        gd->mData.push_back(v);
      }
      catch (NeedsAggregate&)
      {
        mAggregateMode = 1;
        results(state);
        return;
      }
    }
    
    gd->add_ref();
    gds->mData.push_back(gd);
  }
  
  mMonitor->progress(gds);
}

already_AddRefd<CDA_SRuSSimulationStep>
CDA_SRuSSimulationStep::cloneChangingLastSuccessor(CDA_SRuSSimulationStep* aNewEnd)
{
  CDA_SRuSSimulationStep* cur = this;
  ObjRef<CDA_SRuSSimulationStep> firstNewStep(cur->shallowClone());
  CDA_SRuSSimulationStep* newStep = firstNewStep;

  while (newStep->mSuccessor)
  {
    cur = cur->mSuccessor;
    newStep->mSuccessor = cur->shallowClone();
    newStep = newStep->mSuccessor;
  }
  newStep->mSuccessor = aNewEnd;

  return firstNewStep.returnNewReference();
}

void
CDA_SRuSSimulationStep::performNext()
{
  if (mSuccessor == NULL)
    mState->mResultsTo->done();
  else
    mSuccessor->perform();
}

static int countPointsInRange(iface::SProS::Range* aRange)
{
  if (aRange == NULL)
    return -1;
  ObjRef<iface::SProS::VectorRange> vRange(QueryInterface(aRange));
  if (vRange != NULL)
    return static_cast<int>(vRange->numberOfValues());

  ObjRef<iface::SProS::UniformRange> uRange(QueryInterface(aRange));
  if (uRange != NULL)
    return static_cast<int>(uRange->numberOfPoints());
  
  return -1;
}

CDA_SRuSSimulationStepLoop::CDA_SRuSSimulationStepLoop
(
 CDA_SRuSSimulationState* aState,
 CDA_SRuSSimulationStep* aSuccessor,
 const std::wstring& aMasterRangeId,
 std::list<iface::SProS::Range*>& aRanges,
 iface::SProS::SetValueSet* aSetValues,
 CDA_SRuSSimulationStep* aLoopChain
)
  : 
  CDA_SRuSSimulationStep(aState, aSuccessor),
  mCurrentIndex(0), mRanges(aRanges.begin(), aRanges.end()),
  mRangesRAII(mRanges, new container_destructor<std::list<iface::SProS::Range*> >
              (new objref_destructor<iface::SProS::Range>())),
  mSetValues(aSetValues), mLoopChain(aLoopChain)
{
  for (std::list<iface::SProS::Range*>::iterator it = mRanges.begin();
       it != mRanges.end();
       it++)
    (*it)->add_ref();

  if (mRanges.empty())
    throw iface::SRuS::SRuSException();

  mNumPoints = countPointsInRange(mRanges.front());

  if (mNumPoints == -1) // All ranges are functionalRange? Not valid.
    throw iface::SRuS::SRuSException();

  bool foundMasterRange = false;
  std::map<std::wstring, iface::SProS::Range*> rangeByName;
  for (std::list<iface::SProS::Range*>::iterator it = mRanges.begin();
       it != mRanges.end();
       it++)
    rangeByName.insert(std::pair<std::wstring, iface::SProS::Range*>((*it)->id(), *it));
  std::wstring sizeDeterminingName = aMasterRangeId;
  
  while (true)
  {
    std::map<std::wstring, iface::SProS::Range*>::iterator it =
      rangeByName.find(sizeDeterminingName);
    if (it == rangeByName.end())
      throw iface::SRuS::SRuSException();

    mNumPoints = countPointsInRange(it->second);
    if (mNumPoints != -1)
      break;

    ObjRef<iface::SProS::FunctionalRange> fRange(QueryInterface(it->second));
    sizeDeterminingName = fRange->indexName();
  }

  for (std::list<iface::SProS::Range*>::iterator it = mRanges.begin();
       it != mRanges.end();
       it++)
  {
    int count = countPointsInRange(*it);
    // It's invalid to have less points than the range named in the 'range'
    // attribute in any range.
    if (count != -1 && count < mNumPoints)
      throw iface::SRuS::SRuSException();
  }
}

CDA_SRuSSimulationStepLoop::CDA_SRuSSimulationStepLoop
(
 CDA_SRuSSimulationState* aState,
 CDA_SRuSSimulationStep* aSuccessor,
 int aNumPoints, int aCurrentIndex,
 std::list<iface::SProS::Range*>& aRanges,
 iface::SProS::SetValueSet* aSetValues,
 CDA_SRuSSimulationStep* aLoopChain
)
 : CDA_SRuSSimulationStep(aState, aSuccessor),
   mNumPoints(aNumPoints),
   mCurrentIndex(aCurrentIndex), mRanges(aRanges.begin(), aRanges.end()),
   mRangesRAII(mRanges, new container_destructor<std::list<iface::SProS::Range*> >
               (new objref_destructor<iface::SProS::Range>())),
   mSetValues(aSetValues), mLoopChain(aLoopChain)

{
  for (std::list<iface::SProS::Range*>::iterator it = mRanges.begin();
       it != mRanges.end();
       it++)
    (*it)->add_ref();
}


double
CDA_SRuSSimulationStepLoop::getRangeValueFor
(
 iface::SProS::Range* aRange,
 std::map<std::wstring, double>& aCurrentRangeValues
)
{
  ObjRef<iface::SProS::VectorRange> vRange(QueryInterface(aRange));
  if (vRange != NULL)
    return (vRange->valueAt(mCurrentIndex));

  ObjRef<iface::SProS::UniformRange> uRange(QueryInterface(aRange));
  if (uRange != NULL)
    return uRange->start() +
      ((uRange->end() - uRange->start()) / (uRange->numberOfPoints() - 1)) *
      mCurrentIndex;
  
  ObjRef<iface::SProS::FunctionalRange> fRange(QueryInterface(aRange));
  assert(fRange != NULL);
  ObjRef<iface::mathml_dom::MathMLMathElement> mathFunc(fRange->function());

  SEDMLMathEvaluator eval;
  for (std::map<std::wstring, double>::iterator it = aCurrentRangeValues.begin();
       it != aCurrentRangeValues.end(); it++)
    eval.setVariable(it->first, it->second);
  
  ObjRef<iface::SProS::VariableSet> variables(fRange->variables());
  ObjRef<iface::SProS::VariableIterator> varIt(variables->iterateVariables());
  for (ObjRef<iface::SProS::Variable> var = varIt->nextVariable();
       var != NULL; var = varIt->nextVariable())
  {
    std::wstring expr = var->target();

    ObjRef<iface::SRuS::TransformedModel> tm
      (mState->mTMS->getItemByID(var->modelReferenceIdentifier()));
    if (tm == NULL)
      throw iface::SRuS::SRuSException();

    ObjRef<iface::xpath::XPathEvaluator> xe(CreateXPathEvaluator());
    ObjRef<iface::dom::Document> doc(tm->xmlDocument());
    ObjRef<iface::dom::Element> de(doc->documentElement());
    ObjRef<iface::xpath::XPathNSResolver> resolver
      (xe->createNSResolver(de));
    ObjRef<iface::xpath::XPathResult> xr
      (xe->evaluate(expr, doc, resolver,
                    iface::xpath::XPathResult::FIRST_ORDERED_NODE_TYPE,
                    NULL));
    RETURN_INTO_OBJREF(n, iface::dom::Node, xr->singleNodeValue());
    if (n == NULL)
      throw iface::SRuS::SRuSException();

    ObjRef<iface::cellml_api::Model> m(QueryInterface(tm->modelDocument()));
    ObjRef<iface::cellml_api::CellMLElement> el(xmlToCellML(m, n));
    ObjRef<iface::cellml_api::CellMLVariable> cv(QueryInterface(el));
    if (cv == NULL)
      throw iface::SRuS::SRuSException();

    std::map<std::wstring, CDA_SRuSModelSimulationState>::iterator modelState
      (mState->mPerModelState.find(var->modelReferenceIdentifier()));

    ObjRef<iface::cellml_services::ComputationTargetIterator>
      cti(modelState->second.mCodeInfo->iterateTargets());

    bool foundCT = false;
    double ctValue = 0.0;

    for (ObjRef<iface::cellml_services::ComputationTarget> ct =
           cti->nextComputationTarget(); ct != NULL;
         ct = cti->nextComputationTarget())
    {
      ObjRef<iface::cellml_api::CellMLVariable> ctv(ct->variable());
      if (!CDA_objcmp(ctv, cv))
      {
        int32_t idx = ct->assignedIndex();
        iface::cellml_services::VariableEvaluationType t(ct->type());
        switch (t)
        {
        case iface::cellml_services::VARIABLE_OF_INTEGRATION:
          idx = 0;
          break;
        case iface::cellml_services::CONSTANT:
          idx = -1 - idx;
          break;
        case iface::cellml_services::STATE_VARIABLE:
        case iface::cellml_services::PSEUDOSTATE_VARIABLE:
          idx++;
          break;
        case iface::cellml_services::ALGEBRAIC:
          idx += 1 + modelState->second.mCodeInfo->rateIndexCount() * 2;
          break;
        default:
          throw iface::SRuS::SRuSException();
        }
        if (idx < 0)
          ctValue = modelState->second.mCurrentConstants.size() > (-1-idx) ?
            modelState->second.mCurrentConstants[-1-idx] : 0.0;
        else
          ctValue = modelState->second.mCurrentData.size() > idx ?
            modelState->second.mCurrentData[idx] : 0.0;
        foundCT = true;
        break;
      }
    }

    if (!foundCT)
      throw iface::SRuS::SRuSException();

    eval.setVariable(var->id(), ctValue);
  }

  return eval.eval(mathFunc);
}

void
CDA_SRuSSimulationStepLoop::perform()
{
  if (mCurrentIndex == mNumPoints)
    performNext();
  else
  {
    std::map<std::wstring, double> currentRangeValues;
    for (std::list<iface::SProS::Range*>::iterator it = mRanges.begin();
         it != mRanges.end();
         it++)
      currentRangeValues[(*it)->id()] = getRangeValueFor(*it, currentRangeValues);

    ObjRef<iface::SProS::SetValueIterator> svi(mSetValues->iterateSetValues());
    for (ObjRef<iface::SProS::SetValue> sv(svi->nextSetValue()); sv; sv = svi->nextSetValue())
    {
      SEDMLMathEvaluator eval;
      std::wstring range(sv->rangeIdentifier());
      if (range != L"")
      {
        std::map<std::wstring, double>::iterator it =
          currentRangeValues.find(range);
        if (it != currentRangeValues.end())
          eval.setVariable(range, it->second);
      }

      // Now we set all variables that may be used on the evaluator...
      ObjRef<iface::SProS::VariableSet> svvs(sv->variables());
      ObjRef<iface::SProS::VariableIterator> svvi(svvs->iterateVariables());
      for (ObjRef<iface::SProS::Variable> svv(svvi->nextVariable()); svv;
           svv = svvi->nextVariable())
      {
        
      }
      
      double setToValue;

      ObjRef<iface::SRuS::TransformedModel> tm
        (mState->mTMS->getItemByID(sv->modelReferenceIdentifier()));
      if (tm == NULL)
        throw iface::SRuS::SRuSException();

      ObjRef<iface::xpath::XPathEvaluator> xe(CreateXPathEvaluator());
      ObjRef<iface::dom::Document> doc(tm->xmlDocument());
      ObjRef<iface::dom::Element> de(doc->documentElement());
      ObjRef<iface::xpath::XPathNSResolver> resolver
        (xe->createNSResolver(de));
      ObjRef<iface::xpath::XPathResult> xr
        (xe->evaluate(sv->target(), doc, resolver,
                      iface::xpath::XPathResult::FIRST_ORDERED_NODE_TYPE,
                      NULL));
      RETURN_INTO_OBJREF(n, iface::dom::Node, xr->singleNodeValue());
      if (n == NULL)
        throw iface::SRuS::SRuSException();

      ObjRef<iface::cellml_api::Model> m(QueryInterface(tm->modelDocument()));
      ObjRef<iface::cellml_api::CellMLElement> el(xmlToCellML(m, n));
      ObjRef<iface::cellml_api::CellMLVariable> cv(QueryInterface(el));
      if (cv == NULL)
        throw iface::SRuS::SRuSException();

      std::map<std::wstring, CDA_SRuSModelSimulationState>::iterator modelState
        (mState->mPerModelState.find(sv->modelReferenceIdentifier()));

      ObjRef<iface::cellml_services::ComputationTargetIterator>
        cti(modelState->second.mCodeInfo->iterateTargets());

      bool foundCT = false;
      double ctValue = 0.0;
      
      for (ObjRef<iface::cellml_services::ComputationTarget> ct =
             cti->nextComputationTarget(); ct != NULL;
           ct = cti->nextComputationTarget())
      {
        ObjRef<iface::cellml_api::CellMLVariable> ctv(ct->variable());
        if (!CDA_objcmp(ctv, cv))
        {
          int32_t idx = ct->assignedIndex();
          iface::cellml_services::VariableEvaluationType t(ct->type());
          switch (t)
          {
          case iface::cellml_services::VARIABLE_OF_INTEGRATION:
            idx = 0;
            break;
          case iface::cellml_services::CONSTANT:
            idx = -1 - idx;
            break;
          case iface::cellml_services::STATE_VARIABLE:
          case iface::cellml_services::PSEUDOSTATE_VARIABLE:
            idx++;
            break;
          case iface::cellml_services::ALGEBRAIC:
            idx += 1 + modelState->second.mCodeInfo->rateIndexCount() * 2;
            break;
          default:
            throw iface::SRuS::SRuSException();
          }
          if (idx < 0)
            modelState->second.mOverrideConstants.insert
              (std::pair<int, double>(-1-idx, setToValue));
          else
            modelState->second.mOverrideData.insert
              (std::pair<int, double>(idx, setToValue));
          foundCT = true;
          break;
        }
      }

      if (!foundCT)
        throw iface::SRuS::SRuSException();
    }

    // We now have all the range values.
    mCurrentIndex++;
    
    // We make a new chain where we do all the subtask steps and then come
    // back to recheck the loop at the end.
    ObjRef<CDA_SRuSSimulationStep> realSuccessor
      (mLoopChain->cloneChangingLastSuccessor(mCurrentIndex == mNumPoints ?
                                              this->mSuccessor : this));
    realSuccessor->performNext();
  }
}

// TODO: Replace this with something based on the SRuSSimulationStep interface.
class CDA_SRuSResultBridge
  : public iface::cellml_services::IntegrationProgressObserver
{
public:
  CDA_SRuSResultBridge(iface::cellml_services::CellMLIntegrationRun* aRun,
                       iface::SRuS::GeneratedDataMonitor* aMonitor,
                       iface::cellml_services::CodeInformation* aCodeInfo,
                       const std::map<std::wstring, std::list<std::pair<std::wstring, int32_t> > >&
                       aVarInfoByDataGeneratorId,
                       const std::map<std::wstring, iface::SProS::DataGenerator*>& aDataGeneratorsById)
    : mRun(aRun), mMonitor(aMonitor), mCodeInfo(aCodeInfo),
      mAggregateMode(0), mVarInfoByDataGeneratorId(aVarInfoByDataGeneratorId),
      mDataGeneratorsById(aDataGeneratorsById),
      mDataGeneratorsByIdRAII(mDataGeneratorsById,
                              new container_destructor<std::map<std::wstring, iface::SProS::DataGenerator*> >(new pair_both_destructor<const std::wstring, iface::SProS::DataGenerator*>(new void_destructor<const std::wstring>, new objref_destructor<iface::SProS::DataGenerator>()))), mTotalN(0)
  {
    uint32_t aic = mCodeInfo->algebraicIndexCount();
    uint32_t ric = mCodeInfo->rateIndexCount();
    mRecSize = 2 * ric + aic + 1;

    for (std::map<std::wstring, iface::SProS::DataGenerator*>::iterator i =
           mDataGeneratorsById.begin(); i != mDataGeneratorsById.end(); i++)
      (*i).second->add_ref();
  }

  ~CDA_SRuSResultBridge()
  {
  }

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;
  CDA_IMPL_QI1(cellml_services::IntegrationProgressObserver);

  void computedConstants(const std::vector<double>& aValues) throw()
  {
    mConstants = aValues;
  }

  void done()
    throw(std::exception&)
  {
    try
    {
      if (mAggregateMode == 1)
      {
        RETURN_INTO_OBJREF(gds, CDA_SRuSGeneratedDataSet, new CDA_SRuSGeneratedDataSet());
        for (std::map<std::wstring, std::map<std::wstring, std::vector<double> > >::iterator i =
               mAggregateData.begin(); i != mAggregateData.end(); i++)
        {
          iface::SProS::DataGenerator* dg = mDataGeneratorsById[(*i).first];
          RETURN_INTO_OBJREF(m, iface::mathml_dom::MathMLMathElement, dg->math());
          
          SEDMLMathEvaluatorWithAggregate smea((*i).second);
          // Set parameters too...
          RETURN_INTO_OBJREF(ps, iface::SProS::ParameterSet, dg->parameters());
          RETURN_INTO_OBJREF(pi, iface::SProS::ParameterIterator, ps->iterateParameters());
          while (true)
          {
            RETURN_INTO_OBJREF(p, iface::SProS::Parameter, pi->nextParameter());
            if (p == NULL)
              break;
            
            RETURN_INTO_WSTRING(pid, p->id());
            smea.setVariable(pid, p->value());
          }
          
          RETURN_INTO_OBJREF(gd, CDA_SRuSGeneratedData, new CDA_SRuSGeneratedData(dg));
          for (uint32_t j = 0; j < mTotalN; j++)
          {
            for (std::map<std::wstring, std::vector<double> >::iterator vi = (*i).second.begin();
                 vi != (*i).second.end(); vi++)
              smea.setVariable((*vi).first, (*vi).second[j]);
            gd->mData.push_back(smea.eval(m));
          }
          
          gd->add_ref();
          gds->mData.push_back(gd);
        }
      }
      mMonitor->done();
    }
    catch (...) {}

    mRun = NULL;
  }

  void failed(const std::string& aErrorMessage)
    throw(std::exception&)
  {
    try
    {
      mMonitor->failure(aErrorMessage);
    }
    catch (...)
    {
    }

    mRun = NULL;
  }

  void results(const std::vector<double>& state)
    throw(std::exception&)
  {
    uint32_t n = state.size() / mRecSize;
    mTotalN += n;
    if (mAggregateMode == 1)
    {
      for (std::map<std::wstring, std::list<std::pair<std::wstring, int32_t> > >::iterator i =
             mVarInfoByDataGeneratorId.begin(); i != mVarInfoByDataGeneratorId.end(); i++)
        for (std::list<std::pair<std::wstring, int32_t> >::iterator li = (*i).second.begin();
             li != (*i).second.end(); li++)
        {
          int32_t idx = (*li).second;
          std::vector<double>& l = mAggregateData[(*i).first][(*li).first];
          for (uint32_t j = 0; j < n; j++)
            l.push_back(idx < 0 ? mConstants[-1 - idx] : state[j * mRecSize + idx]);
        }
      return;
    }

    SEDMLMathEvaluator sme;
    if (mAggregateMode == 0)
      sme.setExploreEverything(true);

    RETURN_INTO_OBJREF(gds, CDA_SRuSGeneratedDataSet, new CDA_SRuSGeneratedDataSet());

    for (std::map<std::wstring, std::list<std::pair<std::wstring, int32_t> > >::iterator i =
           mVarInfoByDataGeneratorId.begin(); i != mVarInfoByDataGeneratorId.end(); i++)
    {
      iface::SProS::DataGenerator* dg = mDataGeneratorsById[(*i).first];
      // Set parameters too...
      RETURN_INTO_OBJREF(ps, iface::SProS::ParameterSet, dg->parameters());
      RETURN_INTO_OBJREF(pi, iface::SProS::ParameterIterator, ps->iterateParameters());
      while (true)
      {
        RETURN_INTO_OBJREF(p, iface::SProS::Parameter, pi->nextParameter());
        if (p == NULL)
          break;
        
          RETURN_INTO_WSTRING(pid, p->id());
          sme.setVariable(pid, p->value());
      }

      RETURN_INTO_OBJREF(gd, CDA_SRuSGeneratedData, new CDA_SRuSGeneratedData(dg));

      for (uint32_t j = 0; j < n; j++)
      {
        for (std::list<std::pair<std::wstring, int32_t> >::iterator li = (*i).second.begin();
             li != (*i).second.end(); li++)
          sme.setVariable((*li).first, (*li).second < 0 ?
                           mConstants[-1 - (*li).second] :
                           state[j * mRecSize + (*li).second]);
        
        try
        {
          RETURN_INTO_OBJREF(m, iface::mathml_dom::MathMLMathElement, dg->math());
          double v = sme.eval(m);
          gd->mData.push_back(v);
        }
        catch (NeedsAggregate&)
        {
          mAggregateMode = 1;
          results(state);
          return;
        }
      }

      gd->add_ref();
      gds->mData.push_back(gd);
    }

    mMonitor->progress(gds);
  }

private:
  ObjRef<iface::cellml_services::CellMLIntegrationRun> mRun;
  ObjRef<iface::SRuS::GeneratedDataMonitor> mMonitor;
  ObjRef<iface::cellml_services::CodeInformation> mCodeInfo;
  // 0: Unknown. 1: Need aggregate. -1: Don't need aggregate.
  int mAggregateMode;
  std::map<std::wstring, std::list<std::pair<std::wstring, int32_t> > >
    mVarInfoByDataGeneratorId;
  std::vector<double> mConstants;
  std::map<std::wstring, std::map<std::wstring, std::vector<double> > > mAggregateData;
  std::map<std::wstring, iface::SProS::DataGenerator*> mDataGeneratorsById;
  scoped_destroy<std::map<std::wstring, iface::SProS::DataGenerator*> > mDataGeneratorsByIdRAII;
  uint32_t mRecSize, mTotalN;
};

// TODO: Replace this with something based on the SRuSSimulationStep interface.
class CDA_SRuSContinueAtStartTime
  : public iface::cellml_services::IntegrationProgressObserver
{
public:
  CDA_SRuSContinueAtStartTime(iface::cellml_services::CellMLIntegrationRun* aRun,
                              iface::SRuS::GeneratedDataMonitor* aMonitor,
                              iface::cellml_services::CodeInformation* aCodeInfo,
                              const std::map<std::wstring, std::list<std::pair<std::wstring, int32_t> > >&
                                aVarInfoByDataGeneratorId,
                              std::map<std::wstring, iface::SProS::DataGenerator*>& aDataGeneratorsById
                             )
    : mRun(aRun), mMonitor(aMonitor), mCodeInfo(aCodeInfo),
      mVarInfoByDataGeneratorId(aVarInfoByDataGeneratorId),
      mDataGeneratorsById(aDataGeneratorsById),
      mDataGeneratorsByIdRAII(mDataGeneratorsById,
                              new container_destructor<std::map<std::wstring, iface::SProS::DataGenerator*> >(new pair_both_destructor<const std::wstring, iface::SProS::DataGenerator*>(new void_destructor<const std::wstring>, new objref_destructor<iface::SProS::DataGenerator>()))
                              )
  {
    uint32_t aic = mCodeInfo->algebraicIndexCount();
    uint32_t ric = mCodeInfo->rateIndexCount();
    mRecSize = 2 * ric + aic + 1;
    mRow = new double[mRecSize];

    for (std::map<std::wstring, iface::SProS::DataGenerator*>::iterator i =
           mDataGeneratorsById.begin(); i != mDataGeneratorsById.end(); i++)
      (*i).second->add_ref();
  }

  ~CDA_SRuSContinueAtStartTime()
  {
    delete [] mRow;
  }

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;
  CDA_IMPL_QI1(cellml_services::IntegrationProgressObserver);

  void computedConstants(const std::vector<double>&) throw()
  {
  }

  void failed(const std::string& aMessage)
    throw (std::exception&)
  {
    mMonitor->failure(aMessage);
  }

  void results(const std::vector<double>& state)
    throw (std::exception&)
  {
    uint32_t nRows = state.size() / mRecSize;
    uint32_t offs = (nRows - 1) * mRecSize;
    for (uint32_t i = 0; i < mRecSize; i++)
      mRow[i] = state[i + offs];
  }

  void done()
    throw (std::exception&)
  {
    // Start the main run...
    RETURN_INTO_OBJREF(rb, CDA_SRuSResultBridge,
                       new CDA_SRuSResultBridge(mRun, mMonitor, mCodeInfo, mVarInfoByDataGeneratorId, mDataGeneratorsById));
    mRun->setProgressObserver(rb);
    // Time, States, Rates, Algebraic
    RETURN_INTO_OBJREF(cti, iface::cellml_services::ComputationTargetIterator,
                       mCodeInfo->iterateTargets());
    while (true)
    {
      RETURN_INTO_OBJREF(ct, iface::cellml_services::ComputationTarget, cti->nextComputationTarget());
      if (ct == NULL)
        break;

      if (ct->type() == iface::cellml_services::STATE_VARIABLE)
        mRun->setOverride(iface::cellml_services::STATE_VARIABLE,
                          ct->assignedIndex(), mRow[1 + ct->assignedIndex()]);
    }
    mRun->start();
  }

private:
  ObjRef<iface::cellml_services::CellMLIntegrationRun> mRun;
  ObjRef<iface::SRuS::GeneratedDataMonitor> mMonitor;
  ObjRef<iface::cellml_services::CodeInformation> mCodeInfo;
  uint32_t mRecSize;
  double* mRow;
  std::map<std::wstring, std::list<std::pair<std::wstring, int32_t> > >
    mVarInfoByDataGeneratorId;
  std::map<std::wstring, iface::SProS::DataGenerator*>
    mDataGeneratorsById;
  scoped_destroy<std::map<std::wstring, iface::SProS::DataGenerator*> >
    mDataGeneratorsByIdRAII;
};

void
CDA_SRuSProcessor::generateData
(
 iface::SRuS::TransformedModelSet* aSet,
 iface::SProS::SEDMLElement* aElement,
 iface::SRuS::GeneratedDataMonitor* aMonitor
)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(is, iface::cellml_services::CellMLIntegrationService,
                     CreateIntegrationService());

  std::map<std::wstring, iface::SRuS::TransformedModel*> modelsById;
  scoped_destroy<std::map<std::wstring, iface::SRuS::TransformedModel*> >
    modelsByIdRAII(modelsById,
                   new container_destructor<std::map<std::wstring, iface::SRuS::TransformedModel*> >(new pair_both_destructor<const std::wstring, iface::SRuS::TransformedModel*>(new void_destructor<const std::wstring>, new objref_destructor<iface::SRuS::TransformedModel>())));
  std::map<std::wstring, iface::SProS::AbstractTask*> tasksById;
  scoped_destroy<std::map<std::wstring, iface::SProS::AbstractTask*> >
    tasksByIdRAII(tasksById,
                  new container_destructor<std::map<std::wstring, iface::SProS::AbstractTask*> >(new pair_both_destructor<const std::wstring, iface::SProS::AbstractTask*>(new void_destructor<const std::wstring>, new objref_destructor<iface::SProS::AbstractTask>())));
  
  std::set<std::wstring> activeTasks;
  std::multimap<std::wstring, std::wstring> dataGeneratorIdsByTaskId;

  RETURN_INTO_OBJREF(ts, iface::SProS::TaskSet, aElement->tasks());
  RETURN_INTO_OBJREF(ti, iface::SProS::TaskIterator, ts->iterateTasks());
  while (true)
  {
    RETURN_INTO_OBJREF(t, iface::SProS::AbstractTask, ti->nextTask());
    if (t == NULL)
      break;
    std::wstring idStr = t->id();
    t->add_ref();
    tasksById.insert(std::pair<std::wstring, iface::SProS::AbstractTask*>(idStr, t));
  }

  for (uint32_t i = 0, l = aSet->length(); i < l; i++)
  {
    RETURN_INTO_OBJREF(tm, iface::SRuS::TransformedModel, aSet->item(i));
    RETURN_INTO_OBJREF(sm, iface::SProS::Model, tm->sedmlModel());
    std::wstring idStr = sm->id();
    tm->add_ref();
    modelsById.insert(std::pair<std::wstring, iface::SRuS::TransformedModel*>(idStr, tm));
  }

  RETURN_INTO_OBJREF(gens, iface::SProS::DataGeneratorSet,
                     aElement->generators());
  RETURN_INTO_OBJREF(geni, iface::SProS::DataGeneratorIterator,
                     gens->iterateDataGenerators());

  std::map<std::wstring, iface::SProS::DataGenerator*> dataGeneratorsById;
  scoped_destroy<std::map<std::wstring, iface::SProS::DataGenerator*> >
    dataGeneratorsByIdRAII
    (dataGeneratorsById,
     new container_destructor<std::map<std::wstring, iface::SProS::DataGenerator*> >(new pair_both_destructor<const std::wstring, iface::SProS::DataGenerator*>(new void_destructor<const std::wstring>, new objref_destructor<iface::SProS::DataGenerator>())));
  while (true)
  {
    RETURN_INTO_OBJREF(gen, iface::SProS::DataGenerator,
                       geni->nextDataGenerator());
    if (gen == NULL)
      break;

    std::wstring dgid(gen->id());
    gen->add_ref();
    dataGeneratorsById.insert(std::pair<std::wstring, iface::SProS::DataGenerator*>(dgid, gen));

    RETURN_INTO_OBJREF(vs, iface::SProS::VariableSet,
                       gen->variables());
    RETURN_INTO_OBJREF(vi, iface::SProS::VariableIterator,
                       vs->iterateVariables());

    bool didAdd = false;
    while (true)
    {
      RETURN_INTO_OBJREF(v, iface::SProS::Variable, vi->nextVariable());
      if (v == NULL)
        break;

      RETURN_INTO_OBJREF(t, iface::SProS::AbstractTask, v->taskReference());
      std::wstring idS = t->id();
      activeTasks.insert(idS);
      if (!didAdd)
      {
        didAdd = true;
        std::wstring dgidS = gen->id();
        dataGeneratorIdsByTaskId.insert(std::pair<std::wstring, std::wstring>(idS, dgidS));
      }
    }
  }

  for (std::set<std::wstring>::iterator i = activeTasks.begin();
       i != activeTasks.end(); i++)
  {
    iface::SProS::AbstractTask* at = tasksById[(*i)];

    ObjRef<iface::SProS::Task> t(QueryInterface(at));
    if (t != NULL)
      doBasicTask(modelsById, dataGeneratorIdsByTaskId, dataGeneratorsById, is,
                  t, aMonitor);
    else
    {
      ObjRef<iface::SProS::RepeatedTask> rt(QueryInterface(at));
      if (rt != NULL)
        doRepeatedTask(modelsById, dataGeneratorIdsByTaskId, dataGeneratorsById, is,
                       rt, aMonitor);
    }
  }
}

void
CDA_SRuSProcessor::doBasicTask
(
 std::map<std::wstring, iface::SRuS::TransformedModel*>& modelsById,
 std::multimap<std::wstring, std::wstring>& dataGeneratorIdsByTaskId,
 std::map<std::wstring, iface::SProS::DataGenerator*>& dataGeneratorsById,
 iface::cellml_services::CellMLIntegrationService* is,
 iface::SProS::Task* t,
 iface::SRuS::GeneratedDataMonitor* aMonitor
)
{
    RETURN_INTO_OBJREF(sm, iface::SProS::Model, t->modelReference());
    if (sm == NULL)
      throw iface::SRuS::SRuSException();

    std::wstring idS = sm->id();
    iface::SRuS::TransformedModel* tm = modelsById[idS];

    RETURN_INTO_WSTRING(taskId, t->id());

    RETURN_INTO_OBJREF(sim, iface::SProS::Simulation, t->simulationReference());
    if (sim == NULL)
      throw iface::SRuS::SRuSException();

    RETURN_INTO_WSTRING(ksid, sim->algorithmKisaoID());
    DECLARE_QUERY_INTERFACE_OBJREF(utc, sim, SProS::UniformTimeCourse);
    // To do: things other than uniform time course.
    if (utc == NULL)
      throw iface::SRuS::SRuSException();

    // To do: other language support.
    RETURN_INTO_OBJREF(mo, iface::XPCOM::IObject, tm->modelDocument());
    DECLARE_QUERY_INTERFACE_OBJREF(m, mo, cellml_api::Model);
    if (m == NULL)
      throw iface::SRuS::SRuSException();

    RETURN_INTO_OBJREF(doc, iface::dom::Document, tm->xmlDocument());

    ObjRef<iface::cellml_services::CodeInformation> ci;

    if (ksid == L"https://computation.llnl.gov/casc/sundials/documentation/ida_guide/") //IDA
    {
      RETURN_INTO_OBJREF(cmDAE, iface::cellml_services::DAESolverCompiledModel,
                         is->compileModelDAE(m));
      ci = already_AddRefd<iface::cellml_services::CodeInformation>(cmDAE->codeInformation());
    }
    else
    {
      RETURN_INTO_OBJREF(cmODE, iface::cellml_services::ODESolverCompiledModel,
                         is->compileModelODE(m));

      ci = already_AddRefd<iface::cellml_services::CodeInformation>(cmODE->codeInformation());
    }

    std::map<std::wstring, std::list<std::pair<std::wstring, int32_t> > > variableInfoIdxByDataGeneratorId;
    // For each DataGenerator...
    for (std::multimap<std::wstring, std::wstring>::iterator dgi = dataGeneratorIdsByTaskId.find(taskId);
         dgi != dataGeneratorIdsByTaskId.end() && (*dgi).first == taskId; dgi++)
    {
      std::wstring dgid = (*dgi).second;
      iface::SProS::DataGenerator* dg = dataGeneratorsById[dgid];
      RETURN_INTO_OBJREF(vs, iface::SProS::VariableSet, dg->variables());
      RETURN_INTO_OBJREF(vi, iface::SProS::VariableIterator, vs->iterateVariables());
      while (true)
      {
        RETURN_INTO_OBJREF(sv, iface::SProS::Variable, vi->nextVariable());
        if (sv == NULL)
          break;

        RETURN_INTO_OBJREF(xe, iface::xpath::XPathEvaluator, CreateXPathEvaluator());
        RETURN_INTO_OBJREF(de, iface::dom::Element, sv->domElement());
        RETURN_INTO_OBJREF(resolver, iface::xpath::XPathNSResolver,
                           xe->createNSResolver(de));
        RETURN_INTO_WSTRING(expr, sv->target());
        RETURN_INTO_OBJREF(xr, iface::xpath::XPathResult,
                           xe->evaluate(expr, doc, resolver,
                                        iface::xpath::XPathResult::FIRST_ORDERED_NODE_TYPE,
                                        NULL));
        RETURN_INTO_OBJREF(n, iface::dom::Node, xr->singleNodeValue());
        if (n == NULL)
          continue;

        RETURN_INTO_OBJREF(el, iface::cellml_api::CellMLElement, xmlToCellML(m, n));
        DECLARE_QUERY_INTERFACE_OBJREF(cv, el, cellml_api::CellMLVariable);
        if (cv == NULL)
          continue;

        RETURN_INTO_OBJREF(cti, iface::cellml_services::ComputationTargetIterator, ci->iterateTargets());
        while (true)
        {
          RETURN_INTO_OBJREF(ct, iface::cellml_services::ComputationTarget, cti->nextComputationTarget());
          if (ct == NULL)
            break;
          if (ct->degree() != 0)
            continue;
          
          RETURN_INTO_OBJREF(ctv, iface::cellml_api::CellMLVariable, ct->variable());
          if (!CDA_objcmp(ctv, cv))
          {
            int32_t idx = ct->assignedIndex();
            iface::cellml_services::VariableEvaluationType t(ct->type());
            switch (t)
            {
            case iface::cellml_services::VARIABLE_OF_INTEGRATION:
              idx = 0;
              break;
            case iface::cellml_services::CONSTANT:
              idx = -1 - idx;
              break;
            case iface::cellml_services::STATE_VARIABLE:
            case iface::cellml_services::PSEUDOSTATE_VARIABLE:
              idx++;
              break;
            case iface::cellml_services::ALGEBRAIC:
              idx += 1 + ci->rateIndexCount() * 2;
              break;
            default:
              throw iface::SRuS::SRuSException();
            }
            RETURN_INTO_WSTRING(svid, sv->id());
            
            std::map<std::wstring, std::list<std::pair<std::wstring, int32_t> > >::iterator
              vi(variableInfoIdxByDataGeneratorId.find(dgid));
            if (vi == variableInfoIdxByDataGeneratorId.end())
            {
              std::list<std::pair<std::wstring, int32_t> > l;
              l.push_back(std::pair<std::wstring, int32_t>(svid, idx));
              variableInfoIdxByDataGeneratorId.insert(
                std::pair<std::wstring, std::list<std::pair<std::wstring, int32_t> > >
                (dgid, l)
                                                     );
            }
            else
              (*vi).second.push_back(std::pair<std::wstring, uint32_t>(svid, idx));
            break;
          }
        }
      }
    }

    double it = utc->initialTime(), ost = utc->outputStartTime(), oet = utc->outputEndTime();

    uint32_t nSamples = 1;
    DECLARE_QUERY_INTERFACE_OBJREF(ssi, utc, SProS::RepeatedAnalysis);
    if (ssi != NULL)
      nSamples = ssi->numberOfSamples();

    for (uint32_t sampleNo = 0; sampleNo < nSamples; sampleNo++)
    {
      ObjRef<iface::cellml_services::CellMLIntegrationRun> cir1, cir2;

    // Now we need to look at the KISAO term...
    // XXX this is not a proper KISAO term, but a placeholder until KISAO adds one for IDA.
      if (ksid == L"https://computation.llnl.gov/casc/sundials/documentation/ida_guide/") //IDA
      {
        RETURN_INTO_OBJREF(cmDAE, iface::cellml_services::DAESolverCompiledModel,
                           is->compileModelDAE(m));
        ci = already_AddRefd<iface::cellml_services::CodeInformation>(cmDAE->codeInformation());
        cir1 = already_AddRefd<iface::cellml_services::DAESolverRun>(is->createDAEIntegrationRun(cmDAE));
        cir2 = already_AddRefd<iface::cellml_services::DAESolverRun>(is->createDAEIntegrationRun(cmDAE));
      }
      else
      {
        RETURN_INTO_OBJREF(cmODE, iface::cellml_services::ODESolverCompiledModel,
                           is->compileModelODE(m));
        iface::cellml_services::ODESolverRun* ciro1 = is->createODEIntegrationRun(cmODE);
        iface::cellml_services::ODESolverRun* ciro2 = is->createODEIntegrationRun(cmODE);
        ci = already_AddRefd<iface::cellml_services::CodeInformation>(cmODE->codeInformation());
        cir1 = already_AddRefd<iface::cellml_services::ODESolverRun>(ciro1);
        cir2 = already_AddRefd<iface::cellml_services::ODESolverRun>(ciro2);
        
        if (ksid == L"KISAO:0000019") // CVODE
        {
          ciro1->stepType(iface::cellml_services::BDF_IMPLICIT_1_5_SOLVE);
          ciro2->stepType(iface::cellml_services::BDF_IMPLICIT_1_5_SOLVE);
        }
        else if (ksid == L"KISAO:0000032")
        {
          ciro1->stepType(iface::cellml_services::RUNGE_KUTTA_4);
          ciro2->stepType(iface::cellml_services::RUNGE_KUTTA_4);
        }
        else if (ksid == L"KISAO:0000086")
        {
          ciro1->stepType(iface::cellml_services::RUNGE_KUTTA_FEHLBERG_4_5);
          ciro2->stepType(iface::cellml_services::RUNGE_KUTTA_FEHLBERG_4_5);
        }
        else if (ksid == L"KISAO:0000030")
        {
          ciro1->stepType(iface::cellml_services::GEAR_1);
          ciro2->stepType(iface::cellml_services::GEAR_1);
        }
        else // it's a term we don't handle.
          throw iface::SRuS::SRuSException();
      }

      // Run a simulation to bring the system to the start time...
      RETURN_INTO_OBJREF(cast, CDA_SRuSContinueAtStartTime,
                         new CDA_SRuSContinueAtStartTime(cir2, aMonitor, ci,
                                                         variableInfoIdxByDataGeneratorId,
                                                         dataGeneratorsById));
      cir2->setResultRange(ost, oet, 0);
      cir2->setTabulationStepControl((oet - ost) / utc->numberOfPoints(), true);
      
      cir1->setResultRange(it, ost, 1);
      cir1->setProgressObserver(cast);
      cir1->start();
    }
}

void
CDA_SRuSProcessor::doRepeatedTask
(
 std::map<std::wstring, iface::SRuS::TransformedModel*>& modelsById,
 std::multimap<std::wstring, std::wstring>& dataGeneratorIdsByTaskId,
 std::map<std::wstring, iface::SProS::DataGenerator*>& dataGeneratorsById,
 iface::cellml_services::CellMLIntegrationService* is,
 iface::SProS::RepeatedTask* t,
 iface::SRuS::GeneratedDataMonitor* aMonitor
)
{
  std::list<iface::SProS::Range*> orderedRanges;
  std::multimap<std::wstring, iface::SProS::FunctionalRange*> rangeDependencies;
  scoped_destroy<std::list<iface::SProS::Range*> > orderedRangesRAII
    (orderedRanges, new container_destructor<std::list<iface::SProS::Range*> >
     (new objref_destructor<iface::SProS::Range>()));
  scoped_destroy<std::multimap<std::wstring, iface::SProS::FunctionalRange*> >
    rangeDependenciesRAII(rangeDependencies,
                          new container_destructor<std::multimap<std::wstring, iface::SProS::FunctionalRange*> >(new pair_both_destructor<const std::wstring, iface::SProS::FunctionalRange*>(new void_destructor<const std::wstring>, new objref_destructor<iface::SProS::FunctionalRange>()))
                          );

  ObjRef<iface::SProS::RangeSet> ranges(t->ranges());
  ObjRef<iface::SProS::RangeIterator> rangeIt(ranges->iterateRanges());

  std::list<std::wstring> doneQueue;

  for (ObjRef<iface::SProS::Range> range = rangeIt->nextRange(); range;
       range = rangeIt->nextRange())
  {
    ObjRef<iface::SProS::FunctionalRange> funcRange(QueryInterface(range));
    if (funcRange == NULL)
    {
      orderedRanges.push_back(range.returnNewReference());
      doneQueue.push_back(range->id());
    }
    else
      rangeDependencies.insert(std::pair<std::wstring, iface::SProS::FunctionalRange*>
                               (funcRange->indexName(), funcRange.returnNewReference()));
  }

  while (!doneQueue.empty())
  {
    std::wstring ident(doneQueue.front());
    doneQueue.pop_front();

    std::multimap<std::wstring, iface::SProS::FunctionalRange*>::iterator it, itNext;

    for (it = rangeDependencies.find(ident);
         it != rangeDependencies.end() && it->first == ident;
         it = itNext)
    {
      itNext = it;
      itNext++;
      iface::SProS::FunctionalRange* funcRange = it->second;
      orderedRanges.push_back(funcRange);
      rangeDependencies.erase(it);
      doneQueue.push_back(funcRange->id());
    }
  }

  if (!rangeDependencies.empty())
  {
    // If this happens, there was a functionalRange with an index that wasn't a
    // valid range name.
    throw iface::SRuS::SRuSException();
  }
  
  // TODO Set up repeated task to run using new data structures.
}

already_AddRefd<iface::SRuS::SEDMLProcessor>
CDA_SRuSBootstrap::makeDefaultProcessor()
  throw()
{
  return new CDA_SRuSProcessor();
}

already_AddRefd<iface::SRuS::Bootstrap>
CreateSRuSBootstrap() throw()
{
  return new CDA_SRuSBootstrap();
}
