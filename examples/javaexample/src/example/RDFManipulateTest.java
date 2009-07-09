package example;

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

		RDFRepresentation rep = model.getRDFRepresentation("http://www.cellml.org/RDFXML/string");
		
//		RDFXMLStringRepresentation stringRep = (RDFXMLStringRepresentation)rep;
		RDFXMLDOMRepresentation stringRep = (RDFXMLDOMRepresentation)rep;

	}
	
	public static void main(String [] args ){
		RDFManipulateTest rdfMantest = new RDFManipulateTest();
		rdfMantest.readRDF();
	}

}
