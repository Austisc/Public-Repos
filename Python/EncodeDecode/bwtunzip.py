"""
FIT 3155 Assignment2 Q2 Encode

Name: Austin Sing Jet Wong
ID:   32933975


"""



def BWTinverse(strBWT):
    

    """
               BWTinverse: func
               input->txtBWT:string,List[char]
               Output: string
               Time Complexity : O(n)
               Space Complexity : O(n)
      

          Inverse BWT to inverse its BWT process and get back
          1.using counting sort to get back the ranking
            while sorting, for each occu of that particular character, append its index inside the List of List
          2. to get its ranking, traverse in order, and to get its ranking(posiiton in F-Map) and by remember the prev occurance of the previous ranking,
             we can get its ranking in F-Map
          3. and then traverse ranktalbe again, I created F-Map and saved its as  ASCII character number minus ord('$')

          4. so instead of using O(n^2) traverse, sacrifice O(n) of auxillary space complexity to archive O(n),to get back its orignal string
               so finding the dollar sign(first character), we can get its position at Last position through the List of List, and we will get the original first character of the txt
               and then to know which occurance is it, we can use the index of the correspnding character minus rank of that character from rank table to get x-th occu and use to find 
               where the character at the L-Map since it storing its positon at L-Map and repeat the process,until getting the original text in linear time.
          Reference: FIT 3155 week 3 Burrow-Wheeler Transform

         
    """
     #rank occu  char isnide ranktable,occu adn char does not used for inverse
    ranktable=[[-1,0,chr(i+36)] for i in range(126-ord('$'))]
    OccuIndex=[[] for i in range(126-ord('$'))]
    for i in range(len(strBWT)):
        index=ord(strBWT[i])-ord('$')
        ranktable[index][1]+=1
        OccuIndex[index].append(i)
    prev_occu=0
    rank=0
    for i in range(len(ranktable)):
         if ranktable[i][1]>0:
            ranktable[i][0]=rank
            prev_occu=ranktable[i][1]
            rank+=prev_occu
       
    #s=[]
    FMap=[]
    for i in range(len(ranktable)): #3.
          if ranktable[i][0]>=0:
             for j in range(ranktable[i][1]):
                    FMap.append(i)

    decoded_str=""
    rank=0
    occu=0
    index=0
    prevoccu=0
    table_index=0
    table_index=ranktable[table_index][0]
    while len(decoded_str)<len(strBWT)-1: #4.
            indtxt=FMap[OccuIndex[table_index][occu]]+ord('$')
            decoded_str+=chr(indtxt)
            prev_table_index=table_index
            prevoccu=occu
            table_index=FMap[OccuIndex[table_index][occu]]
            rank=ranktable[table_index][0]
            occu=OccuIndex[prev_table_index][prevoccu]-rank
            
    return decoded_str
  


def EliasDecoder(codeword:str,pos) ->int:



     """
               EliasDecode: func
               input->codeword,pos:int
               Output->N:int,pos+readlen
               Output:binary string
               
          

          Just modify the decoder which only decode partially by knowing the starting position of EliasDecoder
          and the remaing decode logic is the same
          output the length and the position after decoding the integer

          Reference: FIT 3155 Week 5 Data Compression:Elias Omega Decoding
             
     """
     
     readlen=1
     component=""
     
     while codeword[pos]=='0':

          component = '1'+ codeword[pos+1:pos+readlen]
          pos+=readlen
          readlen=int(component,2)+1
     else:
          N=int(codeword[pos:pos+readlen],2)
          
          return [N,pos+readlen]
     
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
    



