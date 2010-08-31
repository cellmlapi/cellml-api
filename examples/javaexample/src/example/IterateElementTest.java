package example;

import cellml_api.CellMLComponent;
import cellml_api.CellMLComponentIterator;
import cellml_api.CellMLComponentSet;
import cellml_api.CellMLVariable;
import cellml_api.CellMLVariableIterator;
import cellml_api.CellMLVariableSet;
import cellml_api.Connection;
import cellml_api.ConnectionIterator;
import cellml_api.ConnectionSet;
import cellml_api.MapComponents;
import cellml_api.MapVariables;
import cellml_api.MapVariablesIterator;
import cellml_api.MapVariablesSet;
import cellml_api.Model;

public class IterateElementTest {
	
	CellMLLoader cLoader;
	
	public IterateElementTest(){
		cLoader = new CellMLLoader();
	}
	
	/**
	 * Iterating elements of a model
	 */
	private void iterateModelElements(){   	
		//load the Hodgkin Huxley model from the repository
		Model model = CellMLReader.loadFromURL(cLoader.getCellMLBootstrap(), "http://www.cellml.org/models/hodgkin_huxley_1952_version07/download");

		System.out.println("Model Name:" + model.getName() + "\n");

		//Iterating components and their variables
		CellMLComponentSet componentSet = model.getModelComponents();
		CellMLComponentIterator iter = componentSet.iterateComponents();

		for(int i = 0; i < componentSet.getLength(); i++){
			CellMLComponent comp = iter.nextComponent();
			System.out.println("Component Name:"+comp.getName());

			CellMLVariableSet variableSet = comp.getVariables();
			CellMLVariableIterator varIter = variableSet.iterateVariables();

			for(int j = 0; j < variableSet.getLength(); j ++){
				CellMLVariable variable = varIter.nextVariable();
				System.out.println("Variable Name:"+variable.getName());
			}
			System.out.println();
		}

		//Iterating connections
		ConnectionSet connectionSet = model.getConnections();
		ConnectionIterator conIter = connectionSet.iterateConnections();

		for(int i = 0; i < connectionSet.getLength(); i++){
			Connection connection = conIter.nextConnection();
			MapComponents mapComponent = connection.getComponentMapping();
			System.out.println("Connection Component1: "+mapComponent.getFirstComponentName() + "    Component2: " + mapComponent.getSecondComponentName());

			MapVariablesSet variableSet = connection.getVariableMappings();
			MapVariablesIterator varIter = variableSet.iterateMapVariables();

			for(int j =0; j < variableSet.getLength(); j++){
				MapVariables mapVar = varIter.nextMapVariable();
				System.out.println("\t Variable1: "+mapVar.getFirstVariableName() + "    Variable2: " + mapVar.getSecondVariableName());
			}
			System.out.println();
		}

	}
	
	public static void main(String [] args ){
		IterateElementTest ietest = new IterateElementTest();
		ietest.iterateModelElements();
	}
}
