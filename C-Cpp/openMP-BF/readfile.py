# Corrected file path
file1 = "C:\\FIT3143\\LITTLE_WOMEN.txt"
file2 = "C:\\FIT3143\\MOBY_DICK.txt"
file3 = "C:\\FIT3143\\SHAKESPEARE.txt"
# read unqiue file for the file
def readfile(file:str):
    Lst=[]
    with open(file, "r",encoding="utf-8") as f:
        for line in f:
            stri=line.strip()
            unique=True
            for x in Lst:
                if (stri==x):
                    unique=False
                    break
            if (unique):
                Lst.append(stri);
                
          
        return Lst



# write file as encoder as utf-8 and  newline='\n' to ensure it can be read by C
def writefile(lst:[str],filename:str):
    file = open(filename, "w",encoding="utf-8", newline='\n')
    file.write(str(len(lst))+"\n")
    for i in range(len(lst)):
        file.write(lst[i]+"\n")
    file.close()
    
File1=readfile(file1)
File2=readfile(file2)
File3=readfile(file3)
#merging file which only append unqiue words
def merge(file1,file2):
    
    for x in file2:
        if x not in file1:  
            file1.append(x)

    return file1


print(len(File1))
print(len(File2))
print(len(File3))
File1=merge(File1,File2,1)
File1=merge(File1,File3,2)
writefile(File1,"C:\FIT3143\combination")