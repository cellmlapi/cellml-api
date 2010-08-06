package example;

import cellml_api.CellMLBootstrap;
import cellml_api.Model;

public class CellMLReader {

	/**
	 * Reading in a CellML file
	 */
	public static Model loadFromURL(CellMLBootstrap cb, String inputFileName)	{
		return cb.getModelLoader().loadFromURL(inputFileName);
	}

	/**
	 * Creating a CellML model
	 */
	public static Model createModel(CellMLBootstrap cb, String modelString){
		return cb.createModel(modelString);
	}
}
