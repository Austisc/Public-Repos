"""
FIT 3155 Assignment2 Q2 Encode

Name: Austin Sing Jet Wong
ID:   32933975


"""



def NaiveBWT(txt):
    """
    NaiveBWT:func
    input->string text
    Output:string
    n=len(txt)
    Time Complexity : O(n^2) 
    Space Complexity : O(n^2)
    

    Naive method to generate BWT Matrix
    1. building a cyclic permutation of the txt 
    2. use radii sort to sort the BWT matrix
    3. generate the BWT string, which is the last character of each sorted cyclic permutation
    Reference: FIT 3155 week 3 Burrow-Wheeler Transform

    """
    


  
    d=txt
    d+='$'
    cyc=["" for i in range(len(d))]
    for i in range(len(d)): 
        cyc[i]+= d[len(d)-i-1:]+d[:len(d)-i-1]
    
    i=len(txt)-1
    while i>=0:
        arr=[]
        radii=[[] for i in range(127-ord('$'))]
        for k in range(len(cyc)):
            index=ord(cyc[k][i])-ord('$')
            radii[index].append(cyc[k])
        for j in range(len(radii)):
            for h in range(len(radii[j])):
                arr.append(radii[j][h])
        cyc=arr
        i-=1
    strBWT=""
    
    for i in range((len(arr))):
          strBWT+=arr[i][-1]
    
    return  strBWT


def EliasOmega(x:int)->str:
     """
     EliasOmega:func
     input->int
     Output:binary str
     
     Time Complexity : O(log(x)) 
     Space Complexity : O(log(x))
     Reference: FIT 3155 Week 5 Data Compression:Elias Omega Code


     """
     binary=bin(x)[2:]
     n=len(binary) 
     l=[]
     if n==1:   
          return '1'
     n-=1
     while n>0: 
          
          e='0' #flip the first bit to zero
          e+=bin(n)[3:]
          l.append(e)
          n=len(e)
          n-=1
     
     compressed=""
     for i in range(len(l)-1,-1,-1): #buidling elias code
          compressed+=l[i]
     compressed+=binary
     return compressed
     

class NodeB:
     """
     NodeB:Class
     input->int
     Output:binary,str
     Node for Binary Tree

     """
     def __init__(self,key=0,id=-1) -> None:
        self.id=id
        self.key=key
        self.right=None
        self.left=None
        self.encode=""
    
         
