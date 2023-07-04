The aim of this project is to create a compressor algorithm, based on Huffman coding, that works in parallel. The parallel implementations are implemented with C++ native threads and [FastFlow](https://github.com/fastflow/fastflow).<br /><br />
**HOW TO RUN THE CODE:**
  1. Download the folder
  2. Generate the executable files using the command ```make```
  3. Add the file you want to compress in the **input** folder (there are already three examples in the aforementioned folder)
  4. Execute the program passing the file name to compress
      - ```./huffman_seq <filename.txt> -v``` (-v is optional)
      - ```./huffman_thread <filename.txt>  <num_threads> -v``` (-v is optional)
      - ```./huffman_ff <filename.txt> <num_threads> -v``` (-v is optional)
  5. The compressed files will be stored in the **output** folder.<br /><br />
**NOTE:** Before executing the [FastFlow](https://github.com/fastflow/fastflow) implementation, make sure you correctly downloaded the library first.
