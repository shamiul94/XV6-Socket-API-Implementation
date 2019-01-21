# Patches
## What is patch? 
Patches are files that make necessary changes to specific files to implement new features. 

Here I have 3 patches and another one is from class teacher. 
```
  1. patch_xv6_Socket_1505038_Final_Final - My solution patch for offline which was submitted. 
  
  2. Online_patch_XV6_out_of_length_bound_error_implementation - This patch was created in online sessional. 
     In this problem we have to implement a new Error code E_INVALID_LENGTH. If the length of the string 
     user (client) is going to input is more than 16, send() function will return E_INVALID_LENGTH from kernel 
     and will ask for input again. 
     
  3. xv6_SocketStub_patch_saifur_sir - This is the skeleton code. 
```

## Steps to use patch

1. Copy one of these patches in the ```xv6-public``` folder of a fresh xv6 copy. 

2. Open terminal and run this command - 

```
    git apply patch_xv6_Socket_1505038_Final_Final
```
