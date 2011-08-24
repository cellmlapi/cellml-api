package example;

import cellml_api.CellMLBootstrap;

public class CellMLLoader {
	private CellMLBootstrap bootstrap;
	
	public CellMLLoader(){
		System.loadLibrary("java_cellml");
		bootstrap =  cellml_bootstrap.CellMLBootstrap.createCellMLBootstrap();	
	}
	
	public CellMLBootstrap getCellMLBootstrap(){
		return bootstrap;
	}

}