class MinHeap:
    """
     Class : MinHeap
     input->int
     Output:binary,str
     n:len(sorted_lst)
     Time Complexity : O(1)
     Space Complexity : O(n)
     
     Minheap for huffman encoding tree
     Reference: FIT 1008 week 12 Priority Queue

    """
    
    def __init__(self,sorted_lst:list[NodeB]) -> None:
          self.heap=sorted_lst
    def extract_min(self)->NodeB:
          """
               extract_min : func
               input->None
               Output:NodeB, minimum key
      
               Time Complexity : O(log(n))
               Space Complexity : O(1)

               
          """
          
          self.heap[1],self.heap[-1]=self.heap[-1],self.heap[1]
        
          MinItem=self.heap.pop() 
          self.sink() #O(log(n))
          return MinItem

    def smaller(self,k:int)-> int:
         """
               extract_min : func
               input->int
               Output:int
      
               Time Complexity : O(1)
               Space Complexity : O(1)
               comparing which 2*k and 2*k+1 is smaller
          """
         index=k
         if len(self.heap)==3:
              if self.heap[1].key>self.heap[2].key:
                   return 2
              else:
                   return 1
                   
              
         
         if 2*k < len(self.heap)-1:
            if self.heap[2*k+1].key>self.heap[2*k].key:
               index=2*k
            elif self.heap[2*k+1].key<self.heap[2*k].key:
               index=2*k+1
         elif 2*k ==len(self.heap):
              
              index=2*k
         elif 2*k+1 < len(self.heap):
            
            if self.heap[2*k+1].key>self.heap[2*k].key:
               index=2*k
            elif self.heap[2*k+1].key<self.heap[2*k].key:
               index=2*k+1
         return index


    def insert(self,item):
        """
               insert: func
               input->int
               Output:int
      
               Time Complexity : O(log(n))
               Space Complexity : O(1)
               insert the object and rise the value to its corresponding position in PQ
               
        """
        self.heap.append(item)
        k=len(self.heap)-1
        while k>1:  
             if self.heap[k].key<self.heap[k//2].key:
                   self.heap[k],self.heap[k//2]=self.heap[k//2],self.heap[k]
                   if self.heap[1].key>self.heap[2].key:
                        self.heap[1],self.heap[2]=self.heap[2],self.heap[1]
                  
                  
             else:   
                    if self.heap[1].key>self.heap[2].key:
                        
                        self.heap[1],self.heap[2]=self.heap[2],self.heap[1]
                           
                    
                    break
                  
             k//=2
             
             
    

    def sink(self):
          k=1
          """
               sink : func
               input->None
               Output:None
      
               Time Complexity : O(log(n))
               Space Complexity : O(1)
               sink the object when inserting
          """
        
          while 2*k<len(self.heap):
                index=self.smaller(k)

                if self.heap[k].key == self.heap[index].key:
                     break
                elif self.heap[k].key>self.heap[index].key:
                     self.heap[k],self.heap[index]=self.heap[index],self.heap[k]
                k=index
                
    def __str__(self) -> str:
         """
               __str__: func(Magic Method)
               input->None
               Output:str
      
               Time Complexity : O(n)
               Space Complexity : O(1)
          
         """
         
         sstr="["
         for i in range(1,len(self.heap)-1):
              sstr+=str(self.heap[i].key)+", "
         if len(self.heap)>2:
            sstr+=str(self.heap[-1].key)
         sstr+= " ]"
         return sstr

class BinaryTree:
    


     """
               BinaryTree: Class
               input->SortedList[NodeB]
               Output:None
      
               Time Complexity : O(nlogn)
               Space Complexity : O(n)
          
     """
    
    
     def __init__(self,sorted_list) -> None:
         self.PQ=MinHeap(sorted_list)
         self.Root=self.build()#O(nlogn)

     def mergeNode(self):
            """
               mergeNode: func
               input->None
               Output:None
      
               Time Complexity : O(logn)
               Space Complexity : O(1)
               to merge the node, which if one of the node is a leaf, it will be at the left side of new node 
               and the non-leaf willbe at the right side,
               if both are leaves, the left side will be smaller and the larger will be at the right side

            """
          
            n1=self.PQ.extract_min() #O(log(n))
            n2=self.PQ.extract_min() #O(log(n))
            
            sum=n1.key+n2.key
            
            mergeNode=NodeB(key=sum) 
            if n1.id == -1 and n2.id != -1:
               mergeNode.left=n2
               mergeNode.right=n1
            elif n2.id == -1 and n1.id != -1:
               mergeNode.left=n1
               mergeNode.right=n2
            else:
               if n1.key<=n2.key:
                    mergeNode.left=n1
                    mergeNode.right=n2
               elif n1.key>n2.key:
                    mergeNode.left=n2
                    mergeNode.right=n1
            #insert back into tree
            self.PQ.insert(mergeNode) #O(log(n))
            
         

    
     def build(self):
         
         """
               build: func
               input->None
               Output:None
               Time Complexity : O(nlogn)
               Space Complexity : O(1)
      
               

         """
         while len(self.PQ.heap)>2:#O(nlogn)
           
              self.mergeNode()  #]=O(logn)
         self.Root=self.PQ.extract_min()
         return self.Root 


def HuffManEncode(txt):
     """
          HuffManEncode: func
          input->txt,string
          Output:Binary Tree(not used in decoding),List[str]
          
         
      

          Encode the txt into a stream of binary

          1.to know what is the maximum value of the each unique type
          2. using counting sort to sort by frequency
          3. after sorting, build a tree with minHeap class and Binary class
          4. build the tree to insert the character to build up encoded binary for each unique character in the txt
          5. use DFS to find the encoded character

          Reference: FIT 3155 Week 5 Data Compression:Huffman Code
     """

     Occu=[0 for i in range(127-ord('$'))]
     m=-1
     ExChar=[]
     EncodeTable=[None for i in range(127-ord('$'))]
     for i in range(len(txt)): #O(n)
          
          index=ord(txt[i])-ord('$')
          Occu[index]+=1
          
               
          m=max(m,Occu[index])

     CountingSort=[[] for i in range(m+1)]
     for i in range(len(Occu)): 
        if Occu[i]>0:
            CountingSort[Occu[i]].append(chr(i+ord('$')))


     NodeList=[None] 
     id=0
     for i in range(len(CountingSort)): #O(n+m)
        for j in range(len(CountingSort[i])):
            ExChar.append([CountingSort[i][j],i])
          
            char=CountingSort[i][j]
           
            NodeList.append(NodeB(key=i,id=char))
            id+=1
     BT=BinaryTree(NodeList) #O(nlog(n))
     """"
     
     Usign DFS to traverse
     so I saved a string when traversing,if Node is not a leaf, if S.left or S.right is not a leaf, give it a previous encoded string and add '0' and '1' respecitively
     and if a leaf, we can give its previous saved encode binary string to its.
     and saved it into a encoded table

     
     """

     DFSList=[BT.Root]
     while len(DFSList)>0:
            S=DFSList.pop()
           
            if S.left is not None:
                 DFSList.append(S.left)
                 S.left.encode=S.encode
                 S.left.encode+="0"
                 
            if S.right is not None:
                 DFSList.append(S.right)
                 S.right.encode=S.encode
                 S.right.encode+="1"
            if S.left is None and S.right is None:
                 if S.id != -1:
                      EncodeTable[ord(S.id)-ord('$')]=S.encode                
     return BT,EncodeTable  





  





def Encode(txtBWT):
     """
               Encode: func
               input->txtBWT,string,List[char]
               Output:binary string
               Time Complexity : O(n^2) becuae of stirng binary concatation?
               Space Complexity : O(unknown)
      

          Encode the txt into a stream of binary 

          1. using BWTNaive to generate  BWT
          2. Encode length of txt using Elias Omega and then add it into the encoded streamdata
          3. find the number of unique character and encode the number using Elias Omeaga and then add it into 
          4. then encode each of the character using huffman encode
          5. for each character , append the character binary and then the length of the huffman and encoded huffman code into the encoded stream data
          6. for each character in BWT, if it is in the same, coutinue the loop after reach til different character, and put the huffman encoded data and the continous occurance of the character
             for that particular
             
         
     """


     encoded_streamdata=""
     txtBWT=NaiveBWT(txtBWT)  #1.
     txt_len=len(txtBWT)
     
     
     txt_lenElias=EliasOmega(txt_len)#2.
     encoded_streamdata+=txt_lenElias
     ExistChar=[None for i in range(127-36)]
     for i in range(len(txtBWT)): #3.
            if ExistChar[ord(txtBWT[i])-ord('$')] is None:
                ExistChar[ord(txtBWT[i])-ord('$')] =txtBWT[i]
     nUniqChar=[ExistChar[i] for i in range(len(ExistChar)) if ExistChar[i] is not None]
     nUniqChar_El=EliasOmega(len(nUniqChar))
     encoded_streamdata+=nUniqChar_El #2.

     HuffmanEncode=HuffManEncode(txtBWT)# give encoded table
     #EliasEncodeLen=[EliasOmega(len(HuffmanEncode[1][i])) if ExistChar[i] is not None else None for i in range(127-36)]
     for character in nUniqChar:  # 5.
          index=ord(character)-ord('$')
          encoded_streamdata+=bin(ord(character))[2:].zfill(7)
          encoded_streamdata+=EliasOmega(len(HuffmanEncode[1][index]))
          encoded_streamdata+=HuffmanEncode[1][index]
     same=1 #occurance of that character form index,i
     for i in range(len(txtBWT)):  #6.
          
          if i < len(txtBWT) -1 and txtBWT[i]==txtBWT[i+1]:
               
               same+=1
               
          else:
             index=ord(txtBWT[i])-ord('$')
             encoded_streamdata+=HuffmanEncode[1][index]
             encoded_streamdata+=EliasOmega(same)
             same=1
     
     if len(encoded_streamdata)%8!=0: # make it into has a multiple length of 8
          encoded_streamdata+= '0'*(8-len(encoded_streamdata)%8)
    
     return encoded_streamdata





import sys
# this function reads a file and return its content
def read_file(file_path: str) -> str:
    
    f = open(file_path, 'r')
    line = f.readlines()
    f.close()
    return line



if __name__ == '__main__':
    #retrieve the file paths from the commandline arguments
    _, text_filename = sys.argv
   
    txt = read_file(text_filename)[0]
   
    output = Encode(txt)
    i=0
    with open("bwtencoded.bin" , 'wb') as file:
        while i<len(output):
          
            byte=output[i:i+8]
            

            file.write(int(byte,2).to_bytes())
            i+=8
