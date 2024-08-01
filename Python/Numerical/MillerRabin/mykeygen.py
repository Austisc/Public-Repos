"""
FIT 3155 Assignment3 Q1

Name: Austin Sing Jet Wong
ID:   32933975


"""



import random
from math import log,ceil








        


def ModularExp(a,b,n):
    """
    Reference: FIT3155 S1/2023: Advanced Algorithms and Data
    Structures
    Week 6: Semi-numerical (or number-theoretic) algorithms:  modular exponentiation (pg.54/69)
    """
    bin_b=bin(b)[2:]
    
    binary_rep=[len(bin_b)-i-1 for i in range(len(bin_b)) if bin_b[i]=='1']   
    result=1
    for i in range(len(binary_rep)-1,-1,-1):
       
        result=(result* pow(a,pow(2,binary_rep[i]))%n)%n
    return result




            


def MillerRabin(n,k):
    """
     
     Reference: FIT3155 S1/2023: Advanced Algorithms and Data
     Structures
     Week 6: Semi-numerical (or number-theoretic) algorithms:Miller-Rabin’s Randomized Primality testing algorithm (pg67./69.)

    """
    if n == 2: # Special case
        return True
    if n%2==0:
        return False
    s=0
    t=n-1   
    
   
    while(t%2==0):
        s+=1
        t//=2
    
    for j in range(k):
        a=random.randint(2,n-2)

        if ModularExp(a,n-1,n)!=1:
            return False
        exp=2
        si_cur=ModularExp(a,exp*t,n)
        
        for i in range(1,s+1):
                si_prev=si_cur
                si_cur= ModularExp(a,exp*t,n)  # to use previous value to prevent to calculate it again
                if si_cur == 1  and not(si_prev in [1,n-1]): 
                    return False 
                exp*=2

    return True


def gcd(a,b):
    if b>a:
       a,b=b,a
    if a%b==0:
        return b
    return gcd(b,a%b)



def keygenerator(d:int):
    """
    Input: d, integer
    Output: p:int,q:int, Lambda:int , exp:int
    So the probability estimate of choosing an prime number in a range of n is 1/ln(n) from lecture slide (pg.57/69.),
    so the number is probably related to ln(n) since there is atleast 1 prime number in ln(n), so i choose ln(n).
    """
    
    secret_prime=[]
    while len(secret_prime)<2 and d<=2000:  # relative first two smallest prime and d must be equal or lower to 2000
        x=2**d-1
        if MillerRabin(x,ceil(log(x))):  # test if it is prime through MillerRabin Testing,
            
            secret_prime.append(x)
            d+=1
        else:
            
            d+=1
    
    p,q=secret_prime
    
    n=p*q
    Lambda=(p-1)*(q-1)//gcd(p-1,q-1) #given formula
    exp=random.randint(3,Lambda-1)  # choose a random value
    while gcd(Lambda,exp)!=1: # choose unitl lambda and exponent are relaitve prime
        exp=random.randint(3,Lambda-1) #if not choose another random valuer
    
    return p,q,n,exp





def write_file(d:int):
    values=keygenerator(d)
    p,q,modulus,exponent=values
    with open("publickeyinfo.txt",'w') as file:
             file.write("# modulus (n)\n")
             file.write(str(modulus)+'\n')
             file.write("# exponent (e)\n")
             file.write(str(exponent)+'\n')
    with open("secretprimes.txt",'w') as file:
            file.write("# p\n")
            file.write(str(p)+'\n')
            file.write("# q\n")
            file.write(str(q)+'\n')

             



import sys
if __name__ == '__main__':
    _, value= sys.argv
    write_file(int(value)) 
