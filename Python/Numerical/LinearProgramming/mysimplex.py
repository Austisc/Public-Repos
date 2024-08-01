"""
FIT 3155 Assignment3 Q2

Name: Austin Sing Jet Wong
ID:   32933975


"""

import numpy as np
from math import inf

def SimplexLP(numObjVet,numCons,objVet,Mat, RHS) :
    """
    Input: (numObjVet:int,numCons:int,objVet:List[int],Mat:List[List[int]],RHS: List[int]
    Output: List[int]
    
    
    Simplex method for a given equation to calculate the optimised value for a particular set of decision varaible 
    """
    #Initialize the element that are needed in the table
    deltas=np.zeros((numCons))
    coeffs=np.zeros((numCons))
    const_coeffs=np.concatenate((objVet,coeffs),axis=0) #concatenenate the coeffs accordingly 
    cj_zj=np.zeros((numCons+numObjVet))               
    ObjVpos=[-1 for i in range(numObjVet)]            # position of decision variable that represents which rows in the table for each different state 
    varIndices=[i+numObjVet for i in range(numCons)]  # the position that are currently in the table for each row
    minDeltaPos,maxPos=-1,-1
    Z_val=0
    RHSVec=np.array(RHS)
    Tablue=np.zeros((numCons,numObjVet+numCons))  # initialize the table of zero
    allZeroNeg=False                              # to know cj_zj that there is non-zero positive number
    NRHSVec=np.zeros(numCons)                     # new RHS Vec for the next iteration           
    Tablue=TableIni(numObjVet,Mat,Tablue)         #fill up the table with the correspondiig input
    NTablue=np.zeros((numCons,numObjVet+numCons)) #new table for the next iteration
    #Iterate begin---------
    iter=0
    while not(allZeroNeg):
         #if iterate more than 1: # swap RHS and Tablue each iteration and to do the operation that did not has in the first iterwtion
        if iter>0:   
            NTablue=np.zeros((numCons,numObjVet+numCons))  # a new table
            NTablue[minDeltaPos,:]=Tablue[minDeltaPos,:]/intersect # from the rows of non-zero positive minimum position, each values divide by the intersect (Tablue[minDeltaPos][maxPos])
            NRHSVec[minDeltaPos]=RHSVec[minDeltaPos]/intersect     # do the same process for RHS
            if varIndices[minDeltaPos]<numObjVet:                  # so it is remove the objVet from the rows
                ObjVpos[varIndices[minDeltaPos]]=-1                # so replaced to -1, to know that where is position for obj Var in Tablue in each iteration and no need to loop 
                                                                    # through to get the position of each obj Vet, if it is -1, that variable objective is 0



            
            varIndices[minDeltaPos]=maxPos                      # change the rows of that to that has the maximum ci_cj
            if maxPos<numObjVet:                                # to change the ObjVpos which if it is >-1, means that it got optimised decision variable on the table
                ObjVpos[maxPos]=minDeltaPos                       
            coeffs[minDeltaPos]=const_coeffs[varIndices[minDeltaPos]]    # to change the coeff for dots product
            NTablue,NRHSVec=NTablueFill(maxPos,minDeltaPos,Tablue,NTablue,RHSVec,NRHSVec) # to get the new NTablue and new NRHS Fill with the logic following simplex method
            
            Tablue=NTablue
            RHSVec=NRHSVec #exchange place for next iteration
        
        Z_val=np.dot(coeffs,NRHSVec)  # calculate the Z_Val by dot product of coeffs and RHS 
        maxPos,cj_zj,allZeroNeg=CjZjProd(maxPos,numObjVet,objVet,cj_zj,coeffs,Tablue) # to do the cj_zj product to get max value of the column,cj_zj and to know it is allZeroNeg for cj-zj 
                                                                                      # if it is all Zero Negative, the simplex will stop as it is optimised.
        minDeltaPos,deltas=CDeltaMinimum(maxPos,RHSVec,deltas,Tablue)  # to get minimum non zero positive value from the deltas and the correspond delta value
        intersect=Tablue[minDeltaPos,maxPos] # the intersection position of minDeltaPos,maxPos
        
        iter+=1
    
    ObjOptVal=[]
    for i in range(numObjVet): # so it > -1 ,it got a value >0 else the objective Variable for that instances is 0
        if ObjVpos[i]>-1:
           ObjOptVal.append(RHSVec[ObjVpos[i]])
        else:
           ObjOptVal.append(float(0))


        
    return [ObjOptVal,Z_val]# return the objective value 

   
    

def DotProduct(Coeffs,Mat,j):
    """'
    dot product to calculate the matrix with that particular column
    Output: dot product of Col(j) and coefficient for that iteration
    could be improve by knowing the position of Objective Variable since it is the only coeff that are not potentially zero, 
    which could be improve the average complexity without looping O(C) which is the number of constraint or number of equation
    """
    pro=np.dot(Coeffs,Mat[:,j])
    return pro