class ReConstructedTree:



     """
     Class : ReConstructedTree
     input->None
     Output:None
     
     
     Replicate of the binary tree when encoding by inserting back from the root for each character by inserting only
     

    """
     def __init__(self):
          self.Root=NodeB()
     def insert(self,binary,char)->None:


          """
          Class : ReConstructedTree
          input->binary:str,char: chr
          Output:None
          Timecomplexity:O(n)
          aux space complexity:O(n)
          n=len(binary)
          so inserting back into the tree, 
          if it is '0',it is the left side of the node while '1' is the right side which if its node of the direction is None create a node 
          else: just traverse and change its AN until reached the last index of binary and create a Node and set its id as char
          """
          j=0
          AN=self.Root
          while len(binary)>j:
               if binary[j]=='0':
                    if AN.left is None:
                         AN.left=NodeB()
                         AN=AN.left
                         if j+1==len(binary):
                              AN.id=char
                         
                    else: 
                         AN=AN.left


               if binary[j]=='1':
                    if AN.right is None:
                         AN.right=NodeB()
                         AN=AN.right
                         if j+1==len(binary):
                              AN.id=char
                         
                    else:     
                         
                         AN=AN.right



               j+=1
                         
                    

def HuffmanDecode(codeword,pos,RTree):
          


          
          """
          HuffmanDecode
          input->codeword:str,binary:pos,RTree:Reconstructed Trree
          Output:AN.id:chr, pos:int
          Timecomplexity:O(n)
          aux space complexity:O(1)
     
          traversing left and right until u reach the leaf and return [AN.id,pos],pos is the starting position of the next stage

          """
          AN=RTree.Root
          
          while AN.id==-1:
               if codeword[pos]=='1':
                    AN=AN.right
               elif codeword[pos]=='0':
                    AN=AN.left
               pos+=1
          else:
               return [AN.id,pos]
               


          

     


def decoded(encoded:str)->str:

     """
     decoded:func
     input->binary:string
     Output:OrigString:String
     n=len(txt)
     decoding the encoded binary starting from head
     repeat the process like encoding but with decoding fucntion
     1. deocde the word length 
     2. decode the number of uniq length 
     3. convert binary to its ascii character from pos:pos+7 and then pos+=7 and use EliasDecoder to know what is the length of huffman and then
        and to get the binary, and then insert into reconstruction tree until i==number of uniq character
     4. to find the strBWT with Huffman Decoding to know which character is  it and what is the occurance for the current character and add it into string until it is equal
        to the word len, which u will get the L-Map
     5. and then use the BWT to get the original string

     """
    
     
     pos=0
     word_len = EliasDecoder(encoded,pos)
     
     pos+= word_len[1]
     nUniqChar= EliasDecoder(encoded,pos)
     
     pos= nUniqChar[1]
     CharList=[]
     i=0
     RT=ReConstructedTree()
     while i<nUniqChar[0]: #3.
           character=chr(int(encoded[pos:pos+7],2))
           CharList.append(character)
          
           pos+=7
           huffman_length=EliasDecoder(encoded,pos)
           pos=huffman_length[1]
           encoded_character=encoded[pos:pos+huffman_length[0]]
           RT.insert(encoded_character,character)
           pos+=huffman_length[0]
           i+=1     
     strBWT=""
     
     while len(strBWT)<word_len[0]:
          chrpos= HuffmanDecode(encoded,pos,RT)
          pos=chrpos[1]
          chrOccu=EliasDecoder(encoded,pos)
          pos=chrOccu[1]
          for i in range(chrOccu[0]):
               strBWT+=chrpos[0]
     OrigString=BWTinverse(strBWT)
     return OrigString





import sys

def read_file(file_path: str) -> str:
     with open(file_path, "rb") as file:
        byte_chr = file.read()
     output=""
     for char in byte_chr:
          output+=bin(char)[2:].zfill(8)
          
     #bin_size=len(byte_chr)*8
     #print(len(output))
     #output = output.zfill(bin_size) #make sure the front zero appear
     return output


         
    

if __name__ == '__main__':
#retrieve the file paths from the commandline arguments
    _, text_filename = sys.argv
   
    txt = read_file(text_filename)
   
    
    txt = decoded(txt)
    
    with open("recovered.txt" , 'w') as file:
        file.write(txt)

