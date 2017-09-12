PROC=desquirr
O1=instruction
O2=dataflow
O3=node
O4=expression
O5=idapro
O6=codegen
O7=usedefine
O8=function
O9=frontend
O10=ida-arm
O11=ida-x86
O12=collateexpr
O13=collatenode
O14=controlflow
include ../plugin.mak

CFLAGS+=-I. -Inodes -I/Users/efry/Apps/Build/boost_1_64_0 -DBOOST_NO_TYPEID
ADDITIONAL_LIBS+=-L/Users/efry/Apps/Build/boost_1_64_0/stage/lib -lboost_system

# MAKEDEP dependency list ------------------
$(F)instruction$(O) : instruction.cpp 
$(F)dataflow$(O) : dataflow.cpp 
$(F)node$(O) : node.cpp 
$(F)expression$(O) : expression.cpp 
$(F)idapro$(O) : idapro.cpp 
$(F)codegen$(O) : codegen.cpp 
$(F)usedefine$(O) : usedefine.cpp 
$(F)function$(O) : function.cpp 
$(F)frontend$(O) : frontend.cpp 
$(F)ida-arm$(O) : ida-arm.cpp 
$(F)ida-x86$(O) : ida-x86.cpp 
$(F)collateexpr$(O) : collateexpr.cpp 
$(F)collatenode$(O) : collatenode.cpp 
$(F)desquirr$(O) : desquirr.cpp 
$(F)controlflow$(O) : controlflow.cpp

