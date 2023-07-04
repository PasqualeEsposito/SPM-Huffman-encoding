for ((i = 0; i < 5; i++)); do ./huffman_ff bible1MB.txt 1; done
for ((i = 2; i <= 64; i += 2)); do
    for ((j = 0; j < 5; j++)); do
        ./huffman_ff bible1MB.txt $i
    done
done

for ((i = 0; i < 5; i++)); do ./huffman_ff bible10MB.txt 1; done
for ((i = 2; i <= 64; i += 2)); do
    for ((j = 0; j < 5; j++)); do
        ./huffman_ff bible10MB.txt $i
    done
done

for ((i = 0; i < 5; i++)); do ./huffman_ff bible100MB.txt 1; done
for ((i = 2; i <= 64; i += 2)); do
    for ((j = 0; j < 5; j++)); do
        ./huffman_ff bible100MB.txt $i
    done
done