def TableIni(numObjVet,Mat,Tablue):
    """
    
    Initialize the table
    Output : np.array
    """
    k=numObjVet
    for i,Row in enumerate(Tablue,0): # for each rows in Tablue
        for j in range(numObjVet):    # for each 
            Row[j]=Mat[i][j]          # set Talbue[i][j] to that value 
        Row[k]=1                      # for each constraint, for that particular column in that particular Row, Row[k][k]=1 
        
        k+=1      
    
    return Tablue   
def CjZjProd(maxPos,numObjVet,objVet,cj_zj,coeffs,Tablue):

    """
    the function to find the maximum cj-zj from each column,starting from -inf,
    try to pick the earliest maximum value
    
    """
    maxi=-inf
    allZeroNeg=True
    for i in range(len(cj_zj)):
        if i>numObjVet-1:
            cj=0
        else:
                
            cj=objVet[i]
        cj_zj[i] =cj - DotProduct(coeffs,Tablue,i)
        
        if max(maxi,cj_zj[i])==cj_zj[i] and maxi!=cj_zj[i]: # pick the earliest maximum value appeared 
              
              maxi=cj_zj[i] # change the maxi value
              maxPos=i  # Position of maxPos
        
        if maxi>0:  # if it got value >0:
            allZeroNeg=False 
        
    return maxPos,cj_zj,allZeroNeg

def CDeltaMinimum(maxPos,RHSVec,deltas,Tablue):
     

     """
     the function works similar to the above one except it is for each row and to find minimum that are > 0
    
     """
     
     mini=inf
     for i, rhs in enumerate(RHSVec,0):
        if Tablue[i,maxPos]==0: # if it is divided by 0
            deltas[i]=inf     # jst put inf as it is undefined
        else:
            deltas[i]=rhs/Tablue[i,maxPos] #else, just divide with respective value of Tablue[index] rows of that column MaxPos
        if min(mini,deltas[i])==deltas[i] and deltas[i]>0:
              
              mini=deltas[i]
              minDeltaPos=i
       
     return minDeltaPos,deltas


def NTablueFill(maxPos,minDeltaPos,Tablue,NTablue,RHSVec,NRHSVec):
     """
     Fill the New Tablue and New RHS with K=J-x*I
     x=the coeffiecient values for this equation which are at the particular column 
     J=the values at the previous table of each point
     I=NTablue and NRHS at selected delta index that has been divided by the value of intersection for that particular column and row at minDeltaPos
     K= the result value of that equation at a point at NTablue and NRHS
     """
     for index,x in enumerate(Tablue[:,maxPos],0):
        if index!=minDeltaPos:
           NTablue[index,:]=Tablue[index,:]-x*NTablue[minDeltaPos,:]
           
           NRHSVec[index]=RHSVec[index]-x*NRHSVec[minDeltaPos]
    
     return NTablue,NRHSVec

import sys
# this function reads a file and return its content
def read_file(file_path: str) -> str:
    f = open(file_path, 'r')
    line = f.readlines()
    f.close()
    return line

def partition(info:list)->list:
    numDecisionVariables=int(info[1])
    numConstraint=int(info[3])
    objective=[]
    constraintsLHSMatrix=[]
    constraintsRHSVector=[]
    index=5
    DecisionVariables=info[index].split(", ")
    
    for DV in DecisionVariables:
          objective.append(int(DV))

    index=7
    
    for i in range(index,index+numConstraint):
        
        slst=[]
        ConstraitVariable=info[i].split(", ")
        
        for CV in ConstraitVariable:
            slst.append(int(CV))
        constraintsLHSMatrix.append(slst)
    index+=numConstraint+1
    
    for i in range(index,len(info)):
        constraintsRHSVector.append(int(info[i]))
    return [numDecisionVariables,numConstraint,objective,constraintsLHSMatrix,constraintsRHSVector]



def write(values):
    optimalDecisions,optimalObjective=values
    with open("lpsolution.txt" , 'w') as file:
         file.write("# optimalDecisions\n")
         valstr=""
         for i in range(len(optimalDecisions)):
             valstr+=str(optimalDecisions[i])+", "
         
         valstr=valstr[:-2]+"\n"
         file.write(valstr)
         file.write("# optimalObjective\n")
         file.write(str(optimalObjective))
         
         

if __name__ == '__main__':
#retrieve the file paths from the commandline arguments
    _, text_filename = sys.argv
    info = read_file(text_filename)
    numDecisionVariables,numConstraint,objective,constraintsLHSMatrix,constraintsRHSVector = partition(info)
    Values=SimplexLP(numDecisionVariables,numConstraint,objective,constraintsLHSMatrix,constraintsRHSVector)
    write(Values)

    
    
    



    
