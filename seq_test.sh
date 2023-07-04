for ((i = 0; i < 5; i++)); do
  ./huffman_seq bible1MB.txt;
done

for ((i = 0; i < 5; i++)); do
  ./huffman_seq bible10MB.txt;
done

for ((i = 0; i < 5; i++)); do
  ./huffman_seq bible100MB.txt;
done