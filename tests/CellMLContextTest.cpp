#include "cda_compiler_support.h"
#include "CellMLContextTest.hpp"
#include "IfaceCellML_Context.hxx"
#include "CellMLContextBootstrap.hxx"
#include "Utilities.hxx"

CPPUNIT_TEST_SUITE_REGISTRATION( CellMLContextTest );

#ifndef BASE_DIRECTORY
#ifdef WIN32
#define BASE_DIRECTORY L"file:///" TESTDIR L"/test_xml/"
#else
#define BASE_DIRECTORY L"file://" TESTDIR L"/test_xml/"
#endif
#endif

void
CellMLContextTest::setUp()
{
  mContext = CreateCellMLContext();
  RETURN_INTO_OBJREF(cb, iface::cellml_api::CellMLBootstrap,
                     mContext->cellmlBootstrap());
  mModelLoader = cb->modelLoader();
}

void
CellMLContextTest::tearDown()
{
  mContext->release_ref();
  mModelLoader->release_ref();
  if (mAchCascade != NULL)
    mAchCascade->release_ref();
  if (mBeelerReuter != NULL)
    mBeelerReuter->release_ref();
}

void
CellMLContextTest::loadAchCascade()
{
  mAchCascade = mModelLoader->loadFromURL
    (BASE_DIRECTORY L"Ach_cascade_1995.xml");
}

void
CellMLContextTest::loadBeelerReuter()
{
  mBeelerReuter = mModelLoader->loadFromURL
    (BASE_DIRECTORY L"beeler_reuter_model_1977.xml");
}

void
CellMLContextTest::testCellMLContext()
{
  //    /**
  //     * The module manager for this CellML context...
  //     */
  //    readonly attribute CellMLModuleManager moduleManager;
  RETURN_INTO_OBJREF(mm, iface::cellml_context::CellMLModuleManager,
                     mContext->moduleManager());
  CPPUNIT_ASSERT(mm);

  //
  //    /**
  //     * The type annotation manager for this CellML context...
  //     */
  //    readonly attribute CellMLTypeAnnotationManager typeAnnotationManager;
  RETURN_INTO_OBJREF(tam, iface::cellml_context::CellMLTypeAnnotationManager,
                     mContext->typeAnnotationManager());
  CPPUNIT_ASSERT(tam);

  //    
  //    /**
  //     * The model loader...
  //     */
  //    readonly attribute cellml_api::CellMLBootstrap cellmlBootstrap;
  RETURN_INTO_OBJREF(cb, iface::cellml_api::CellMLBootstrap,
                     mContext->cellmlBootstrap());
  CPPUNIT_ASSERT(cb);

  //
  //    /**
  //     * The list of all loaded models and their derivatives.
  //     */
  //    readonly attribute ModelList loadedModels;
  RETURN_INTO_OBJREF(ml, iface::cellml_context::ModelList,
                     mContext->loadedModels());
  CPPUNIT_ASSERT(ml);
}

