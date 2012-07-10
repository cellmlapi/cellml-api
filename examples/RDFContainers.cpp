#include <cellml-api-cxx-support.hpp>
#include <CellMLBootstrap.hpp>
#include <iostream>
#include <IfaceCellML_APISPEC.hxx>
#include <IfaceRDF_APISPEC.hxx>

static void displayNode(iface::rdf_api::Node* aNode)
{
  ObjRef<iface::rdf_api::Literal> l(QueryInterface(aNode));
  if (l)
  {
    std::wcout << L"\"" << l->lexicalForm() << L"\"";
    return;
  }
  
  ObjRef<iface::rdf_api::URIReference> r(QueryInterface(aNode));
  if (r)
  {
    std::wcout << r->URI();
    return;
  }
  
  ObjRef<iface::rdf_api::BlankNode> n(QueryInterface(aNode));
  std::wcout << L"BlankNode(" << reinterpret_cast<long long>(static_cast<iface::rdf_api::BlankNode*>(n)) << L")" << std::endl;
}

#ifdef DUMP_ALL_TRIPLES_FIRST
static void displayTriple(iface::rdf_api::Triple* aTriple)
{
  std::wcout << L"Triple: Subject=";
  ObjRef<iface::rdf_api::Resource> r(aTriple->subject());
  displayNode(r);
  std::wcout << L" Predicate=";
  r = aTriple->predicate();
  displayNode(r);
  std::wcout << L" Object=";
  ObjRef<iface::rdf_api::Node> n(aTriple->object());
  displayNode(n);
  std::wcout << std::endl;
}
#endif

void displayContainer(iface::rdf_api::Container* aContainer, const std::wstring& aPrefix)
{
  try
  {
    ObjRef<iface::rdf_api::Resource> ctype(aContainer->containerType());
    std::wcout << aPrefix << "Container type = ";
    displayNode(ctype);
    std::wcout << std::endl;
  }
  catch (...) {}

  ObjRef<iface::rdf_api::NodeIterator> it(aContainer->iterateChildren());
  while (true)
  {
    ObjRef<iface::rdf_api::Node> n(it->getNextNode());
    if (n == NULL) break;
    std::wcout << aPrefix << "* ";
    displayNode(n);
    std::wcout << std::endl;
  }
}

