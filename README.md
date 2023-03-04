<h2 align="center"> EntropyReducer: Reduce The Entropy Of Youre Payload And Obfuscate It With Serialized Linked Lists </h2>

<br>


### How Does It Work

<!-- define entropy + it lower the entropy bcz 0x00 is repeated in an organized/ordered manner -->

**EntropyReducer** algorithm is determined by [BUFF_SIZE](https://github.com/Maldev-Academy/EntropyReducer/blob/main/EntropyReducer/Common.h#L13) and [NULL_BYTES](https://github.com/Maldev-Academy/EntropyReducer/blob/main/EntropyReducer/Common.h#L14) values. The following is how would **EntropyReducer** organize your payload if `BUFF_SIZE` was set to **4**, and `NULL_BYTES` to **2**.

![image](https://user-images.githubusercontent.com/111295429/222906172-800d8436-abb4-4c1a-96b6-19a8f0c846cf.png)

<br>


#### Obfuscation Algorithm 

- **EntropyReducer** first checks if the input raw payload is of a size that's multiple of `BUFF_SIZE`, if not, it pads it to be as so.
- It then takes every `BUFF_SIZE` chunk from the payload, and makes a linked list node for it, using the [InitializePayloadList](https://github.com/Maldev-Academy/EntropyReducer/blob/main/EntropyReducer/EntropyReducer.c#L10) function, initializing the payload as a linked list.
- The created node will have an empty buffer of size `NULL_BYTES`, that will be used to lower the entropy 
- At this point, although **EntropyReducer** completed its task by lowering the entropy of the payload, it doesn't stop here. It then continues to randomize the order of each node in the linked list, breaking down the raw payload's order. This step is done via a [Merge Sort Algorithm](https://www.geeksforgeeks.org/merge-sort-for-linked-list/) that is implemented through the [MergeSort](https://github.com/Maldev-Academy/EntropyReducer/blob/main/EntropyReducer/EntropyReducer.c#L160) function.
- The sorted linked list is in random order because the value in which the linked list is *sorted* is the **XOR** value of the first three bytes of the raw payload, this value determines its size in the re-organized linked list, this step can be shown [here](https://github.com/Maldev-Academy/EntropyReducer/blob/main/EntropyReducer/EntropyReducer.c#L133)
- Since saving a linked list to a file is [impossible](https://stackoverflow.com/a/9854707/15354012) due to the fact that it's linked together by pointers. We are forced to [serialize](https://qr.ae/prZ6Lx) it.
- Serialization of the generated linked list is done via the `Obfuscate` function [here](https://github.com/Maldev-Academy/EntropyReducer/blob/main/EntropyReducer/main.c#L71).
- After that, the serialized data is ready to be written to the output file.


<br>

#### Deobfuscation Algorithm 

- Since the last step in the `Obfuscation Algorithm` was serializing the linked list, the first thing that must be done here is to deserialize the obfuscated payload, generating a linked list from it, this step is done [here](https://github.com/Maldev-Academy/EntropyReducer/blob/main/PoC/EntropyReducer.c#L210) in the `Deobfuscate` function.
- Next step is to sort the linked list using the node's Id, which is done using the same [Merge Sort Algorithm](https://github.com/Maldev-Academy/EntropyReducer/blob/main/PoC/EntropyReducer.c#L216) used before.
- Now, the linked list is in the right order to re-construct the payload's bytes as they should. So we simply strip the payload's original bytes from each node, as done [here](https://github.com/Maldev-Academy/EntropyReducer/blob/main/PoC/EntropyReducer.c#L223).
- Last step is to free the allocated nodes, which is done [here](https://github.com/Maldev-Academy/EntropyReducer/blob/main/PoC/EntropyReducer.c#L250).

<br>

### Usage

- **EntropyReducer** simply read the raw payload file from the command line, and writes the obfuscated version to the same file's name prefixed with ".ER".
- The size of the final obfuscated payload varies depending on the values of both `BUFF_SIZE` and `NULL_BYTES`. However, it can be determined using the following equation
```
FinalSize = ((OriginalSize + BUFF_SIZE - OriginalSize % BUFF_SIZE ) / BUFF_SIZE) * (BUFF_SIZE + NULL_BYTES + sizeof(INT))
```
- The [PoC](https://github.com/Maldev-Academy/EntropyReducer/tree/main/PoC) project in this repo is used to execute the `".ER"` file generated as an example of deserializing and deobfuscating it.

<br>

### Include In Your Projects

All you have to do is add [EntropyReducer.c](https://github.com/Maldev-Academy/EntropyReducer/blob/main/PoC/EntropyReducer.c) and [EntropyReducer.h](https://github.com/Maldev-Academy/EntropyReducer/blob/main/PoC/EntropyReducer.h) files to your project, and call the [Deobfuscate](https://github.com/Maldev-Academy/EntropyReducer/blob/main/PoC/EntropyReducer.h#L20) function. You can check [PoC/main.c](https://github.com/Maldev-Academy/EntropyReducer/blob/main/PoC/main.c#L54) for reference.


<br>

### Output Example

In this example, `BUFF_SIZE` was set to **3**, and `NULL_BYTES` to **1**.

- The raw payload, first payload chunk (`FC 48 83`)

![image](https://user-images.githubusercontent.com/111295429/222896340-b1d7fe55-6bb3-4614-be91-38c939f8ea77.png)


- The same payload chunk, but at a different offset 

![image](https://user-images.githubusercontent.com/111295429/222896883-8f98a4c0-2820-4af7-b8fb-817069e4cf31.png)

<br>

### Profit

- The x64 calc shellcode generated by metasploit is of entropy `5.883`, view by [pestudio](https://www.winitor.com/).

![image](https://user-images.githubusercontent.com/111295429/222897280-caa4f2dc-bacb-42eb-808f-fbc81094c1de.png)


- The same file, AES encrypted, scores entropy of `7.110`.

![image](https://user-images.githubusercontent.com/111295429/222897475-45705211-6d4d-41b5-9358-e9ea215f3bd2.png)


- Nearly the same result with the RC4 algorithm as well; `7.210`

![image](https://user-images.githubusercontent.com/111295429/222897447-32958bb3-1db2-4056-b23a-1c4f53b1a67e.png)


- Using **EntropyReducer** however, scoring entropy even lower that that of the original raw payload; `4.093`

![image](https://user-images.githubusercontent.com/111295429/222897491-f9217e51-3007-4f1c-a5e4-b8e4c89442c3.png)


<br>


#### The Merge Sort Algorithm Is Taken From [c-linked-list](https://github.com/Leyxargon/c-linked-list).


<!-- add our names if u want hbb idk but delete all the comments -->



