package example;

import cellml_api.CellMLBootstrap;

public class CellMLLoader {
	private CellMLBootstrap bootstrap;
	
	public CellMLLoader(){
		System.loadLibrary("cellml_java_bridge");
		bootstrap =  cellml_bootstrap.CellMLBootstrap.createCellMLBootstrap();	
	}
	
	public CellMLBootstrap getCellMLBootstrap(){
		return bootstrap;
	}

}