class TestModelMonitor
  : public iface::cellml_context::ModelNodeMonitor,
    public iface::cellml_context::ModelListMonitor
{
public:
  void add_ref() throw()
  {
  }

  void release_ref() throw()
  {
  }

  void* query_interface(const char* id) throw()
  {
    if (!strcmp(id, "cellml_context::ModelNodeMonitor"))
      return reinterpret_cast<void*>(static_cast<iface::cellml_context::ModelNodeMonitor*>(this));
    if (!strcmp(id, "cellml_context::ModelListMonitor"))
      return reinterpret_cast<void*>(static_cast<iface::cellml_context::ModelListMonitor*>(this));
    return NULL;
  }

  char* objid() throw() { return strdup("TheTestModelMonitor"); }

  void
  modelRenamed(iface::cellml_context::ModelNode* renamedNode,
               const wchar_t* newName) throw()
  {
    countRenames++;
  }

  void
  modelReplaced(iface::cellml_context::ModelNode* changedNode,
                iface::cellml_api::Model* newModel) throw()
  {
    countReplaces++;
  }


  void
  changesFlushed(iface::cellml_context::ModelNode* changedNode)
    throw()
  {
    countFlushes++;
  }

  void
  ownerChanged(iface::cellml_context::ModelNode* changedNode,
               iface::XPCOM::IObject* newOwner)
    throw()
  {
    countChangeOwner++;
  }

  void
  modelFrozenStateChanged(iface::cellml_context::ModelNode* changedNode,
                          bool newState)
    throw()
  {
    if (newState)
      countFreeze++;
    else
      countUnfreeze++;
  }

  void
  modelAdded(iface::cellml_context::ModelNode* newNode, short depth)
    throw()
  {
    countAdds++;
  }

  void
  modelRemoved(iface::cellml_context::ModelNode* oldNode, short depth)
    throw()
  {
    countRemoves++;
  }

  void
  resetCounts()
  {
    countAdds = countRemoves = countRenames = countReplaces =
      countFlushes = countChangeOwner = countFreeze = countUnfreeze
      = 0;
  }

  int countAdds, countRemoves, countRenames, countReplaces, countFlushes, countChangeOwner,
    countFreeze, countUnfreeze;
};
TestModelMonitor gTMM;

