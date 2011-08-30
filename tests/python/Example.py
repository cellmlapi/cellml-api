#!/usr/bin/env python

from cellml_api import CellML_APISPEC
from cellml_api import VACSS
from sys import exit

canQuit = False

m = CellML_APISPEC.CellMLBootstrap().createModel("1.1")
if m == None:
    exit(-1)

m.setname("example1")
if m.getname() != "example1":
    exit(-1)

units = m.createUnits()
m.addElement(units)
units.setname("second")

unit = m.createUnit()
units.addElement(unit)
unit.setunits("second")

comp1 = m.createComponent()
m.addElement(comp1)
comp1.setname("component1")

var1 = m.createCellMLVariable()
comp1.addElement(var1)
var1.setname("variable1")
var1.setunitsElement(units)
var1.setinitialValue("10")
var1.setpublicInterface(CellML_APISPEC.VariableInterface["INTERFACE_OUT"])

comp2 = m.createComponent()
m.addElement(comp2)
comp2.setname("component2")

var2 = m.createCellMLVariable()
comp2.addElement(var2)
var2.setname("variable2")
var2.setunitsElement(units)
var2.setpublicInterface(CellML_APISPEC.VariableInterface["INTERFACE_IN"])

con = m.createConnection()
m.addElement(con)

mapComp = con.getcomponentMapping()
mapComp.setfirstComponent(comp1)
mapComp.setsecondComponent(comp2)

mapvar = m.createMapVariables()
con.addElement(mapvar)

mapvar.setfirstVariable(var1)
mapvar.setsecondVariable(var2)

vc = VACSS.VACSService()
col = []
vc.getPositionInXML(CellML_APISPEC.CellMLDOMElement(comp1).getdomElement(), 0, col)
if col[0] != 56:
    print "Read column " + col[0] + ", expected 56 - possible problem with out params"
    exit(-1)

print "Success"
exit(0)
