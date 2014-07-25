package example;

import cellml_api.CellMLComponent;
import cellml_api.CellMLVariable;
import cellml_api.Connection;
import cellml_api.MapComponents;
import cellml_api.MapVariables;
import cellml_api.Model;
import cellml_api.Unit;
import cellml_api.Units;
import cellml_api.VariableInterface;

public class CreateModelTest {

    CellMLLoader cLoader;

    public CreateModelTest(){
        cLoader = new CellMLLoader();
    }

    /**
     * creating an example CellML 1.1 model with two components
     */
    private void createCellMLModel(){
        Model m = CellMLReader.createModel(cLoader.getCellMLBootstrap(),"1.1");
        m.setName("example1");

        //creating units
        Units units = m.createUnits();
        m.addElement(units);
        units.setName("second");


        Unit unit = m.createUnit();
        units.addElement(unit);
        unit.setUnits("second");


        //creating component 1
        CellMLComponent comp1 = m.createComponent();
        m.addElement(comp1);
        comp1.setName("component1");

        //adding a variable to component 1
        CellMLVariable var1 = m.createCellMLVariable();
        comp1.addElement(var1);
        var1.setName("variable1");
        var1.setUnitsElement(units);
        var1.setInitialValue("10");
        var1.setPublicInterface(VariableInterface.INTERFACE_OUT);

        //creating component 2
        CellMLComponent comp2 = m.createComponent();
        m.addElement(comp2);
        comp2.setName("component2");


        //adding a variable to component 2
        CellMLVariable var2 = m.createCellMLVariable();
        comp2.addElement(var2);
        var2.setName("variable2");
        var2.setUnitsElement(units);
        var2.setPublicInterface(VariableInterface.INTERFACE_IN);


        //connecting the two variables (Creating a connection also creates a MapComponent)
        Connection con = m.createConnection();
        m.addElement(con);

        MapComponents mapComp = con.getComponentMapping();
        mapComp.setFirstComponent(comp1);
        mapComp.setSecondComponent(comp2);

        MapVariables mapvar = m.createMapVariables();
        con.addElement(mapvar);

        mapvar.setFirstVariable(var1);
        mapvar.setSecondVariable(var2);

        CellMLWriter.writeToFile(m, m.getName()+ ".cellml");
    }

    public static void main(String [] args ){
        CreateModelTest cmtest = new CreateModelTest();
        cmtest.createCellMLModel();
    }

}