void
CellMLContextTest::testModelList()
{
  RETURN_INTO_OBJREF(ml, iface::cellml_context::ModelList,
                     mContext->loadedModels());
  CPPUNIT_ASSERT(ml);
  loadBeelerReuter();

  RETURN_INTO_OBJREF(mlBr, iface::cellml_context::ModelNode,
                     ml->makeNode(mBeelerReuter));

  //    /**
  //     * Adds a model monitor. This will receive notifications for all models
  //     * in the list, and all more derived models.
  //     * @param monitor The model monitor to add.
  //     */
  //    void addModelMonitor(in ModelNodeMonitor monitor);
  gTMM.resetCounts();
  ml->addModelMonitor(&gTMM);
  ml->addModel(mlBr);
  mlBr->name(L"hello");
  CPPUNIT_ASSERT_EQUAL(1, gTMM.countRenames);
  gTMM.resetCounts();
  mlBr->isFrozen(true);
  CPPUNIT_ASSERT_EQUAL(1, gTMM.countFreeze);
  CPPUNIT_ASSERT_EQUAL(0, gTMM.countUnfreeze);
  gTMM.resetCounts();
  mlBr->isFrozen(false);
  CPPUNIT_ASSERT_EQUAL(1, gTMM.countUnfreeze);
  CPPUNIT_ASSERT_EQUAL(0, gTMM.countFreeze);
  gTMM.resetCounts();

  loadAchCascade();
  mlBr->model(mAchCascade);
  CPPUNIT_ASSERT_EQUAL(1, gTMM.countReplaces);
  gTMM.resetCounts();
  mlBr->flushChanges();
  CPPUNIT_ASSERT_EQUAL(1, gTMM.countFlushes);
  gTMM.resetCounts();
  mlBr->owner(&gTMM);
  CPPUNIT_ASSERT_EQUAL(1, gTMM.countChangeOwner);

  //
  //    /**
  //     * Removes a model monitor.
  //     * @param monitor The model monitor to remove.
  //     */
  //    void removeModelMonitor(in ModelNodeMonitor monitor);
  gTMM.resetCounts();
  ml->removeModelMonitor(&gTMM);
  mlBr->name(L"world");
  CPPUNIT_ASSERT_EQUAL(0, gTMM.countRenames);
  
  //
  //    /**
  //     * Adds a model list monitor. This will receive notifications of changes to
  //     * the model list, and of changes to more derived model lists.
  //     * @param monitor The model list monitor to add.
  //     */
  //    void addListMonitor(in ModelListMonitor monitor);
  gTMM.resetCounts();
  ml->addListMonitor(&gTMM);

  RETURN_INTO_OBJREF(mlAch, iface::cellml_context::ModelNode,
                     ml->makeNode(mAchCascade));
  ml->addModel(mlAch);
  CPPUNIT_ASSERT_EQUAL(1, gTMM.countAdds);
  gTMM.resetCounts();
  ml->removeModel(mlAch);
  CPPUNIT_ASSERT_EQUAL(1, gTMM.countRemoves);

  //
  //    /**
  //     * Removes a model list monitor.
  //     * @param monitor The model list monitor to remove.
  //     */
  //    void removeListMonitor(in ModelListMonitor monitor);
  gTMM.resetCounts();
  ml->removeListMonitor(&gTMM);
  ml->addModel(mlAch);
  CPPUNIT_ASSERT_EQUAL(0, gTMM.countAdds);

  //
  //    /**
  //     * Makes a node suitable for passing to addModel. It does not actually add
  //     * the node to the list.
  //     * @param mod The model the node refers to.
  //     * @returns The new node.
  //     */
  //    ModelNode makeNode(in cellml_api::Model mod);
  // Tested above already.

  //
  //    /**
  //     * Adds a model to the list...
  //     * @param node The model node to add to the list...
  //     * Note: node must be created in the same context as the ModelList. You
  //     *   cannot define your own nodes, or mix across contexts. However, the
  //     *   model itself can be created any way and in any context you like.
  //     */
  //    void addModel(in ModelNode node);
  // Tested above already.

  //
  //    /**
  //     * Removes a model from the list...
  //     * @param node The model node to remove from the list...
  //     */
  //    void removeModel(in ModelNode node);
  // Test above already...
  //
  //    /**
  //     * Iterates over all model nodes in the list.
  //     * @return an iterate to do the iteration.
  //     */
  //    ModelNodeIterator iterateModelNodes();
  RETURN_INTO_OBJREF(mli, iface::cellml_context::ModelNodeIterator, ml->iterateModelNodes());
  RETURN_INTO_OBJREF(mn1, iface::cellml_context::ModelNode, mli->nextModelNode());
  CPPUNIT_ASSERT(mn1);
  CPPUNIT_ASSERT(!CDA_objcmp(mn1, mlAch) || !CDA_objcmp(mn1, mlBr));
  RETURN_INTO_OBJREF(mn2, iface::cellml_context::ModelNode, mli->nextModelNode());
  CPPUNIT_ASSERT(mn2);
  // 1 & 2 must be different objects..
  CPPUNIT_ASSERT(CDA_objcmp(mn1, mn2));
  CPPUNIT_ASSERT(!CDA_objcmp(mn2, mlAch) || !CDA_objcmp(mn2, mlBr));
  RETURN_INTO_OBJREF(mn3, iface::cellml_context::ModelNode, mli->nextModelNode());
  CPPUNIT_ASSERT(!mn3);

  //
  //    /**
  //     * Fetches the model node under which this list belongs, or null if
  //     * this list has no parent.
  //     */
  //    readonly attribute ModelNode parentNode;
  CPPUNIT_ASSERT(ml->parentNode() == NULL);
  RETURN_INTO_OBJREF(mlBrD, iface::cellml_context::ModelList, mlBr->derivedModels());
  RETURN_INTO_OBJREF(mlBrDN, iface::cellml_context::ModelNode, mlBrD->parentNode());
  CPPUNIT_ASSERT(!CDA_objcmp(mlBr, mlBrDN));
}