int main(int argc, char** argv)
{
  try
  {
    std::wstring modelURL
      (L"http://models.cellml.org/workspace/hodgkin_huxley_1952/@@rawfile/949cd4d3/hodgkin_huxley_1952.cellml");

    // Create a CellML Bootstrap and a model loader.
    ObjRef<iface::cellml_api::CellMLBootstrap> bootstrap(CreateCellMLBootstrap());
    ObjRef<iface::cellml_api::DOMModelLoader> loader(bootstrap->modelLoader());
    
    // Load a CellML model.
    ObjRef<iface::cellml_api::Model> hhModel
      (loader->loadFromURL(modelURL));
    
    // Request a RDF/API capable RDF representation...
    ObjRef<iface::cellml_api::RDFRepresentation> rr(hhModel->getRDFRepresentation(L"http://www.cellml.org/RDF/API"));
    ObjRef<iface::rdf_api::RDFAPIRepresentation> rrHH(QueryInterface(rr));

    // Retrieve the DataSource. Note: Changes to the data source are not pushed back
    // to the model until the source attribute is set on the rrHH again.
    ObjRef<iface::rdf_api::DataSource> dsHH(rrHH->source());

    // Get the resource corresponding to the cmeta:id on the model. Note that
    // cmetaId can return the empty string, which you should check for in
    // production code, if there is no cmeta:id
    ObjRef<iface::rdf_api::URIReference> hhModelRef
      (dsHH->getURIReference(bootstrap->makeURLAbsolute(modelURL, L"#" + hhModel->cmetaId())));

#ifdef DUMP_ALL_TRIPLES_FIRST
    {
      ObjRef<iface::rdf_api::TripleSet> ts(dsHH->getAllTriples());
      ObjRef<iface::rdf_api::TripleEnumerator> te(ts->enumerateTriples());
      while (true)
      {
        ObjRef<iface::rdf_api::Triple> t(te->getNextTriple());
        if (t == NULL)
          break;

        displayTriple(t);
      }
    }
#endif

    ObjRef<iface::rdf_api::URIReference> pmid(dsHH->getURIReference(L"http://www.cellml.org/bqs/1.0#Pubmed_id"));
    // Note: Calls like this could fail with an exception if there was no pubmed ID - production code should check.
    ObjRef<iface::rdf_api::Triple> t(hhModelRef->getTripleOutOfByPredicate(pmid));
    ObjRef<iface::rdf_api::Node> n(t->object());
    ObjRef<iface::rdf_api::Literal> lPMID(QueryInterface(n));
    if (lPMID != NULL) // It will be null if for some reason it isn't a literal...
      std::wcout << L"Model pubmed ID: " << lPMID->lexicalForm() << std::endl;
    
    ObjRef<iface::rdf_api::URIReference> bqsReference(dsHH->getURIReference(L"http://www.cellml.org/bqs/1.0#reference"));
    ObjRef<iface::rdf_api::TripleSet> ts(hhModelRef->getTriplesOutOfByPredicate(bqsReference));
    ObjRef<iface::rdf_api::TripleEnumerator> te(ts->enumerateTriples());
    while (true)
    {
      t = te->getNextTriple();
      if (t == NULL) break;
      ObjRef<iface::rdf_api::Resource> r(QueryInterface(t->object()));
      if (r == NULL) continue;
      std::wcout << L"Found a resource description:" << std::endl;
      try
      {
        t = r->getTripleOutOfByPredicate(pmid);
        n = t->object();
        lPMID = QueryInterface(n);
        if (lPMID != NULL)
          std::wcout << L"  Reference pubmed ID: " << lPMID->lexicalForm() << std::endl;
      }
      catch(...) {}
      // Look up the subject...
      ObjRef<iface::rdf_api::URIReference> subject(dsHH->getURIReference(L"http://purl.org/dc/elements/1.1/subject"));
      ts = r->getTriplesOutOfByPredicate(subject);
      ObjRef<iface::rdf_api::TripleEnumerator> te2 = ts->enumerateTriples();
      while (true)
      {
        t = te2->getNextTriple();
        if (t == NULL)
          break;
        n = t->object();
        r = QueryInterface(n);
        if (r == NULL)
          continue;
        std::wcout << "  Subject:" << std::endl;
        ObjRef<iface::rdf_api::URIReference> subjectType(dsHH->getURIReference(L"http://www.cellml.org/bqs/1.0#subject_type"));
        ts = r->getTriplesOutOfByPredicate(subjectType);
        ObjRef<iface::rdf_api::TripleEnumerator> te3 = ts->enumerateTriples();
        t = te3->getNextTriple();
        if (t)
        {
          n = t->object();
          ObjRef<iface::rdf_api::Literal> l(QueryInterface(n));
          std::wcout << "    Subject Type=" << l->lexicalForm() << std::endl;
        }
        ObjRef<iface::rdf_api::URIReference> value(dsHH->getURIReference(L"http://www.w3.org/1999/02/22-rdf-syntax-ns#value"));
        ts = r->getTriplesOutOfByPredicate(value);
        te3 = ts->enumerateTriples();

        t = te3->getNextTriple();
        if (t == NULL) continue;
        n = t->object();
        r = QueryInterface(n);
        if (r == NULL) continue;
        ObjRef<iface::rdf_api::Container> cont(r->correspondingContainer());

        // Add a new entry to the container first...
        ObjRef<iface::rdf_api::PlainLiteral> newPL(dsHH->getPlainLiteral(L"Newly created label", L"en"));
        cont->appendChild(newPL);

        // Delete any labels that match 'giant axon'...
        ObjRef<iface::rdf_api::NodeIterator> ni(cont->iterateChildren());
        while (true)
        {
          ObjRef<iface::rdf_api::Node> n(ni->getNextNode());
          if (n == NULL)
            break;
          ObjRef<iface::rdf_api::Literal> l(QueryInterface(n));
          if (l == NULL)
            continue;
          if (l->lexicalForm() == L"giant axon")
            // The false means don't renumber immediately. This leaves the
            // container with gaps...
            cont->removeChild(l, false);
        }
        cont->renumberContainer(); // Remove the gaps.

        displayContainer(cont, L"    ");
      }      
    }
  }
  catch (iface::cellml_api::CellMLException&)
  {
    std::wcout << L"A CellMLException was raised. In production code, you would normally have more "
               << L"exception handlers to work out exactly where the problem occurred. This program "
               << L"avoids that for simplicity, to make the normal control flow easier to understand. "
               << L"The most likely cause is a network problem retrieving the hardcoded model URL."
               << std::endl;
    return 1;
  }
  catch (iface::rdf_api::RDFProcessingError&)
  {
    std::wcout << L"An RDFProcessingError was raised. In production code, you would normally have more "
               << L"exception handlers to work out exactly where the problem occurred. This program "
               << L"avoids that for simplicity, to make the normal control flow easier to understand."
               << std::endl;
    return 2;
  }
  return 0;
}
