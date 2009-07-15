package example;

import pjm.XPCOMDerived;
import cellml_api.Model;
import cellml_api.RDFRepresentation;
import cellml_api.RDFXMLDOMRepresentation;
import cellml_api.RDFXMLStringRepresentation;


public class RDFManipulateTest {
	CellMLLoader cLoader;
	public RDFManipulateTest(){
		cLoader = new CellMLLoader();
	}
	
	private void readRDF(){
		//load the Hodgkin Huxley model from the repository
		Model model = CellMLReader.loadFromURL(cLoader.getCellMLBootstrap(), "http://www.cellml.org/models/hodgkin_huxley_1952_version07/download");

		RDFRepresentation rdfStringRep = model.getRDFRepresentation("http://www.cellml.org/RDFXML/string");
		pjm2pcm.cellml_api.RDFXMLStringRepresentation rep =  pjm2pcm.cellml_api.RDFXMLStringRepresentation.queryInterface((XPCOMDerived)rdfStringRep);
		System.out.print(rep.getSerialisedData());
		
//		RDFRepresentation rdfDomRep = model.getRDFRepresentation("http://www.cellml.org/RDFXML/DOM");
//		pjm2pcm.cellml_api.RDFXMLDOMRepresentation domRep =  pjm2pcm.cellml_api.RDFXMLDOMRepresentation.queryInterface((XPCOMDerived)rdfDomRep);
//		System.out.print(domRep.getData());		

	}
	
	public static void main(String [] args ){
		RDFManipulateTest rdfMantest = new RDFManipulateTest();
		rdfMantest.readRDF();
	}

}