void
CellMLContextTest::testModelNode()
{
  RETURN_INTO_OBJREF(ml, iface::cellml_context::ModelList,
                     mContext->loadedModels());
  CPPUNIT_ASSERT(ml);

  loadBeelerReuter();
  RETURN_INTO_OBJREF(mlBr, iface::cellml_context::ModelNode,
                     ml->makeNode(mBeelerReuter));
  ml->addModel(mlBr);

  mlBr->addModelMonitor(&gTMM);

  //    /**
  //     * The name of the model, according to the user.
  //     */
  //    attribute wstring name;
  gTMM.resetCounts();
  mlBr->name(L"helloWorld");
  CPPUNIT_ASSERT_EQUAL(1, gTMM.countRenames);
  RETURN_INTO_WSTRING(mlBrName, mlBr->name());
  CPPUNIT_ASSERT(mlBrName == L"helloWorld");

  //
  //    /**
  //     * Returns the node in the derivative tree which is the latest
  //     * (i.e. greatest modificationTimestamp). This can return itself if it is
  //     * the latest model(this will only happen under the normal usage convention
  //     * if there are no derivatives).
  //     */
  //    ModelNode getLatestDerivative();
  RETURN_INTO_OBJREF(mlBrLD1, iface::cellml_context::ModelNode, mlBr->getLatestDerivative());
  CPPUNIT_ASSERT(!CDA_objcmp(mlBrLD1, mlBr));
  // Make a later derivative...
  RETURN_INTO_OBJREF(mlBrD, iface::cellml_context::ModelList, mlBr->derivedModels());
  loadAchCascade();
  sleep(1); // Essential to ensure timestamps are different...
  RETURN_INTO_OBJREF(mlAch, iface::cellml_context::ModelNode, mlBrD->makeNode(mAchCascade));
  mlBrD->addModel(mlAch);
  RETURN_INTO_OBJREF(mlBrLD2, iface::cellml_context::ModelNode, mlBr->getLatestDerivative());
  CPPUNIT_ASSERT(!CDA_objcmp(mlBrLD2, mlAch));

  //
  //    /**
  //     * Gets a model node which is writable. If this node is not frozen, will
  //     * return itself. Otherwise, the model is cloned, and a new node pointing
  //     * to the cloned is inserted into this node, and the new node returned.
  //     */
  //    ModelNode getWritable();
  RETURN_INTO_OBJREF(mlgw1, iface::cellml_context::ModelNode, mlBr->getWritable());
  CPPUNIT_ASSERT(!CDA_objcmp(mlgw1, mlBr));
  gTMM.resetCounts();
  mlBr->isFrozen(true);
  CPPUNIT_ASSERT_EQUAL(1, gTMM.countFreeze);
  CPPUNIT_ASSERT_EQUAL(true, mlBr->isFrozen());
  RETURN_INTO_OBJREF(mlgw2, iface::cellml_context::ModelNode, mlBr->getWritable());
  CPPUNIT_ASSERT_EQUAL(false, mlgw2->isFrozen());
  CPPUNIT_ASSERT(CDA_objcmp(mlgw2, mlBr));

  //
  //    /**
  //     * If the model is frozen, the user should be prevented from making changes
  //     * to it, because it will not make semantic sense. Individual tools need
  //     * to enforce the frozen model restrictions themselves.
  //     */
  //    attribute boolean isFrozen;
  gTMM.resetCounts();
  CPPUNIT_ASSERT_EQUAL(true, mlBr->isFrozen());
  mlBr->isFrozen(false);
  CPPUNIT_ASSERT_EQUAL(false, mlBr->isFrozen());
  CPPUNIT_ASSERT_EQUAL(1, gTMM.countUnfreeze);

  //
  //    /**
  //     * A timestamp representing when stampModifiedNow was last called.
  //     * Equal to 0 if stampModifiedNow() never called.
  //     */
  //    readonly attribute unsigned long modificationTimestamp;
  //  Tested below.
  //
  //    /**
  //     * Sets the last modified stamp to now.
  //     */
  //    void stampModifiedNow();
  mlBr->stampModifiedNow();
  CPPUNIT_ASSERT(0 != mlBr->modificationTimestamp());

  //
  //    /**
  //     * The model this node points to...
  //     */
  //    attribute cellml_api::Model model;
  RETURN_INTO_OBJREF(mlBrM, iface::cellml_api::Model, mlBr->model());
  CPPUNIT_ASSERT(!CDA_objcmp(mlBrM, mBeelerReuter));

  //
  //    /**
  //     * Requests that any tools / controls which have changes that they have not
  //     * saved cached somewhere flush their changes back to the model. Any tool
  //     * or control except for the node owner should always call this before
  //     * making changes to the model, because otherwise there might be conflicting
  //     * changes made by different tools.
  //     */
  //    void flushChanges();
  gTMM.resetCounts();
  mlBr->flushChanges();
  CPPUNIT_ASSERT_EQUAL(1, gTMM.countFlushes);

  //
  //    /**
  //     * Describes the 'owner' tool / control of the current model object. The
  //     * owner is allowed to cache changes to the model node and only flush them
  //     * later when required (although it is of course preferrable that where
  //     * possible changes are made immediately so views will not be stale).
  //     * Owners should always monitor the node and look for ownerChanged
  //     * notifications so they can flush any changes and stop acting as the owner
  //     * at the appropriate point in time.
  //     *
  //     * May be set to null to clear the owner. There is no strong reference to
  //     * owners held, so owners must clear their ownership by setting this to null
  //     * before being destroyed.
  //     */
  //    attribute XPCOM::IObject owner;
  CPPUNIT_ASSERT(mlBr->owner() == NULL);
  gTMM.resetCounts();
  mlBr->owner(mlBr);
  RETURN_INTO_OBJREF(mlBrO, iface::XPCOM::IObject, mlBr->owner());
  CPPUNIT_ASSERT(!CDA_objcmp(mlBrO, mlBr));
  mlBr->owner(NULL);
  CPPUNIT_ASSERT_EQUAL(2, gTMM.countChangeOwner);

  //
  //    /**
  //     * The list of all derived models...
  //     */
  //    readonly attribute ModelList derivedModels;
  RETURN_INTO_OBJREF(dm, iface::cellml_context::ModelList, mlBr->derivedModels());
  RETURN_INTO_OBJREF(mni, iface::cellml_context::ModelNodeIterator,
                     dm->iterateModelNodes());
  RETURN_INTO_OBJREF(nmn, iface::cellml_context::ModelNode,
                     mni->nextModelNode());
  CPPUNIT_ASSERT(!CDA_objcmp(nmn, mlAch));

  //
  //    /**
  //     * Adds a model monitor. This will receive notifications for all models
  //     * in the list, and all more derived models.
  //     * @param monitor The model monitor to add.
  //     */
  //    void addModelMonitor(in ModelNodeMonitor monitor);
  // Tested above.

  //
  //    /**
  //     * Removes a model monitor.
  //     * @param monitor The model node monitor to remove.
  //     */
  //    void removeModelMonitor(in ModelNodeMonitor monitor);
  mlBr->removeModelMonitor(&gTMM);
  gTMM.resetCounts();
  mlBr->name(L"worldHello");
  CPPUNIT_ASSERT_EQUAL(0, gTMM.countRenames);

  //
  //    /**
  //     * Fetches the model list this node belongs to, or null if this node has no
  //     * parent.
  //     */
  //    readonly attribute ModelList parentList;
  RETURN_INTO_OBJREF(mlBrPL, iface::cellml_context::ModelList, mlBr->parentList());
  CPPUNIT_ASSERT(!CDA_objcmp(mlBrPL, ml));

  //
  //    /**
  //     * Determines if the model is 'dirty'. Models are initially not dirty, but
  //     * if any changes are made to the model, they become dirty. The dirty flag
  //     * can also be manually set or cleared. This flag is used to indicate if
  //     * the model has unsaved changes.
  //     */
  //    attribute boolean dirty;
  mlBr->dirty(true);
  CPPUNIT_ASSERT_EQUAL(true, mlBr->dirty());
  mlBr->dirty(false);
  CPPUNIT_ASSERT_EQUAL(false, mlBr->dirty());
  mBeelerReuter->name(L"mytest");
  CPPUNIT_ASSERT_EQUAL(true, mlBr->dirty());
}
